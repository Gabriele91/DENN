#include <fstream>
#include "Denn/Algorithm.h"
#include "Denn/Core/Filesystem.h"

namespace Denn
{
	DennAlgorithm::DennAlgorithm(Instance&	instance, const Parameters&   params)
	: m_main_random(instance.random_engine())
    , m_thpool(instance.thread_pool())
	, m_loss_function(instance.loss_function())
	, m_validation_function(instance.validation_function())
	, m_test_function(instance.test_function())
	, m_output(RuntimeOutputFactory::create(*params.m_runtime_output_type, 
											 instance.output_stream(), *this))
	, m_default(std::make_shared<Individual>( 
		  *params.m_default_f
		, *params.m_default_cr
	    , *params.m_perc_of_best
		, instance.neural_network()
		))
	, m_dataset_loader(&instance.dataset_loader())
	, m_dataset_batch(&instance.dataset_loader())
	, m_params(params)
	{
	}

	//init
	bool DennAlgorithm::init()
	{
		//success flag
		bool success = m_dataset_loader != nullptr;
		//init test set
		if(*m_params.m_batch_offset <= 0)
			m_dataset_batch.start_read_batch(*m_params.m_batch_size, *m_params.m_batch_size);
		else
			m_dataset_batch.start_read_batch(*m_params.m_batch_size, *m_params.m_batch_offset);
		//init random engine
		m_main_random.reinit(*m_params.m_seed);
		//gen clamp functions
		m_clamp_function = gen_clamp_func();
		//clear random engines
		m_population_random.clear();
		//true
		return success;
	}

	bool DennAlgorithm::init_population()
	{
		//get np
		size_t np = (size_t)m_params.m_np; //init current_np();
		//min size
		if (!np) return false;
		//init random engines
		for(size_t i=0; i != np ;++i)
		{
			m_population_random.emplace_back(main_random().uirand());
		}
		//init pop
		m_population.init
		(
			   np
			,  m_default
			,  current_batch()
			,  gen_random_func()
			, *m_loss_function
			,  m_thpool
			,  gen_random_func_thread()
		);
		//method of evoluction
		m_e_method = EvolutionMethodFactory::create(m_params.m_evolution_type, *this);
		//reset method
		m_e_method->start();
		//true
		return true;
	}

	//big loop
	Individual::SPtr DennAlgorithm::execute()
	{
		//init all
		if (!init()) 			return nullptr;
		if (!init_population()) return nullptr;
		//global info
		const size_t n_global_pass = ((size_t)m_params.m_generations / (size_t)m_params.m_sub_gens);
		const size_t n_sub_pass = m_params.m_sub_gens;
		//restart init
		m_restart_ctx = RestartContext();
		//best
		Scalar worst_eval = m_e_method->best_from_validation() 
						          ? validation_function_worst() 
						          : loss_function_worst();
		//default best
		m_best_ctx = BestContext(nullptr, worst_eval);
		execute_update_best(0,0);
		//mask
		if(*m_params.m_use_mask)
		{
            m_nnlast = m_best_ctx.m_best->m_network;
			m_nnlast_eval = m_best_ctx.m_eval;
			m_nnmask = m_default->m_network;
			m_nnmask_count = 0;
			execute_update_mask(0,0);
		}
		//start output
		if (m_output) m_output->start();
		//main loop
		for (size_t pass = 0; pass != n_global_pass; ++pass)
		{
			execute_a_pass(pass, n_sub_pass);
			//next
			next_batch();
		}
		//best on validation?
		if((bool)m_params.m_last_with_validation && !m_e_method->best_from_validation())
		{
			m_best_ctx.m_eval = validation_function_worst();
			execute_update_best_on_validation();
		}
		//end output
		if (m_output) m_output->end();
		//result
		return m_best_ctx.m_best;
	}

	//using the test set on a individual
	Scalar DennAlgorithm::execute_test() const 
	{
		//validation
		DataSetScalar test;
		m_dataset_loader->read_test(test);
		//compute test
		Scalar eval = (*m_test_function)((NeuralNetwork&)*m_best_ctx.m_best, test);
		//return
		return eval;
	}
	Scalar DennAlgorithm::execute_test(Individual& individual) const 
	{
		//validation
		DataSetScalar test;
		m_dataset_loader->read_test(test);
		//compute		
		Scalar eval = (*m_test_function)((NeuralNetwork&)individual, test);
		//return
		return eval;
	}
	/////////////////////////////////////////////////////////////////
	//test
	//find best individual (validation test)
	Individual::SPtr DennAlgorithm::find_best_on_validation(Scalar& out_eval)
	{
		//best 
		size_t best_i;
		find_best_on_validation(best_i, out_eval);
		//return best
		return m_population.parents()[best_i];
	}
	bool DennAlgorithm::find_best_on_validation(size_t& out_i, Scalar& out_eval)
	{
		if(m_thpool) return parallel_find_best_on_validation(*m_thpool,out_i,out_eval);
		else 		 return serial_find_best_on_validation(out_i, out_eval);
	}
	bool DennAlgorithm::serial_find_best_on_validation(size_t& out_i, Scalar& out_eval)
	{
		//ref to pop
		auto& population = m_population.parents();
		//validation
		DataSetScalar validation;
		m_dataset_loader->read_validation(validation);
		//best
		Scalar best_eval =  validation_function_worst();
		size_t	   best_i= 0;
		//find best
		for (size_t i = 0; i != current_np(); ++i)
		{
			auto& i_target = *population[i];
			//test
			Scalar eval = (*m_validation_function)((NeuralNetwork&)i_target, validation);
			//safe, nan = worst
			if (std::isnan(eval)) eval = validation_function_worst();
			//find best
			if(!i && validation_function_compare(eval,best_eval))
			{
				best_i = i;
				best_eval = eval;
			}
		}
		//find best
		out_i    = best_i;
		out_eval = best_eval;
		return true;

	}
	bool DennAlgorithm::parallel_find_best_on_validation(ThreadPool& thpool, size_t& out_i, Scalar& out_eval)
	{
		//ref to pop
		auto& population = m_population.parents();
		//get np
		size_t np = current_np();
		//validation
		DataSetScalar validation;
		m_dataset_loader->read_validation(validation);
		//list eval
		std::vector<Scalar> validation_evals(population.size(), validation_function_worst());
		//alloc promises
		m_promises.resize(np);
		//for all
		for (size_t i = 0; i != np; ++i)
		{
			//ref to target
			auto& i_target = *population[i];
			auto& eval     = validation_evals[i];
			//add
			m_promises[i] = thpool.push_task([&]()
			{
				//test
				eval = (*m_validation_function)((NeuralNetwork&)i_target, validation);
				//safe, nan = worst
				if (std::isnan(eval)) eval = validation_function_worst();;
			});
		}
		//wait
		for (auto& promise : m_promises) promise.wait();
		//find best
		if(m_validation_function->minimize())
		{
			out_i = std::distance(validation_evals.begin(), std::min_element(validation_evals.begin(), validation_evals.end()));
		}
		else 
		{
			out_i = std::distance(validation_evals.begin(), std::max_element(validation_evals.begin(), validation_evals.end()));			
		}
		out_eval = validation_evals[out_i];
		return true;

	}
	
	/////////////////////////////////////////////////////////////////
	//Intermedie steps
	void DennAlgorithm::execute_a_pass(size_t pass, size_t n_sub_pass)
	{
		///////////////////////////////////////////////////////////////////
		if (*m_params.m_reval_pop_on_batch || pass == 0) 
			execute_loss_function_on_all_population(m_population.parents());
		///////////////////////////////////////////////////////////////////
		//output
		if(m_output) m_output->start_a_pass();
		//start pass
		m_e_method->start_a_gen_pass(m_population);
		//sub pass
		for (size_t sub_pass = 0; sub_pass != n_sub_pass; ++sub_pass)
		{
			execute_a_sub_pass(pass, sub_pass);
		}
		//end pass
		m_e_method->end_a_gen_pass(m_population);
		//update context
		execute_update_best(pass, n_sub_pass);
		//update mask
		execute_update_mask(pass, n_sub_pass);
		//restart
		if(m_e_method->can_reset()) execute_update_restart(pass);
		//output
		if(m_output) m_output->end_a_pass();
	}
	void DennAlgorithm::execute_a_sub_pass(size_t pass, size_t sub_pass)
	{
		//output
		if(m_output) m_output->start_a_sub_pass();
		//pass
		execute_pass();
		//output
		if(m_output) m_output->end_a_sub_pass();
	}
	void DennAlgorithm::execute_update_best(int pass, int n_sub_pass)
	{
		if(m_e_method->best_from_validation()) execute_update_best_on_validation();
		else             					   execute_update_best_on_loss_function();
		//if save intermediate results
		if(m_params.m_save_intermediate && m_best_ctx.m_best && m_best_ctx.m_best->m_network.size())
		{
			//cout
			static size_t best_counter=0;
			++best_counter;
			//string name
			auto path = Filesystem::get_directory(m_params.m_intermediate_output);
			auto name = Filesystem::get_basename(m_params.m_intermediate_output);
			auto ext = Filesystem::get_extension(m_params.m_intermediate_output);
			//fainal name
			auto outputpath = path + "/" + 
							  name + "_best_" +
							  std::to_string(best_counter) + "_" +
							  std::to_string((pass+1)*n_sub_pass) + ext;
			//output
			std::ofstream m_file(outputpath);
			SerializeOutputFactory::create(ext, m_file, m_params)->serialize_best(
				0, //no time
				m_best_ctx.m_best->m_eval,
				m_best_ctx.m_best->m_f,
				m_best_ctx.m_best->m_cr,
				m_best_ctx.m_best->m_network
			);
		}
	}	
	void DennAlgorithm::execute_update_best_on_validation()
	{
		//find best
		Scalar curr_eval = Scalar(0.0);
		auto curr = find_best_on_validation(curr_eval);
		//validation best
		if (validation_function_compare(curr_eval, m_best_ctx.m_eval))
		{
            //copy last
            if((*m_params.m_use_mask) 
			&& m_best_ctx.m_best 
			&& m_best_ctx.m_best->m_network.size())
            {
                //copy
                m_nnlast = m_best_ctx.m_best->m_network;
            }
			//it can change the values of the best individual
			m_best_ctx.m_best = curr->copy();
			//save eval (on validation) of best
			m_best_ctx.m_eval = curr_eval;
		}
	}
	void DennAlgorithm::execute_update_best_on_loss_function()
	{
		//find best
		auto curr = m_population.parents().best();
		//revalue best of the best on the new batch
		if(m_best_ctx.m_best)
		{
			m_best_ctx.m_eval =
			m_best_ctx.m_best->m_eval = (*m_loss_function)((NeuralNetwork&)*m_best_ctx.m_best, current_batch());
		}
		//loss best
		if (loss_function_compare(curr->m_eval, m_best_ctx.m_eval))
		{
            //copy last
            if((*m_params.m_use_mask) 
			&& m_best_ctx.m_best 
			&& m_best_ctx.m_best->m_network.size())
            {
                //copy
                m_nnlast = m_best_ctx.m_best->m_network;
            }
			//it can change the values of the best individual
			m_best_ctx.m_best = curr->copy();
			//save eval (on test set) of best
			m_best_ctx.m_eval = curr->m_eval;
		}
	}
	void DennAlgorithm::execute_update_mask(int pass, int n_sub_pass)
	{
		//update mask
		if(*m_params.m_use_mask)
		{
			//first pass (==init)
			if(pass == 0)
			{
				m_nnmask.fill(1);
				return;
			}
			//cout if and only if it's the same best
    		m_nnmask_count = (m_nnmask_count+1) * m_best_ctx.m_eval==m_nnlast_eval; 
			//save eval
			m_nnlast_eval = m_best_ctx.m_eval;
            //rest mask if more than 10 equal best 
			if( *m_params.m_mask_reset_count < 0 ||
				m_nnmask_count <= *m_params.m_mask_reset_count )
			{
				m_nnmask.compute_mask(m_nnlast, m_best_ctx.m_best->m_network,
				[](Scalar oldw, Scalar neww) -> Scalar
				{
					return std::pow(std::abs(neww-oldw)+1.0,2.0)-Scalar(1.0);
					//return std::abs(neww-oldw);
				});
				m_nnmask.apply_sort([this](Scalar value, size_t i, size_t msize) -> Scalar
				{
					if(Scalar(i)/Scalar(msize) >= *m_params.m_mask_factor)
						return Scalar(true);
					else
						return 0;//Scalar(random().uniform() < *m_params.m_mask_factor);
				});
			}
			else
			{
				m_nnmask.fill(1);
			}
			//if save intermediate results
			if(m_params.m_save_intermediate && m_nnmask.size())
			{
				//cout
				static size_t mask_counter=0;
				++mask_counter;
				//string name
				auto path = Filesystem::get_directory(m_params.m_intermediate_output);
				auto name = Filesystem::get_basename(m_params.m_intermediate_output);
				auto ext = Filesystem::get_extension(m_params.m_intermediate_output);
				//fainal name
				auto outputpath = path + "/" + 
								  name + "_mask_" +
								  std::to_string(mask_counter) + "_" +
								  std::to_string((pass+1)*n_sub_pass) + ext;			
				//output
				std::ofstream m_file(outputpath);
				SerializeOutputFactory::create(ext, m_file, m_params)->serialize_best(
					0, //no time
					0, //no eval
					0, //no f 
					0, //no cr
					m_nnmask
				);
			}
		}
	}
	void DennAlgorithm::execute_update_restart(size_t pass)
	{
		//if not enabled then not reset
		if(!*m_params.m_restart_enable) return ;
		//inc count
		if (std::abs(m_best_ctx.m_eval - m_restart_ctx.m_last_eval) <= (Scalar)m_params.m_restart_delta)
		{
			++m_restart_ctx.m_test_count;
		}
		else
		{
			m_restart_ctx.m_test_count = 0;
			m_restart_ctx.m_last_eval = m_best_ctx.m_eval;
		}
		//first
		if (!pass) m_restart_ctx.m_last_eval = m_best_ctx.m_eval;
		//test
		if (m_restart_ctx.m_test_count >= (Scalar)m_params.m_restart_count)
		{
			m_population.restart
			(
				  m_best_ctx.m_best					     //best
				, main_random().index_rand(current_np()) //where put
				, m_default							     //default individual
				, current_batch()						 //current batch
				, gen_random_func()				         //random generator
				, *m_loss_function				         //fitness function
				, m_thpool								 //threads
				, gen_random_func_thread()				 //random generator (by thread)
			);
			m_restart_ctx.m_test_count = 0;
			m_restart_ctx.m_last_eval = m_best_ctx.m_eval;
			//restart inc
			++m_restart_ctx.m_count;
			//output
			if(m_output) m_output->restart();
		}
	}
	
	/////////////////////////////////////////////////////////////////
	//execute a pass
	void DennAlgorithm::execute_pass()
	{
		m_e_method->start_a_subgen_pass(m_population);
		if (m_thpool) parallel_execute_pass(*m_thpool);
		else          serial_execute_pass();
		m_e_method->end_a_subgen_pass(m_population);
	}
	void DennAlgorithm::serial_execute_pass()
	{
		//get np
		size_t np = current_np();
		//for all
		for (size_t i = 0; i != np; ++i)
		{
			execute_generation_task(i);
		}
		//swap
		m_e_method->selection(m_population);
	}
	void DennAlgorithm::parallel_execute_pass(ThreadPool& thpool)
	{
		//get np
		size_t np = current_np();
		//alloc promises
		m_promises.resize(np);
		//execute
		for (size_t i = 0; i != np; ++i)
		{
			//add
			m_promises[i] = thpool.push_task([this, i]()
			{
				execute_generation_task(i);
			});
		}
		//wait
		for (auto& promise : m_promises) promise.wait();
		//swap
		m_e_method->selection(m_population);
	}
	void DennAlgorithm::execute_generation_task(size_t i)
	{
		//ref to sons
		auto& parents = m_population.parents();
		auto& sons = m_population.sons();
		//get temp individual
		auto& parent = parents[i];
		auto& son = sons[i];
		//Compute new individual
		m_e_method->create_a_individual(m_population, i, *son);
		//test
		if(*m_params.m_use_mask)
		{
			//net
			son->m_network.apply_mask(m_nnmask, parent->m_network);
		}
		//eval
		son->m_eval = (*m_loss_function)(son->m_network.feedforward(current_batch().features(), &random(i)),  current_batch());
	}
	
	/////////////////////////////////////////////////////////////////
	//fitness function on a population
	void DennAlgorithm::execute_fitness_on(Population& population) const
	{
		execute_loss_function_on_all_population(population);
	}
	void DennAlgorithm::execute_loss_function_on_all_population(Population& population) const
	{
		//eval on batch
		if (m_thpool) parallel_execute_loss_function_on_all_population(population, *m_thpool);
		else 		  serial_execute_loss_function_on_all_population(population);
	}
	void DennAlgorithm::serial_execute_loss_function_on_all_population(Population& population) const
	{
		//np
		size_t np = current_np();
		//for all
		for (size_t i = 0; i != np; ++i)
		{
			//ref to loss
			auto& i_target = *population[i];
			//eval
			i_target.m_eval = (*m_loss_function)((NeuralNetwork&)i_target, current_batch());
			//safe, nan = worst
			if (std::isnan(i_target.m_eval)) i_target.m_eval = loss_function_worst(); 
		}
	}
	void DennAlgorithm::parallel_execute_loss_function_on_all_population(Population& population, ThreadPool& thpool) const
	{
		//get np
		size_t np = current_np();
		//alloc promises
		m_promises.resize(np);
		//for all
		for (size_t i = 0; i != np; ++i)
		{
			//ref to target
			auto& i_target = *population[i];
			//add
			m_promises[i] = thpool.push_task([this,&i_target]()
			{
				//test
				i_target.m_eval = (*m_loss_function)((NeuralNetwork&)i_target, current_batch());
				//safe, nan = worst
				if (std::isnan(i_target.m_eval)) i_target.m_eval = loss_function_worst(); 
			});
		}
		//wait
		for (auto& promise : m_promises) promise.wait();
	}
	
	/////////////////////////////////////////////////////////////////
	//gen random function
	DennAlgorithm::RandomFunction DennAlgorithm::gen_random_func() const
	{
		if(*m_params.m_distribution == "uniform")
		{
			Scalar min = m_params.m_uniform_min;
			Scalar max = m_params.m_uniform_max;
			return [this,min,max](Scalar x) -> Scalar
			{
				return Scalar(main_random().uniform(min, max));
			};
		}
		else if(*m_params.m_distribution == "normal")
		{
			Scalar mu = m_params.m_normal_mu;
			Scalar sigma = m_params.m_normal_sigma;
			return [this,mu,sigma](Scalar x) -> Scalar
			{
				return Scalar(main_random().normal(mu, sigma));
			};
		}
		else 
		{
			denn_assert(0);
			return [](Scalar) -> Scalar{ return 0; };
		}
	}
	DennAlgorithm::RandomFunctionThread DennAlgorithm::gen_random_func_thread() const
	{		
		if(*m_params.m_distribution == "uniform")
		{
			Scalar min = m_params.m_uniform_min;
			Scalar max = m_params.m_uniform_max;
			return [this,min,max](Scalar x, size_t i) -> Scalar
			{
				return Scalar(population_random(i).uniform(min, max));
			};
		}
		else if(*m_params.m_distribution == "normal")
		{
			Scalar mu = m_params.m_normal_mu;
			Scalar sigma = m_params.m_normal_sigma;
			return [this,mu,sigma](Scalar x, size_t i) -> Scalar
			{
				return Scalar(population_random(i).normal(mu, sigma));
			};
		}
		else 
		{
			denn_assert(0);
			return [](Scalar, size_t i) -> Scalar{ return 0; };
		}
	}
	//gen clamp function	
	DennAlgorithm::ClampFunction DennAlgorithm::gen_clamp_func() const
	{
		Scalar min = m_params.m_clamp_min;
		Scalar max = m_params.m_clamp_max;
		return [min,max](Scalar x) -> Scalar
		{
			return Denn::clamp<Scalar>(x, min, max);
		};
	}
	//load next batch
	bool DennAlgorithm::next_batch()
	{
		m_dataset_batch.read_batch();
		return true;
	}
	/////////////////////////////////////////////////////////////////

}

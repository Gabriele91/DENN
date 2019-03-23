#include "Denn/Parameters.h"
#include "Denn/Denn/Solver.h"

namespace Denn
{
    Solver::Solver(Instance&	instance, const Parameters&   params)
	: m_main_random(instance.random_engine())
    , m_thpool(instance.thread_pool())
	, m_loss_function(instance.loss_function())
	, m_validation_function(instance.validation_function())
	, m_test_function(instance.test_function())
	, m_output(RuntimeOutputFactory::create(*params.m_runtime_output_type, 
											 instance.output_stream(), *this))
	, m_start_network(instance.neural_network())
	, m_dataset_loader(&instance.dataset_loader())
	, m_dataset_batch(&instance.dataset_loader())
	, m_paramters(params)
	{
	}

	/////////////////////////////////////////////////
	//info
	const Parameters& Solver::parameters() const { return m_paramters; }
	const Population& Solver::population() const{ return *m_population; }
	const EvolutionMethodList& Solver::evolution_methods() const{ return m_e_methods; }
	const Solver::ClampFunction& Solver::clamp_function() const{ return m_clamp_function;  }
	Random& Solver::random(size_t i) const{ return m_population_random[i]; }
	Random& Solver::random() const{ return m_main_random; }

	const DataSetScalar& Solver::current_batch() const{ return m_dataset_batch.last_batch(); }
	const DataSetLoader* Solver::get_datase_loader() const{ return m_dataset_loader; }

	Scalar Solver::loss_function_eval(Individual& i, size_t thread_id ) const
	{
		return 	(*loss_function())(*build_neural_network(i, thread_id), current_batch());
	}
	Scalar Solver::validation_function_eval(Individual& ind, size_t thread_id) const
	{
		DataSetScalar validation;
		m_dataset_loader->read_validation(validation);
		return 	(*validation_function())(*build_neural_network(ind, thread_id), validation);
	}
	Scalar Solver::test_function_eval(Individual& ind, size_t thread_id) const
	{
		DataSetScalar test;
		m_dataset_loader->read_test(test);
		return 	(*test_function())(*build_neural_network(ind, thread_id), test);
	}

	Scalar Solver::loss_function_eval(NeuralNetwork& nn) const
	{
		return 	(*loss_function())(nn, current_batch());
	}
	Scalar Solver::validation_function_eval(NeuralNetwork& nn) const
	{
		DataSetScalar validation;
		m_dataset_loader->read_validation(validation);
		return 	(*validation_function())(nn, validation);
	}
	Scalar Solver::test_function_eval(NeuralNetwork& nn) const
	{
		DataSetScalar test;
		m_dataset_loader->read_test(test);
		return 	(*test_function())(nn, test);
	}

	Evaluation::SPtr Solver::loss_function() const{ return m_loss_function; }
	Evaluation::SPtr Solver::validation_function() const{ return m_validation_function; }
	Evaluation::SPtr Solver::test_function() const{ return m_test_function; }
	
	bool Solver::loss_function_compare(Scalar left, Scalar right) const
	{
		return  m_loss_function->minimize() 
		?  left < right
		:  right < left
		;
	}
	bool Solver::validation_function_compare(Scalar left, Scalar right) const
	{
		return  m_validation_function->minimize() 
		?  left < right
		:  right < left
		;
	}
	bool Solver::test_function_compare(Scalar left, Scalar right) const
	{
		return  m_test_function->minimize() 
		?  left < right
		:  right < left
		;
	}

	Scalar Solver::loss_function_worst() const
	{
		return  m_loss_function->minimize() 
		?   std::numeric_limits<Scalar>::max() 
		:  -std::numeric_limits<Scalar>::max()
		;
	}
	Scalar Solver::validation_function_worst() const
	{
		return  m_validation_function->minimize() 
		?   std::numeric_limits<Scalar>::max() 
		:  -std::numeric_limits<Scalar>::max()
		;
	}
	Scalar Solver::test_function_worst() const
	{
		return  m_test_function->minimize() 
		?   std::numeric_limits<Scalar>::max() 
		:  -std::numeric_limits<Scalar>::max()
		;
	}
	/////////////////////////////////////////////////
	//fit
	const NeuralNetwork& Solver::fit()
	{
		//init all
		if (!init()) return m_start_network;
		//global info
		const size_t n_global_pass = ((size_t)parameters().m_generations / 
									  (size_t)parameters().m_sub_gens);
		const size_t n_sub_pass = parameters().m_sub_gens;
		//default best
		execute_update_best(true);
		//start output
		m_output->start();
		//main loop
		for (size_t pass = 0; pass != n_global_pass; ++pass)
		{
			execute_a_pass(pass, n_sub_pass);
			//next
			next_batch();
		}
		//end output
		m_output->end();
		//result
		return *best.network;
	}
	//Init solver
    bool Solver::init()
    {		
        //success flag
		bool success = m_dataset_loader != nullptr;
		//init test set
		if(*parameters().m_batch_offset <= 0)
			m_dataset_batch.start_read_batch(*parameters().m_batch_size, *parameters().m_batch_size);
		else
			m_dataset_batch.start_read_batch(*parameters().m_batch_size, *parameters().m_batch_offset);
		//init random engine
		m_main_random.reinit(*parameters().m_seed);
		//gen clamp functions
		m_clamp_function = gen_clamp_func();
		//clear random engines
		m_population_random.clear();
		//vecotr np
		std::vector<size_t> nps;
		//cases
        for(size_t l = 0; l < m_start_network.size(); ++l)
        for(size_t m = 0; m < m_start_network[l].size(); ++m)
		{
			if(*parameters().m_np_perc > Scalar(0))
			{
				nps.push_back(size_t(m_start_network[l][m].rows() * *parameters().m_np_perc));
			}
			else 
			{
				nps.push_back(*parameters().m_np);
			}
		}
		//get np
		const size_t max_np = *std::max_element(std::begin(nps), std::end(nps)); //init current_np();
		//min size
		if (!max_np) return false;
		//init random engines
		for(size_t i=0; i != max_np ;++i)
		{
			m_population_random.emplace_back(random().uirand());
		}
		//init attributes
		Attributes attributes
		{
			*parameters().m_default_f,
			*parameters().m_default_cr,
			*parameters().m_perc_of_best,
		};
		//init pop
		m_population = std::make_shared<Population>(nps, attributes, m_start_network);
        //init 
        PromiseList promises;
		//random init function
		auto thread_random = gen_random_func_thread();
        //parallel
        for (SubPopulation::SPtr subpop : (*m_population)) //for each sub population
        {
			//target parent
			int i_target = 0;
			//list individuals
			auto& parents = subpop->parents();
			//for each individual
            for (auto individual : parents) //for each parents
            {
                promises.push_back(m_thpool->push_task([this, thread_random, individual, i_target]()
                {
                    //build fun
                    auto rand_weight = [=](Scalar weight) -> Scalar
                    { 
                        const Scalar eps = SCALAR_EPS;
                        do weight = thread_random(weight, i_target); while(std::abs(weight) <= eps);
                        return weight;
                    };
                    //init
                    individual->matrix() = individual->matrix().unaryExpr(rand_weight);
					individual->eval() = loss_function_worst();
                }));
				//next
				++i_target;
            }
            //wait
            for (auto& p : promises) p.wait();
        }
		//methods
        for (SubPopulation::SPtr subpop : population())
        {
		    //alloc
            m_e_methods.push_back(EvolutionMethodFactory::create(parameters().m_evolution_type, *this, *subpop));
		    //reset method
		    m_e_methods.back()->start();
        }
		//true
		return true;
	}
	//Denn
	void Solver::execute_a_pass(size_t pass, size_t n_sub_pass)
	{
		///////////////////////////////////////////////////////////////////
		if (*parameters().m_reval_pop_on_batch || pass == 0) 
			loss_function_eval_all();
		///////////////////////////////////////////////////////////////////
		//output
		m_output->start_a_pass();
		//start pass
		for(auto emethod : evolution_methods())
			emethod->start_a_gen_pass();
		//sub pass
		for (size_t sub_pass = 0; sub_pass != n_sub_pass; ++sub_pass)
		{
			execute_a_sub_pass(pass, sub_pass);
		}
		//end pass
		for(auto emethod : evolution_methods())
			emethod->end_a_gen_pass();
		//update context
		execute_update_best();
		//output
		m_output->end_a_pass();
	}
	void Solver::execute_a_sub_pass(size_t pass, size_t sub_pass)
	{
		//output
		m_output->start_a_sub_pass();
		//pass
		execute_pass();
		//output
		m_output->end_a_sub_pass();
	}
	void Solver::execute_pass()
	{
		//it ev
		auto it_emethod = m_e_methods.begin();
		//execute
		for (SubPopulation::SPtr subpop : population())           
		{
			//ref
			EvolutionMethod* emethod = (*it_emethod).get();
			//start
			emethod->start_a_subgen_pass();
			//alloc promises
			m_promises.resize(subpop->size());
			//subpop
			for (size_t i = 0; i < subpop->size(); ++i)
			{
				m_promises[i] = m_thpool->push_task([emethod, i, subpop]()
				{
					emethod->create_a_individual(i, *(*subpop)[i].son());
				});
			}
			//wait
			for (auto& promise : m_promises) promise.wait();
			//swap
			emethod->selection();		
			emethod->end_a_subgen_pass();
			//next
			++it_emethod;
		}	
	}
	/////////////////////////////////////////////////////////////////
	void Solver::loss_function_eval_all()
	{
		for (SubPopulation::SPtr subpop : population()) 
		for (size_t i = 0; i < subpop->size(); ++i)
		{
			auto parent = (*subpop)[i].parent();
			parent->eval() = loss_function_eval(*parent);
		}
	}
	//load next batch
	bool Solver::next_batch()
	{
		m_dataset_batch.read_batch();
		return true;
	}
	/////////////////////////////////////////////////////////////////
	void Solver::execute_update_best(bool first)
	{
		//netowrk
		auto network = build_neural();
		//eval
		auto eval = *parameters().m_use_validation  
					? validation_function_eval(network) 
					: loss_function_eval(network);
		//compare
		if ((*parameters().m_use_validation && validation_function_compare(eval, best.eval) ||
		    (!*parameters().m_use_validation && loss_function_compare(eval, best.eval)  ) ||
			first))
		{
			best.eval = eval;
			best.network = network;
		}
	}
	NeuralNetwork::SPtr Solver::build_neural() const
	{
		NeuralNetwork::SPtr newnn = m_start_network.copy();
        size_t subpop_id = 0;
        for(size_t l = 0; l < m_start_network.size(); ++l)
        for(size_t m = 0; m < m_start_network[l].size(); ++m)
		{
			SubPopulation::SPtr subpop = population()[subpop_id++];
			Individual::SPtr individual = subpop->parents()[subpop->best_parent_id()];
			(*newnn)[l][m] = individual->matrix();
		}
		return newnn;
	}
	NeuralNetwork::SPtr Solver::build_neural_network(Individual& ind, size_t thread_id) const 
	{
		NeuralNetwork::SPtr newnn = nullptr;
		{
			std::lock_guard<std::mutex>  lock(m_mutex);
			newnn = m_start_network.copy();
		}
        size_t subpop_id = 0;
        for(size_t l = 0; l < m_start_network.size(); ++l)
        for(size_t m = 0; m < m_start_network[l].size(); ++m)
		{
			//cases
			if(ind.subpopulation()->layer_id() == l && ind.subpopulation()->matrix_id() == m)
			{
				(*newnn)[l][m] = ind.matrix();
				subpop_id++;
			}
			else
			{
				//ref
				SubPopulation::SPtr subpop = population()[subpop_id++];
				Individual::SPtr individual = subpop->parents()[subpop->best_parent_id()];
				(*newnn)[l][m] = individual->matrix();
			}
		}
		return newnn;
	}
	/////////////////////////////////////////////////////////////////
	//gen random function
	Solver::RandomFunction Solver::gen_random_func() const
	{
		if(*parameters().m_distribution == "uniform")
		{
			Scalar min = parameters().m_uniform_min;
			Scalar max = parameters().m_uniform_max;
			return [this,min,max](Scalar x) -> Scalar
			{
				return Scalar(random().uniform(min, max));
			};
		}
		else if(*parameters().m_distribution == "normal")
		{
			Scalar mu = parameters().m_normal_mu;
			Scalar sigma = parameters().m_normal_sigma;
			return [this,mu,sigma](Scalar x) -> Scalar
			{
				return Scalar(random().normal(mu, sigma));
			};
		}
		else 
		{
			denn_assert(0);
			return [](Scalar) -> Scalar{ return 0; };
		}
	}
	Solver::RandomFunctionThread Solver::gen_random_func_thread() const
	{		
		if(*parameters().m_distribution == "uniform")
		{
			Scalar min = parameters().m_uniform_min;
			Scalar max = parameters().m_uniform_max;
			return [this,min,max](Scalar x, size_t i) -> Scalar
			{
				return Scalar(random(i).uniform(min, max));
			};
		}
		else if(*parameters().m_distribution == "normal")
		{
			Scalar mu = parameters().m_normal_mu;
			Scalar sigma = parameters().m_normal_sigma;
			return [this,mu,sigma](Scalar x, size_t i) -> Scalar
			{
				return Scalar(random(i).normal(mu, sigma));
			};
		}
		else 
		{
			denn_assert(0);
			return [](Scalar, size_t i) -> Scalar{ return 0; };
		}
	}
	//gen clamp function	
	Solver::ClampFunction Solver::gen_clamp_func() const
	{
		Scalar min = parameters().m_clamp_min;
		Scalar max = parameters().m_clamp_max;
		return [min,max](Scalar x) -> Scalar
		{
			return Denn::clamp<Scalar>(x, min, max);
		};
	}
}
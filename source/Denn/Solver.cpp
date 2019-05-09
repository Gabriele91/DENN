#include "Denn/Parameters.h"
#include "Denn/Denn/Solver.h"

namespace Denn
{
    Solver::Solver(Instance&	instance, const Parameters&   params)
	: m_paramters(params)
	, m_main_random(instance.random_engine())
    , m_thpool(instance.thread_pool())
	//eval funcs
	, m_loss_function(instance.loss_function())
	, m_validation_function(instance.validation_function())
	, m_test_function(instance.test_function())
	//networks
	, m_start_network(instance.neural_network())
	//dataset
	, m_dataset_loader(&instance.dataset_loader())
	, m_dataset_batch(&instance.dataset_loader())
	//output
	, m_output(RuntimeOutputFactory::create(*params.m_runtime_output_type, 
											 instance.output_stream(), *this))
	{
	}

	//Init solver
    bool Solver::init()
    {		
        //success flag
		if(m_dataset_loader == nullptr) return false;
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
		//conet build mode
		int split_type = SplitNetwork::get(*parameters().m_conet_split);
		//vecotr np
		PopulationDescription popdecs;
		switch (split_type)
		{
		default:
		case SplitNetwork::SN_MATRIX: popdecs = matrix_sub_population(); break;
		case SplitNetwork::SN_LAYER: popdecs = layer_sub_population(); break;
		case SplitNetwork::SN_ONE: popdecs = one_sub_population(); break;
		}
		//get np
		const size_t max_np = compute_max_np(popdecs); //init current_np();
		//min size
		if (!max_np) return false;
		//init random engines
		for(size_t i=0; i != max_np ;++i)
			m_population_random.emplace_back(random().uirand());
		//conet build mode
		best.build_type = BuildNetwork::get(*parameters().m_conet_build);
		//init attributes
		Attributes attributes
		{
			*parameters().m_default_f,
			*parameters().m_default_cr,
			*parameters().m_perc_of_best,
		};
		//init pop
		m_population = std::make_shared<Population>(popdecs, attributes, m_start_network);
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
                    individual->array() = individual->array().unaryExpr(rand_weight);
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
		//force to test on validation 
		if(*parameters().m_last_with_validation && !*parameters().m_use_validation)
		{
			best.eval = validation_function_eval(best.network,/*no training_phase*/ false) ;
		}
		//end output
		m_output->end();
		//result
		return *best.network;
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
	//eval
	void Solver::loss_function_eval_all()
	{
		for (SubPopulation::SPtr subpop : population()) 
		for (size_t i = 0; i < subpop->size(); ++i)
		{
			auto parent = (*subpop)[i].parent();
			parent->eval() = loss_function_eval(*parent);
		}
	}
	void Solver::execute_update_best(bool first)
	{
		//netowrk
		auto network = build_neural();
		//eval
		auto eval = *parameters().m_use_validation  
					? validation_function_eval(network,/*no training_phase*/ false) 
					: loss_function_eval(network,/*no training_phase*/ false) ;
		//compare
		if ((*parameters().m_use_validation && validation_function_compare(eval, best.eval))||
		    (!*parameters().m_use_validation && loss_function_compare(eval, best.eval)) ||
			first)
		{
			best.eval = eval;
			best.network = network;
		}
	}

}
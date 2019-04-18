#pragma once
#include "Config.h"
#include "Evaluation.h"
#include "DataSetLoader.h"
#include "Parameters.h"
#include "Population.h"
#include "Mutation.h"
#include "Crossover.h"
#include "EvolutionMethod.h"
#include "RuntimeOutput.h"
#include "TestSetStream.h"
#include "Instance.h"

namespace Denn
{
//Def DennAlgorithm
class DennAlgorithm
{
public:
	////////////////////////////////////////////////////////////////////////
	//Alias
	using LayerList      = typename NeuralNetwork::LayerList;
	//Search space
	using DBPopulation         = DoubleBufferPopulation;
	using RandomFunction       = std::function<Scalar(Scalar)>;
	using RandomFunctionThread = std::function<Scalar(Scalar, size_t)>;
	//Ref mutation crossover
	using ClampFunction  = std::function<Scalar(Scalar)>;
	//Vector of random
	using RandomList     = std::vector< Random >;
	//Ref mutation
	////////////////////////////////////////////////////////////////////////
	//structs utilities
	struct RestartContext
	{
		Scalar     m_last_eval;
		size_t	   m_test_count;
		size_t	   m_count;
        //
		RestartContext(
			  Scalar last_eval  = Scalar(0.0)
			, size_t	 test_count = 0
			, size_t	 count      = 0
		)
		{
			m_last_eval  = last_eval;
			m_test_count = test_count;
			m_count      = count;
		}
	};
	struct BestContext
	{
		Individual::SPtr m_best;
		Scalar           m_eval;
        //
		BestContext(Individual::SPtr best = nullptr, Scalar eval = 0)
		{
			m_best = best;
			m_eval = eval;
		}
	};
	////////////////////////////////////////////////////////////////////////
	DennAlgorithm(Instance&	instance, const Parameters&   params);	

	//big loop
	virtual Individual::SPtr execute();

	//find best individual (validation test)
	bool find_best_on_validation(size_t& out_i, Scalar& out_eval);
	
	//find best individual (validation test)
	Individual::SPtr find_best_on_validation(Scalar& out_eval);

	//fitness function on a population
	void execute_fitness_on(Population& population) const;

	//using the test set on a individual
	Scalar execute_test() const;
	Scalar execute_test(Individual& individual) const;

	//info
	const Parameters& parameters() const
	{
		return m_params;
	}

	const size_t current_np() const
	{
		return m_population.size();
	}

	const DBPopulation& population() const
	{
		return m_population;
	}

	const EvolutionMethod& evolution_method() const
	{
		return *m_e_method;
	}

	const Individual::SPtr get_default_individual() const
	{
		return m_default;
	}

	const ClampFunction& clamp_function() const
	{
		return m_clamp_function;
	}

	const BestContext& best_context() const
	{
		return m_best_ctx;
	}

	const RestartContext& restart_context() const
	{
		return m_restart_ctx;
	}

	Random& population_random(size_t i) const
	{
		return m_population_random[i];
	}

	Random& main_random() const
	{
		return m_main_random;
	}

	Random& random(size_t i) const
	{
		return m_population_random[i];
	}

	Random& random() const
	{
		return m_main_random;
	}

	const DataSetScalar& current_batch() const
	{
		return m_dataset_batch.last_batch();
	}

	const DataSetLoader* get_datase_loader() const 
	{
		return m_dataset_loader;
	}

	Evaluation::SPtr loss_function() const 
	{
		return m_loss_function;
	}
	
	Evaluation::SPtr validation_function() const 
	{
		return m_validation_function;
	}

	Evaluation::SPtr test_function() const 
	{
		return m_test_function;
	}
	
	bool loss_function_compare(Scalar left, Scalar right) const
	{
		return  m_loss_function->minimize() 
		    ?   left < right
			:  right < left
			;
	}

	bool validation_function_compare(Scalar left, Scalar right) const
	{
		return  m_validation_function->minimize() 
		    ?   left < right
			:  right < left
			;
	}

	bool test_function_compare(Scalar left, Scalar right) const
	{
		return  m_test_function->minimize() 
		    ?  left < right
			:  right < left
			;
	}

	Scalar loss_function_worst() const
	{
		return  m_loss_function->minimize() 
		    ?   std::numeric_limits<Scalar>::max() 
			:  -std::numeric_limits<Scalar>::max()
			;
	}

	Scalar validation_function_worst() const
	{
		return  m_validation_function->minimize() 
		    ?   std::numeric_limits<Scalar>::max() 
			:  -std::numeric_limits<Scalar>::max()
			;
	}

	Scalar test_function_worst() const
	{
		return  m_test_function->minimize() 
		    ?   std::numeric_limits<Scalar>::max() 
			:  -std::numeric_limits<Scalar>::max()
			;
	}

protected:
	//init
	bool init();
	bool init_population();
	/////////////////////////////////////////////////////////////////
	//tests (validation)
	bool serial_find_best_on_validation(size_t& out_i, Scalar& out_eval);
	bool parallel_find_best_on_validation(ThreadPool& thpool, size_t& out_i, Scalar& out_eval);
	/////////////////////////////////////////////////////////////////
	//Intermedie steps
	void execute_a_pass(size_t pass, size_t n_sub_pass);
	void execute_a_sub_pass(size_t pass, size_t sub_pass);
	void execute_update_best(int pass, int sub_pass);
	void execute_update_mask(int pass, int sub_pass);
	void execute_update_best_on_validation();
	void execute_update_best_on_loss_function();
	void execute_update_restart(size_t pass);
	/////////////////////////////////////////////////////////////////
	//execute a pass
	void execute_pass();
	void serial_execute_pass();
	void parallel_execute_pass(ThreadPool& thpool);
	void  execute_generation_task(size_t i);
	/////////////////////////////////////////////////////////////////
	//eval all
	void execute_loss_function_on_all_population(Population& population) const;
	void serial_execute_loss_function_on_all_population(Population& population) const;
	void parallel_execute_loss_function_on_all_population(Population& population,ThreadPool& thpool) const;
	/////////////////////////////////////////////////////////////////
	//gen random function
	RandomFunction       gen_random_func() const;
	RandomFunctionThread gen_random_func_thread() const;
	//gen clamp function
	ClampFunction gen_clamp_func() const;
	//load next batch
	bool next_batch();
	/////////////////////////////////////////////////////////////////
	//Random engine
	Random&	m_main_random;
	mutable RandomList m_population_random;
	//multi threads
	ThreadPool*			  m_thpool;
	mutable PromiseList	  m_promises;
	//serach space
	DBPopulation          m_population;
	Evaluation::SPtr      m_loss_function;
	Evaluation::SPtr      m_validation_function;
	Evaluation::SPtr      m_test_function;
	RuntimeOutput::SPtr   m_output;
	//mask
	NeuralNetwork         m_nnlast;
	NeuralNetwork         m_nnmask;
	Scalar			      m_nnlast_eval{0};
	int					  m_nnmask_count{0};
	//dataset
	Individual::SPtr      m_default;
	DataSetLoader*		  m_dataset_loader;
	TestSetStream         m_dataset_batch;
	//Execution Context
	BestContext		      m_best_ctx;
	RestartContext		  m_restart_ctx;
	//params of DE
	Parameters 		      m_params;
	EvolutionMethod::SPtr m_e_method;
	//function for DE
	ClampFunction		  m_clamp_function;
};

}
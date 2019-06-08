#pragma once
#include <sstream>
#include <iterator>
#include "Denn/Config.h"
#include "Denn/Instance.h"
#include "Denn/Core/Json.h"
#include "Denn/DataSet/DataSetLoader.h"
#include "Denn/DataSet/TestSetStream.h"
#include "Population.h"
#include "EvolutionMethod.h"
#include "Solver_config.h"

namespace Denn
{

class Solver
{
public:
	////////////////////////////////////////////////////////////////////////
	//Alias
	using RandomFunction       = std::function<Scalar(Scalar)>;
	using RandomFunctionThread = std::function<Scalar(Scalar, size_t)>;
	//Ref mutation crossover
	using ClampFunction = std::function<Scalar(Scalar)>;
	//Vector of random
	using RandomList = std::vector< Random >;
    ////////////////////////////////////////////////////////////////////////
    //init
    Solver(Instance& instance, const Parameters& params);

	//optimize
	virtual const NeuralNetwork& fit();

    //info
	const Parameters& parameters() const;
	const Population& population() const;
	const EvolutionMethodList& evolution_methods() const;
	const ClampFunction& clamp_function() const;

    //random
	Random& random(size_t i) const;
	Random& random() const;

    //dataset    
	const DataSetScalar& current_batch() const;
	const DataSetLoader* get_datase_loader() const;

    //eval
	Scalar loss_function_eval(Individual& i, size_t thread_id = 0, bool training_phase=true) const;
	Scalar validation_function_eval(Individual& i, size_t thread_id = 0, bool training_phase=true) const;
	Scalar test_function_eval(Individual& i, size_t thread_id = 0, bool training_phase=true) const;

	Scalar loss_function_eval(NeuralNetwork& n, bool training_phase=true) const;
	Scalar validation_function_eval(NeuralNetwork& n, bool training_phase=true) const;
	Scalar test_function_eval(NeuralNetwork& n, bool training_phase=true) const;

	Scalar loss_function_eval(NeuralNetwork::SPtr n, bool training_phase=true) const { return loss_function_eval(*n, training_phase);  }
	Scalar validation_function_eval(NeuralNetwork::SPtr n, bool training_phase=true) const { return validation_function_eval(*n, training_phase);  }
	Scalar test_function_eval(NeuralNetwork::SPtr n, bool training_phase=true) const { return test_function_eval(*n, training_phase);  }
	
	const NeuralNetwork::SPtr current_best_network() const { return best.network; }
	const Scalar current_best_eval() const { return best.eval; }

	bool loss_function_compare(Scalar left, Scalar right) const;
	bool validation_function_compare(Scalar left, Scalar right) const;
	bool test_function_compare(Scalar left, Scalar right) const;

	Evaluation::SPtr loss_function() const;
	Evaluation::SPtr validation_function() const;
	Evaluation::SPtr test_function() const;

	Scalar loss_function_worst() const;
	Scalar validation_function_worst() const;
	Scalar test_function_worst() const;

protected:
	//help
	size_t n_sup_pass() const ;
	size_t n_pass() const ;
    //funcs
	bool init();
    //Denn
    void execute_a_pass(size_t pass, size_t n_sub_pass);
	void execute_a_sub_pass(size_t pass, size_t sub_pass);
	void execute_update_best(bool first=false);
	void execute_pass();
	//statistics
	void execute_statistics(size_t pass, size_t sub_pass);
	//load next batch
	void loss_function_eval_all();
	bool next_batch();
    //utils
	void execute_loss_function_on_all_population(Population &population) const;
	//map population
	PopulationDescription one_sub_population();
	PopulationDescription layer_sub_population();
	PopulationDescription matrix_sub_population();
	//build network
	NeuralNetwork::SPtr build_neural() const;
	NeuralNetwork::SPtr build_neural_network(Individual &i, size_t thread_id = 0) const;
	//all nets
	std::vector<NeuralNetwork::SPtr> build_line_np_neural() const;
	std::vector<NeuralNetwork::SPtr> build_cross_np_neural() const;
	//gen random function
	RandomFunction       gen_random_func() const;
	RandomFunctionThread gen_random_func_thread() const;
	//gen clamp function
	ClampFunction gen_clamp_func() const;
    //parameters
	Parameters m_paramters;
	//Random engine
	Random&	m_main_random;
	mutable RandomList m_population_random;
	//multi threads
	ThreadPool*	m_thpool;
	mutable std::mutex  m_mutex;
	mutable PromiseList m_promises;
    //Denn
    EvolutionMethodList m_e_methods;
    Population::SPtr m_population;
	ClampFunction m_clamp_function;
    //eval
	Evaluation::SPtr m_loss_function;
	Evaluation::SPtr m_validation_function;
	Evaluation::SPtr m_test_function;
	//neural netowrk
	struct 
	{
		BuildNetwork::Value build_type;
		Scalar              eval;
		NeuralNetwork::SPtr network;
	}
	best;
	const NeuralNetwork&  m_start_network;
	//dataset
	DataSetLoader*		  m_dataset_loader;
	TestSetStream         m_dataset_batch;
    //shell info
	RuntimeOutput::SPtr   m_output;
	//statistics info
	Json m_statistics;
};

} // Denn

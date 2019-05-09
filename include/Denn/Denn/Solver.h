#pragma once
#include <sstream>
#include <iterator>
#include "Denn/Config.h"
#include "Denn/Instance.h"
#include "Denn/DataSet/DataSetLoader.h"
#include "Denn/DataSet/TestSetStream.h"
#include "Population.h"
#include "EvolutionMethod.h"


namespace Denn
{
class SplitNetwork
{
public:
	enum Value
	{
		SN_ONE,
		SN_LAYER,
		SN_MATRIX,
		SN_SIZE
	};

	static std::vector<std::string> list_of_splitters()
	{
		return { "one", "layer", "matrix" };
	}

	static std::string names_of_splitters(const std::string& sep = ", ")
	{
		std::stringstream sout;
		auto list = list_of_splitters();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}

	static bool exists(const std::string& value)
	{
		for(auto& type : list_of_splitters()) 
			if(type == value)
				return true;
		return false;
	}

	static Value get(const std::string& value)
	{
		int i=0;
		for(auto& type : list_of_splitters()) 
		{
			if(type == value) return Value(i);
			++i;
		}
		return SN_SIZE;
	}
};

class BuildNetwork
{
public:
	enum Value
	{
		BN_BEST,
		BN_PBEST,
		BN_ROULETTE,
		BN_SIZE
	};

	static std::vector<std::string> list_of_builders()
	{
		return { "best", "pbest", "roulette" };
	}

	static std::string names_of_builders(const std::string& sep = ", ")
	{
		std::stringstream sout;
		auto list = list_of_builders();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}

	static bool exists(const std::string& value)
	{
		for(auto& type : list_of_builders()) 
			if(type == value)
				return true;
		return false;
	}

	static Value get(const std::string& value)
	{
		int i=0;
		for(auto& type : list_of_builders()) 
		{
			if(type == value) return Value(i);
			++i;
		}
		return BN_SIZE;
	}
};

class GlobalSelectionNetwork
{
public:
	enum Value
	{
		GN_BEST,
		GN_LINE,
		GN_CROSS,
		GN_SIZE
	};

	static std::vector<std::string> list_of_global_selectors()
	{
		return {"best", "line", "cross"};
	}

	static std::string names_of_global_selectors(const std::string &sep = ", ")
	{
		std::stringstream sout;
		auto list = list_of_global_selectors();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}

	static bool exists(const std::string &value)
	{
		for (auto &type : list_of_global_selectors())
			if (type == value)
				return true;
		return false;
	}

	static Value get(const std::string &value)
	{
		int i = 0;
		for (auto &type : list_of_global_selectors())
		{
			if (type == value)
				return Value(i);
			++i;
		}
		return GN_SIZE;
	}
};

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
    //funcs
	bool init();
    //Denn
    void execute_a_pass(size_t pass, size_t n_sub_pass);
	void execute_a_sub_pass(size_t pass, size_t sub_pass);
	void execute_update_best(bool first=false);
	void execute_pass();
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

};

} // Denn

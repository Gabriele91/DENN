#include "Denn/Parameters.h"
#include "Denn/Denn/Solver.h"

namespace Denn
{
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

	Scalar Solver::loss_function_eval(Individual& i, size_t thread_id, bool training_phase) const
	{
		return 	(*loss_function())(*build_neural_network(i, thread_id), current_batch(), training_phase);
	}
	Scalar Solver::validation_function_eval(Individual& ind, size_t thread_id, bool training_phase) const
	{
		DataSetScalar validation;
		m_dataset_loader->read_validation(validation);
		return 	(*validation_function())(*build_neural_network(ind, thread_id), validation, training_phase);
	}
	Scalar Solver::test_function_eval(Individual& ind, size_t thread_id, bool training_phase) const
	{
		DataSetScalar test;
		m_dataset_loader->read_test(test);
		return 	(*test_function())(*build_neural_network(ind, thread_id), test, training_phase);
	}

	Scalar Solver::loss_function_eval(NeuralNetwork& nn, bool training_phase) const
	{
		return 	(*loss_function())(nn, current_batch(), training_phase);
	}
	Scalar Solver::validation_function_eval(NeuralNetwork& nn, bool training_phase) const
	{
		DataSetScalar validation;
		m_dataset_loader->read_validation(validation);
		return 	(*validation_function())(nn, validation, training_phase);
	}
	Scalar Solver::test_function_eval(NeuralNetwork& nn, bool training_phase) const
	{
		DataSetScalar test;
		m_dataset_loader->read_test(test);
		return 	(*test_function())(nn, test, training_phase);
	}

	Evaluation::SPtr Solver::loss_function() const{ return m_loss_function; }
	Evaluation::SPtr Solver::validation_function() const{ return m_validation_function; }
	Evaluation::SPtr Solver::test_function() const{ return m_test_function; }
	
	bool Solver::loss_function_compare(Scalar left, Scalar right) const
	{
		return  m_loss_function->minimize() 
		?  left <= right
		:  right <= left
		;
	}
	bool Solver::validation_function_compare(Scalar left, Scalar right) const
	{
		return  m_validation_function->minimize() 
		?  left <= right
		:  right <= left
		;
	}
	bool Solver::test_function_compare(Scalar left, Scalar right) const
	{
		return  m_test_function->minimize() 
		?  left <= right
		:  right <= left
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
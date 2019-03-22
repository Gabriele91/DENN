#include "Denn/Config.h"
#include "Denn/Instance.h"
#include "Denn/Evaluation.h"
#include "Denn/RuntimeOutput.h"
#include "Denn/SerializeOutput.h"
#include "Denn/DataSet/DataSet.h"
#include "Denn/DataSet/DataSetLoader.h"
#include "Denn/DataSet/TestSetStream.h"
#include "Denn/Utilities/Build.h"
#include "Denn/Utilities/Networks.h"
#include <fstream>

namespace Denn
{
	class BackpropagationInstance : public Instance
	{
	public:
		//local info
		std::ofstream  m_runtime_output_file_stream;  //n.b. before of context dec
		std::ofstream  m_serialize_output_file_stream;//n.b. before of context dec
		bool		   m_success_init{ false };
		//context
		const Denn::Parameters&	      m_parameters;    //< parameters
		mutable DataSetLoader::SPtr   m_dataset;	   //< dataset
		mutable NeuralNetwork         m_network;	   //< default network
		mutable Random				  m_random_engine; //< random engine
		mutable SerializeOutput::SPtr m_serialize;     //< serialize output
		mutable std::unique_ptr<ThreadPool> m_pool{ nullptr };     //< thread pool
		mutable std::ostream   m_runtime_output_stream{ nullptr }; //< stream
		//init
		BackpropagationInstance(const Denn::Parameters& parameters) : m_parameters(parameters)
		{
			////////////////////////////////////////////////////////////////////////////////////////////////
			//init
			m_random_engine.reinit(parameters.m_seed);
			////////////////////////////////////////////////////////////////////////////////////////////////
			//threads
			if (!build_thread_pool(m_pool, parameters)) return;
			//stream
			if (!build_outputstream(m_runtime_output_stream, m_runtime_output_file_stream, parameters)) return;
			//serialize
			if (!build_serialize(m_serialize, m_serialize_output_file_stream, parameters))  			return;
			////////////////////////////////////////////////////////////////////////////////////////////////
			//Dataset
			if (!(*parameters.m_dataset_filename).size()) return;
			//test file
			if (!Denn::Filesystem::exists((const std::string&)parameters.m_dataset_filename))
			{
				std::cerr << "input file: \"" << *parameters.m_dataset_filename << "\" does does not exists!" << std::endl;
				return; //exit
			}
			//get loader
			m_dataset = get_datase_loader((const std::string&)parameters.m_dataset_filename);
			//test loader
			if (!m_dataset)
			{
				std::cerr << "input file: \"" << *parameters.m_dataset_filename << "\" does not supported!" << std::endl;
				return; //exit
			}
			//network	
			size_t n_features = m_dataset->get_main_header_info().m_n_features;
			size_t n_class = m_dataset->get_main_header_info().m_n_classes;
			m_network = std::get<0>(get_network_from_string(parameters.m_network, n_features, n_class));
			////////////////////////////////////////////////////////////////////////////////////////////////
			m_success_init = true;
		}

		Random&  random_engine()  const override
		{
			return m_random_engine;
		}

		const NeuralNetwork&  neural_network() const override
		{
			return m_network;
		}

		DataSetLoader& dataset_loader() const override
		{
			return *m_dataset;
		}

		Evaluation::SPtr loss_function() const override
		{
			return EvaluationFactory::get("cross_entropy");
		}

		Evaluation::SPtr validation_function() const override
		{
			return  EvaluationFactory::get("accuracy");
		}

		Evaluation::SPtr test_function() const override
		{
			return  EvaluationFactory::get("accuracy");
		}

		std::ostream&  output_stream() const override
		{
			return m_runtime_output_stream;
		}

		SerializeOutput::SPtr serialize_output() const override
		{
			return m_serialize;
		}

		ThreadPool*	thread_pool() const override
		{
			return m_pool.get();
		}

		bool execute() override
		{
			////////////////////////////////////////////////////////////////////////////////////////////////
			if (!m_success_init) return false;
			////////////////////////////////////////////////////////////////////////////////////////////////
			//init random engine
			m_random_engine.reinit(m_parameters.m_seed);
			//init rand funcs
			std::function< Scalar(Scalar) > random_function;
			//init
			if(*m_parameters.m_distribution == "uniform")
			{
				Scalar min = m_parameters.m_uniform_min;
				Scalar max = m_parameters.m_uniform_max;
				random_function = [this,min,max](Scalar x) -> Scalar
				{
					return Scalar(m_random_engine.uniform(min, max));
				};
			}
			else if(*m_parameters.m_distribution == "normal")
			{
				Scalar mu = m_parameters.m_normal_mu;
				Scalar sigma = m_parameters.m_normal_sigma;
				random_function = [this,mu,sigma](Scalar x) -> Scalar
				{
					return Scalar(m_random_engine.normal(mu, sigma));
				};
			}
			else 
			{
				denn_assert(0);
			}
			//init clamp
			auto clamp_function = [this](Scalar x) -> Scalar
			{ 
				return clamp<Scalar>(x, m_parameters.m_clamp_min, m_parameters.m_clamp_max);
			};
			//random init network
			for (int l = 0; l < m_network.size(); ++l)
			{
				//layer
				Layer& layer = m_network[l];
				//for ach matrix in layer
				for (int i = 0; i < layer.size(); ++i)
				{
					//matrix
					auto vector = layer[i].array();
					//apply random
					for (int i = 0; i < vector.size(); ++i)
					{
						vector(i) = clamp_function(random_function(vector(i)));
					}
				}
			}
			//init batch system 
			TestSetStream batch_stream(&dataset_loader());
			if (*m_parameters.m_batch_offset <= 0)
				batch_stream.start_read_batch(m_parameters.m_batch_size, m_parameters.m_batch_size);
			else
				batch_stream.start_read_batch(m_parameters.m_batch_size, m_parameters.m_batch_offset);
			//init loss
			Scalar batch_eval = std::numeric_limits<Scalar>::max();
			//execute
			double execute_time = Time::get_time();
			output_stream() << "=== BACKPROPAGATION START ===" << std::endl;
			{
				//optimizer
				SGD sgd(m_parameters.m_learning_rate, 
						m_parameters.m_decay,
						m_parameters.m_momentum, 
						m_parameters.m_nesterov
						);
				//for each batch
				for (int epoch = 0; epoch < *m_parameters.m_generations; ++epoch)
				{
					auto batch = batch_stream.read_batch();			
					//update
					m_network.fit
					(
						batch.features(),
						batch.labels(),
						sgd
					);
					//loss update
					batch_eval = (*loss_function())(m_network, batch);
					//std output
					if (epoch % *m_parameters.m_sub_gens == 0)
					{
						output_stream() << "epoch: " << epoch << ", loss: " << batch_eval << std::endl;
					}
				}

			}
			execute_time = Time::get_time() - execute_time;
			//test
			DataSetScalar test_set;
			m_dataset->read_test(test_set);
			Scalar test_eval = (*test_function())(m_network, test_set);
			//std output
			output_stream() << "test: " << test_eval << std::endl;
			output_stream() << "=== BACKPROPAGARION END ===" << std::endl;
			//serialize
			m_serialize->serialize_parameters(m_parameters);
			m_serialize->serialize_best
			(
				  execute_time
				, test_eval
				, m_network
			);
			//success
			return true;
		}
	};
	REGISTERED_INSTANCE(BackpropagationInstance, "backpropagation")
}
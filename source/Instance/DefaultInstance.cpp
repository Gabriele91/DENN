#include "Denn/Config.h"
#include "Denn/Instance.h"
#include "Denn/Evaluation.h"
#include "Denn/RuntimeOutput.h"
#include "Denn/SerializeOutput.h"
#include "Denn/DataSet.h"
#include "Denn/DataSetLoader.h"
#include "Denn/Algorithm.h"
#include "Denn/Utilities/Networks.h"
#include "Denn/Utilities/Build.h"
#include <fstream>

namespace Denn
{
	class DefaultInstance : public Instance
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
		DefaultInstance(const Denn::Parameters& parameters) : m_parameters(parameters)
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
				std::cerr << "input file: \"" << *parameters.m_dataset_filename << "\" not exists!" << std::endl;
				return; //exit
			}
			//get loader
			m_dataset = get_datase_loader((const std::string&)parameters.m_dataset_filename);
			//test loader
			if (!m_dataset)
			{
				std::cerr << "input file: \"" << *parameters.m_dataset_filename << "\" not supported!" << std::endl;
				return; //exit
			}
			//network	
			size_t n_features = m_dataset->get_main_header_info().m_n_features;
			size_t n_class = m_dataset->get_main_header_info().m_n_classes;
			m_network = std::get<0>(get_network_from_string(parameters.m_network, n_features, n_class));
			//build_mlp_network(n_features, n_class, parameters);
			////////////////////////////////////////////////////////////////////////////////////////////////
			m_success_init = true;
		}

		virtual ~DefaultInstance()
		{
			//none	
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
			//DENN
			DennAlgorithm denn(*this, m_parameters);
			//execute
			double execute_time = Time::get_time();
			auto result = denn.execute();
			execute_time = Time::get_time() - execute_time;
			//output
			m_serialize->serialize_parameters(m_parameters);
			m_serialize->serialize_best
			(
				  execute_time
				, denn.execute_test(*result)
				, result->m_f
				, result->m_cr
				, result->m_network
			);
			//save best
			m_network = result->m_network;
			//success
			return true;
		}
	};
	REGISTERED_INSTANCE(DefaultInstance, "default")
}

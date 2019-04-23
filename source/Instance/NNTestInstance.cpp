#include "Denn/Config.h"
#include "Denn/Instance.h"
#include "Denn/Evaluation.h"
#include "Denn/RuntimeOutput.h"
#include "Denn/SerializeOutput.h"
#include "Denn/DataSet.h"
#include "Denn/DataSetLoader.h"
#include "Denn/TestSetStream.h"
#include "Denn/Utilities/Build.h"
#include "Denn/Utilities/Networks.h"
#include <fstream>
#include <iostream>

namespace Denn
{
	class NNTestInstance : public Instance
	{
	public:
		//output
		mutable std::ofstream         m_runtime_output_file_stream;  //n.b. before of context dec
        mutable std::ostream          m_runtime_output_stream{ nullptr }; //< stream
		//context
        mutable NeuralNetwork         m_network;	   //< default network
		//params			
		const Denn::Parameters&	      m_parameters;    //< parameters
        bool  m_success_init{ false };
		int   m_input_1d{ -1 };
		int   m_output_1d{ -1 };

		//init
		NNTestInstance(const Denn::Parameters& parameters) : m_parameters(parameters)
		{
        	////////////////////////////////////////////////////////////////////////////////////////////////
			//Dataset
			if (!(*parameters.m_dataset_filename).size()) return;
			if (!(*parameters.m_network_weights).size()) return;
			//stream
			if (!build_outputstream(m_runtime_output_stream, m_runtime_output_file_stream, parameters)) return;
			//input shape
			if((*parameters.m_features) > 0 && (*parameters.m_classes) > 0)
			{
				m_input_1d =(*parameters.m_features);
				m_output_1d = (*parameters.m_classes);		
			}
			else if (Denn::Filesystem::exists((const std::string&)parameters.m_dataset_filename))
			{		
				//get loader
				DataSetLoader::SPtr dataset = get_datase_loader((const std::string&)parameters.m_dataset_filename);
				m_input_1d = dataset->get_main_header_info().m_n_features;
				m_output_1d = dataset->get_main_header_info().m_n_classes;		
			}
			else //if start as FC and end as FC
			{
				m_input_1d = (*parameters.m_network_weights)[0].rows();
				m_output_1d = (*parameters.m_network_weights).back().rows();				
			}
			//network	
			m_network = std::get<0>(get_network_from_string(parameters.m_network, m_input_1d, m_output_1d));
			//init weights
			auto wit = parameters.m_network_weights.get().begin();
			for(size_t l=0; l < m_network.size(); ++l)
			for(size_t m=0; m < m_network[l].size(); ++m)
			{
				m_network[l][m] = *(wit++);
			}
			//ok
            m_success_init = true;
		}

		virtual ~NNTestInstance()
		{
			//none	
		}

		Random&  random_engine()  const override
		{
			return (*(Random*)nullptr);
		}

		const NeuralNetwork&  neural_network() const override
		{
			return m_network;
		}

		DataSetLoader& dataset_loader() const override
		{
			return (*(DataSetLoader*)nullptr);
		}

		Evaluation::SPtr loss_function() const override
		{
			return nullptr;
		}

		Evaluation::SPtr validation_function() const override
		{
			return  nullptr;
		}

		Evaluation::SPtr test_function() const override
		{
			return  nullptr;
		}

		std::ostream&  output_stream() const override
		{
			return m_runtime_output_stream;
		}

		SerializeOutput::SPtr serialize_output() const override
		{
			return nullptr;
		}

		ThreadPool*	thread_pool() const override
		{
			return nullptr;
		}

		bool execute() override
		{
			////////////////////////////////////////////////////////////////////////////////////////////////
			if (!m_success_init) return false;
			////////////////////////////////////////////////////////////////////////////////////////////////
			if(!m_network.size()) return false;
		    ////////////////////////////////////////////////////////////////////////////////////////////////
			Matrix inputs;
			inputs.resize(m_input_1d,1); //only FC
			//while
			bool exit_loop = false;
			while(!exit_loop)
			{
				output_stream() << "input[" << m_input_1d << "]:\t";
				//get inputs
				for (int i = 0; i < m_input_1d; ++i) std::cin >> inputs(i,0);
				//res
				Matrix result = m_network.predict(inputs);
				//ouput
				output_stream() << "output[" << m_output_1d << "]:\t";
				//get inputs
				for (int i = 0; i < m_output_1d; ++i)  output_stream() << result(i,0) << " " ;
				//print arg max
				Matrix::Index max_row = 0,max_col = 0;
				result.array().maxCoeff(&max_row,&max_col);
				output_stream() << std::endl << "argmax:\t\t" << max_row << std::endl;

			}
			//success
			return true;
		}
	};
	REGISTERED_INSTANCE(NNTestInstance, "nntest")
}

#include "Denn/Config.h"
#include "Denn/Instance.h"
#include "Denn/Evaluation.h"
#include "Denn/RuntimeOutput.h"
#include "Denn/SerializeOutput.h"
#include "Denn/DataSet/DataSet.h"
#include "Denn/DataSet/DataSetLoader.h"
#include "Denn/DataSet/TestSetStream.h"
#include "Denn/Utilities/Build.h"
#include <fstream>

namespace Denn
{
	class DBTestInstance : public Instance
	{
	public:
		//output
		mutable std::ofstream  m_runtime_output_file_stream;  //n.b. before of context dec
        mutable std::ostream          m_runtime_output_stream{ nullptr }; //< stream
		//context
        mutable NeuralNetwork         m_network;	   //< default network
        mutable Random				  m_random_engine; //< random engine
		mutable DataSetLoader::SPtr   m_dataset;	   //< dataset	
		//params			
		const Denn::Parameters&	      m_parameters;    //< parameters
        bool  m_success_init{ false };

		//init
		DBTestInstance(const Denn::Parameters& parameters) : m_parameters(parameters)
		{
        	////////////////////////////////////////////////////////////////////////////////////////////////
			//Dataset
			if (!(*parameters.m_dataset_filename).size()) return;
			//test file
			if (!Denn::Filesystem::exists((const std::string&)parameters.m_dataset_filename))
			{
				std::cerr << "input file: \"" << *parameters.m_dataset_filename << "\" does not exists!" << std::endl;
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
			//stream
			if (!build_outputstream(m_runtime_output_stream, m_runtime_output_file_stream, parameters)) return;
			//ok
            m_success_init = true;
		}

		virtual ~DBTestInstance()
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
			//Test
            TestSetStream dbstream(m_dataset.get());
            //print values
            dbstream.start_read_batch(*m_parameters.m_batch_size, m_parameters.m_batch_offset);
            //start id
            size_t n_batch = m_dataset->get_main_header_info().m_n_batch;
            size_t c_batch = 0;
			size_t samples_count = 0;
			//////////////////////////////////////////////
			//cout fiels
			m_dataset->start_read_batch();
			m_dataset->clear_batch_counter();
            while(m_dataset->number_of_batch_read() < n_batch)
			{
                //get
                DataSetScalar batch;
				m_dataset->read_batch(batch);
				samples_count += m_dataset->get_last_batch_info().m_n_row;
			}            
			output_stream() << "-------------------------" << std::endl;
            output_stream() << "- SAMPLES:" << samples_count << std::endl;
            output_stream() << "-------------------------" << std::endl;
			//////////////////////////////////////////////
			//..
            output_stream() << "-------------------------" << std::endl;
        	output_stream() << "READ BATCH BY TestStream:" << std::endl;
			size_t read_count = 0;
            //print
            while(read_count < samples_count)
            {
                //print batch id
                output_stream() << "-----------------" << std::endl;
                output_stream() << "BATCH N[" << c_batch << "]" << std::endl;
                //getcd 
                auto batch = dbstream.last_batch();
                //print
                for(size_t c = 0; c!=batch.features().cols(); ++c)
                {
                	for(size_t r = 0; r!=batch.features().rows(); ++r)
                    {
                        output_stream() << batch.features()(r,c) << " ";
                    }
                    output_stream() << "| ";
                	for(size_t r = 0; r!=batch.labels().rows(); ++r)
                    {
                        output_stream() << batch.labels()(r,c) << " ";
                    }
                    output_stream() << std::endl;
                }
				//add
				read_count += dbstream.batch_offset();
				c_batch += 1;
                //next
                dbstream.read_batch();
            }           
			//out
            output_stream() << "-------------------------" << std::endl;
        	output_stream() << "READ BATCH BY RawStream:" << std::endl;
			//Restart
			m_dataset->start_read_batch();
			m_dataset->clear_batch_counter();
			//read
            while(m_dataset->number_of_batch_read() < n_batch)
            {
                //get
                DataSetScalar batch;
				m_dataset->read_batch(batch,false);
                //print batch id
                output_stream() << "-----------------" << std::endl;
                output_stream() << "BATCH"
				<< "[id: "   << m_dataset->get_last_batch_info().m_batch_id 
				<< ", depth:" << m_dataset->get_last_batch_info().m_n_depth
				<< ", row:"   << m_dataset->get_last_batch_info().m_n_row
				<< "]" << std::endl;
                //print
                for(size_t c = 0; c!=batch.features().cols(); ++c)
                {
                	for(size_t r = 0; r!=batch.features().rows(); ++r)
                    {
                        output_stream() << batch.features()(r,c) << " ";
                    }
                    output_stream() << "| ";
                	for(size_t r = 0; r!=batch.labels().rows(); ++r)
                    {
                        output_stream() << batch.labels()(r,c) << " ";
                    }
                    output_stream() << std::endl;
                }
            }
			//TEST SET
            output_stream() << "-------------------------" << std::endl;
			{
				//read
                DataSetScalar batch;
				m_dataset->read_test(batch);
				//test print
                output_stream() << "TEST "
				<< "[features: "   << m_dataset->get_main_header_info().m_n_features 
				<< ", classes:" << m_dataset->get_main_header_info().m_n_classes
				<< ", size:" << batch.features().cols()
				<< "]" << std::endl;
				//print
				for(size_t c = 0; c!=batch.features().cols(); ++c)
				{
					for(size_t r = 0; r!=batch.features().rows(); ++r)
					{
						output_stream() << batch.features()(r,c) << " ";
					}
					output_stream() << "| ";
					for(size_t r = 0; r!=batch.labels().rows(); ++r)
					{
						output_stream() << batch.labels()(r,c) << " ";
					}
					output_stream() << std::endl;
				}
			}
			//success
			return true;
		}
	};
	REGISTERED_INSTANCE(DBTestInstance, "dbtest")
}

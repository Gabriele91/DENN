#pragma once
#include "Config.h"
#include "Parameters.h"
#include "NeuralNetwork.h"
#include "DatasetLoader.h"
#include "Evaluation.h"
#include "SerializeOutput.h"
#include "RuntimeOutput.h"

namespace Denn
{
	class Instance : public std::enable_shared_from_this< Instance >
	{
	public:
		using SPtr = std::shared_ptr<Instance>;

		Instance();
		Instance(const Denn::Parameters& parameters);
		virtual ~Instance();

		SPtr get_ptr() { return shared_from_this(); }

		virtual Random&				  random_engine()  const = 0;
		virtual const NeuralNetwork&  neural_network() const = 0;
		virtual DataSetLoader&        dataset_loader() const = 0;
		virtual Evaluation::SPtr      loss_function() const = 0;
		virtual Evaluation::SPtr      validation_function() const = 0;
		virtual Evaluation::SPtr      test_function() const = 0;
		virtual std::ostream&         output_stream() const = 0;
		virtual SerializeOutput::SPtr serialize_output() const = 0;
		virtual ThreadPool*			  thread_pool() const = 0;

		virtual bool execute() = 0;
	};

	//class factory of Instance
	class InstanceFactory
	{

	public:
		//create function
		typedef Instance::SPtr(*CreateObject)(const Denn::Parameters& parameters);

		//public
		static Instance::SPtr create(const std::string& name, const Denn::Parameters& parameters);
		static void append(const std::string& name, CreateObject fun, size_t size);

		//list of methods
		static std::vector< std::string > list_of_instances();
		static std::string names_of_instances(const std::string& sep = ", ");

		//info
		static bool exists(const std::string& name);

	};

	//class used for static registration of a object class
	template<class T>
	class InstanceItem
	{

		static Instance::SPtr create(const Denn::Parameters& parameters)
		{
			return (std::make_shared< T >(parameters))->get_ptr();
		}

		InstanceItem(const std::string& name, size_t size)
		{
			InstanceFactory::append(name, InstanceItem<T>::create, size);
		}

	public:


		static InstanceItem<T>& instance(const std::string& name, size_t size)
		{
			static InstanceItem<T> objectItem(name, size);
			return objectItem;
		}

	};


#define REGISTERED_INSTANCE(class_,name_)\
	namespace\
	{\
		static const auto& _Denn_ ## class_ ## _InstanceItem= Denn::InstanceItem<class_>::instance( name_, sizeof(class_) );\
	}

}

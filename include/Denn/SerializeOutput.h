#pragma once 
#include "Config.h"
#include <ostream>

namespace Denn 
{
	//dec classes
	class Parameters;
	class NeuralNetwork;
	class Individual;

	//SerializeOutput
	class SerializeOutput : public std::enable_shared_from_this< SerializeOutput >
	{
	public:

		using SPtr = std::shared_ptr<SerializeOutput>;

		SPtr get_ptr() { return shared_from_this(); }

		virtual std::ostream&     output()     const { return m_stream; }

		virtual const Parameters& parameters() const { return m_params; }

		SerializeOutput(std::ostream& ostream, const Parameters& params) :m_stream(ostream), m_params(params) {}

		virtual ~SerializeOutput() {}

		virtual void serialize_parameters(const Denn::Parameters& args) {}

		virtual void serialize_best(double time, Denn::Scalar accuracy, Denn::Scalar f, Denn::Scalar cr, const Denn::NeuralNetwork& network)
		{
		}

	protected:

		std::ostream& m_stream;
		const Parameters& m_params;

	};

	//class factory of SerializeOutput
	class SerializeOutputFactory
	{

	public:
		//Crossover classes map
		typedef SerializeOutput::SPtr(*CreateObject)(std::ostream& stream, const Parameters& params);

		//public
		static SerializeOutput::SPtr create(const std::string& name, std::ostream& stream, const Parameters& params);
		static void append(const std::string& name, CreateObject fun, size_t size);

		//list of methods
		static std::vector< std::string > list_of_serialize_outputs();
		static std::string names_of_serialize_outputs(const std::string& sep = ", ");

		//info
		static bool exists(const std::string& name);

	};

	//class used for static registration of a object class
	template<class T>
	class SerializeOutputItem
	{

		static SerializeOutput::SPtr create(std::ostream& stream, const Parameters& params)
		{
			return (std::make_shared< T >(stream, params))->get_ptr();
		}

		SerializeOutputItem(const std::string& name, size_t size)
		{
			SerializeOutputFactory::append(name, SerializeOutputItem<T>::create, size);
		}

	public:


		static SerializeOutputItem<T>& instance(const std::string& name, size_t size)
		{
			static SerializeOutputItem<T> objectItem(name, size);
			return objectItem;
		}

	};


	#define REGISTERED_SERIALIZE_OUTPUT(class_,name_)\
	namespace\
	{\
		static const SerializeOutputItem<class_>& _Denn_ ## class_ ## _SerializeOutputItem= SerializeOutputItem<class_>::instance( name_, sizeof(class_) );\
	}

}
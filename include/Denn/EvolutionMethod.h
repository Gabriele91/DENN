#pragma once
#include "Config.h"

namespace Denn
{
	//dec class
	class DennAlgorithm;
	class EvolutionMethod;
	class Parameters;
	class Random;
	class DoubleBufferPopulation;
	class Individual;

	class EvolutionMethod : public std::enable_shared_from_this< EvolutionMethod >
	{
	public:
		//ref to EvolutionMethod
		using SPtr = std::shared_ptr<EvolutionMethod>;
		//return ptr
		SPtr get_ptr() { return this->shared_from_this(); }
		//EvolutionMethod
		EvolutionMethod(const DennAlgorithm& algorithm);
		//..		
		virtual void start();
		virtual void start_a_gen_pass(DoubleBufferPopulation& population);
		virtual void start_a_subgen_pass(DoubleBufferPopulation& population);
		virtual void create_a_individual(DoubleBufferPopulation& population, size_t target, Individual& i_output) = 0;
		virtual	void selection(DoubleBufferPopulation& population) = 0;
		virtual void end_a_subgen_pass(DoubleBufferPopulation& population);
		virtual void end_a_gen_pass(DoubleBufferPopulation& population);
        virtual bool best_from_validation();
        virtual const VariantRef get_context_data() const;

	protected:
		//attributes
		const DennAlgorithm& m_algorithm;

		//easy access
		const Parameters& parameters() const;
		const EvolutionMethod& evolution_method() const;
	
		const size_t current_np() const;
		const DoubleBufferPopulation& population() const;

		Random& random()  const;
		Random& random(size_t i)  const;

		//help, how is the best
		bool loss_function_compare(Scalar left, Scalar right) const;
		bool validation_function_compare(Scalar left, Scalar right) const;
		bool test_function_compare(Scalar left, Scalar right) const;
	};

	//class factory of Evolution methods
	class EvolutionMethodFactory
	{

	public:
		//EvolutionMethod classes map
		typedef EvolutionMethod::SPtr(*CreateObject)(const DennAlgorithm& algorithm);

		//public
		static EvolutionMethod::SPtr create(const std::string& name, const DennAlgorithm& algorithm);
		static void append(const std::string& name, CreateObject fun, size_t size);

		//list of methods
		static std::vector< std::string > list_of_evolution_methods();
		static std::string names_of_evolution_methods(const std::string& sep = ", ");

		//info
		static bool exists(const std::string& name);

	};

	//class used for static registration of a object class
	template<class T>
	class EvolutionMethodItem
	{

		static EvolutionMethod::SPtr create(const DennAlgorithm& algorithm)
		{
			return (std::make_shared< T >(algorithm))->get_ptr();
		}

		EvolutionMethodItem(const std::string& name, size_t size)
		{
			EvolutionMethodFactory::append(name, EvolutionMethodItem<T>::create, size);
		}

	public:


		static EvolutionMethodItem<T>& instance(const std::string& name, size_t size)
		{
			static EvolutionMethodItem<T> objectItem(name, size);
			return objectItem;
		}

	};

	#define REGISTERED_EVOLUTION_METHOD(class_,name_)\
	namespace\
	{\
		static const EvolutionMethodItem<class_>& _Denn_ ## class_ ## _EvolutionMethodItem= EvolutionMethodItem<class_>::instance( name_, sizeof(class_) );\
	}
}

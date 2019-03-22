#pragma once
#include "Denn/Config.h"
#include "Population.h"

namespace Denn
{	
	//dec class
	class Solver;
	class EvolutionMethod;
	class Parameters;
	class Random;

	//crossover
	class Crossover : public std::enable_shared_from_this< Crossover >
	{
		public:
		//Crossover info
		Crossover(const EvolutionMethod& emethod);
		//ref to Crossover
		using SPtr = std::shared_ptr<Crossover>;
		//operator
		virtual void operator()(size_t id_target, Individual& output)= 0;
		//return ptr
		SPtr get_ptr() { return this->shared_from_this(); }

	protected:
		//easy access
		const EvolutionMethod& evolution_method() const;
		const Solver& solver() const;	
		const Parameters& parameters() const;

		const SubPopulation& population() const;
		const size_t current_np() const;
		const Individual& parent(size_t) const;
        
		//random engine
		Random& random(size_t i)  const;
		
		//help, how is the best
		bool loss_function_compare(Scalar left, Scalar right) const;
		bool validation_function_compare(Scalar left, Scalar right) const;
		bool test_function_compare(Scalar left, Scalar right) const;
	
	private:

		const EvolutionMethod& m_emethod;

	};

	//class factory of Crossover methods
	class CrossoverFactory
	{

	public:
		//Crossover classes map
		typedef Crossover::SPtr(*CreateObject)(const EvolutionMethod& emethod);

		//public
		static Crossover::SPtr create(const std::string& name, const EvolutionMethod& method);
		static void append(const std::string& name, CreateObject fun, size_t size);

		//list of methods
		static std::vector< std::string > list_of_crossovers();
		static std::string names_of_crossovers(const std::string& sep = ", ");

		//info
		static bool exists(const std::string& name);

	};

	//class used for static registration of a object class
	template<class T>
	class CrossoverItem
	{

		static Crossover::SPtr create(const EvolutionMethod& emethod)
		{
			return (std::make_shared< T >(emethod))->get_ptr();
		}

		CrossoverItem(const std::string& name, size_t size)
		{
			CrossoverFactory::append(name, CrossoverItem<T>::create, size);
		}

	public:


		static CrossoverItem<T>& instance(const std::string& name, size_t size)
		{
			static CrossoverItem<T> objectItem(name, size);
			return objectItem;
		}

	};


	#define REGISTERED_CROSSOVER(class_,name_)\
	namespace\
	{\
		static const CrossoverItem<class_>& _Denn_ ## class_ ## _CrossoverItem= CrossoverItem<class_>::instance( name_, sizeof(class_) );\
	}

}
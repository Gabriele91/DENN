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

	//mutation
    class Mutation : public std::enable_shared_from_this< Mutation >
	{ 
		public:
		//ref to Mutation
		using SPtr = std::shared_ptr<Mutation>;
		//return ptr
		SPtr get_ptr() { return this->shared_from_this(); }
		//Crossover info
		Mutation(const EvolutionMethod& method);
		//operation
		virtual void operator() (size_t id_target,Individual& slider) = 0; 
		//required sort (default false)
		virtual bool required_sort() const { return false; }

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

	//class factory of Mutation methods
	class MutationFactory
	{

	public:
		//Mutation classes map
		typedef Mutation::SPtr(*CreateObject)(const EvolutionMethod& emethod);

		//public
		static Mutation::SPtr create(const std::string& name, const EvolutionMethod& emethod);
		static void append(const std::string& name, CreateObject fun, size_t size);

		//list of methods
		static std::vector< std::string > list_of_mutations();
		static std::string names_of_mutations(const std::string& sep = ", ");
		
		//info
		static bool exists(const std::string& name);

	};

	//class used for static registration of a object class
	template<class T>
	class MutationItem
	{

		static Mutation::SPtr create(const EvolutionMethod& emethod)
		{
			return (std::make_shared< T >(emethod))->get_ptr();
		}

		MutationItem(const std::string& name, size_t size)
		{
			MutationFactory::append(name, MutationItem<T>::create, size);
		}

	public:


		static MutationItem<T>& instance(const std::string& name, size_t size)
		{
			static MutationItem<T> objectItem(name, size);
			return objectItem;
		}

	};


	#define REGISTERED_MUTATION(class_,name_)\
	namespace\
	{\
		static const MutationItem<class_>& _Denn_ ## class_ ## _MutationItem= MutationItem<class_>::instance( name_, sizeof(class_) );\
	}
}
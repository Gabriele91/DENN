#pragma once
#include "Denn/Config.h"
#include "Individual.h"
#include "SubPopulation.h"

namespace Denn
{
	//dec class
	class Solver;
	class Parameters;
	class Random;
	class Population;
	class Mutation;
	class Crossover;

	class EvolutionMethod : public std::enable_shared_from_this< EvolutionMethod >
	{
	public:
		//ref to EvolutionMethod
		using SPtr = std::shared_ptr<EvolutionMethod>;
		//return ptr
		SPtr get_ptr() { return this->shared_from_this(); }
		//EvolutionMethod
		EvolutionMethod(const Solver& solver, SubPopulation& population);
		//..		
		virtual void start();
		virtual void start_a_gen_pass();
		virtual void start_a_subgen_pass();
		virtual void create_a_individual(size_t target, Individual& i_output) = 0;
		virtual	void selection() = 0;
		virtual void end_a_subgen_pass();
		virtual void end_a_gen_pass();
        virtual const VariantRef get_context_data() const;

	protected:

		//easy access		
		const Solver& solver() const;
		const Parameters& parameters() const;

		const size_t current_np() const;
		SubPopulation& population();
		SubPopulation::Pair population(size_t);
		const SubPopulation& population() const;
		const SubPopulation::Pair population(size_t) const;
		void population_sort();
		virtual	void performe_selection();
		virtual	void performe_selection(std::vector<int>& idxs);

		const size_t best_parent_id() const;
		const Individual& parent(size_t) const;
		const Individual& son(size_t) const;
		

		Random& random()  const;
		Random& random(size_t i)  const;

		//help, how is the best
		bool loss_function_compare(Scalar left, Scalar right) const;
		bool validation_function_compare(Scalar left, Scalar right) const;
		bool test_function_compare(Scalar left, Scalar right) const;

	private:	
		//attributes
		const Solver& m_solver;
		//population
		SubPopulation& m_sub_population;
		//friends
		friend class Mutation;
		friend class Crossover;
	};
	//alias of Evolution Methods vector
    using EvolutionMethodList = std::vector< EvolutionMethod::SPtr >;
	//class factory of Evolution methods
	class EvolutionMethodFactory
	{

	public:
		//EvolutionMethod classes map
		typedef EvolutionMethod::SPtr(*CreateObject)(const Solver& solver, SubPopulation& population);

		//public
		static EvolutionMethod::SPtr create(const std::string& name, const Solver& solver, SubPopulation& population);
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

		static EvolutionMethod::SPtr create(const Solver& solver, SubPopulation& population)
		{
			return (std::make_shared< T >(solver, population))->get_ptr();
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

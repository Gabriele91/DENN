#include "Denn/Parameters.h"
#include "Denn/Denn/Individual.h"
#include "Denn/Denn/Population.h"
#include "Denn/Denn/Solver.h"
#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Crossover.h"
namespace Denn
{
	class DEMethod : public EvolutionMethod
	{
	public:

		DEMethod(const Solver& solver, SubPopulation& sub_pop) : EvolutionMethod(solver, sub_pop) {}

		virtual void start() override
		{
			//create mutation/crossover
			m_mutation = MutationFactory::create(parameters().m_mutation_type, *this);
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, *this);
		}
		
		virtual void start_a_subgen_pass() override
		{
			//sort parents
			if (m_mutation->required_sort())  population_sort();
		}

		virtual void create_a_individual
		(
			  size_t id_target
			, Individual& output
		)
		override
		{
			//vectors
			Individual& target = *population()[id_target].parent();
			//copy
			output.f()  = target.f();
			output.cr() = target.cr();
			output.p()  = target.p();
			//call muation + crossover
			(*m_mutation) (id_target, output);
			(*m_crossover)(id_target, output);
			//no 0
			output.no_0_weights();
			//eval
			output.eval() = solver().loss_function_eval(output, id_target);
		}

		virtual	void selection() override
		{
			if(*parameters().m_crowding_selection)
				population().swap_crowding();
			else 
				population().swap_best();
		}

	private:

		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(DEMethod,"DE")

}
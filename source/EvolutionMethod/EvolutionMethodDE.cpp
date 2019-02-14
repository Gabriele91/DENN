#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	class DEMethod : public EvolutionMethod
	{
	public:

		DEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm) {}

		virtual void start() override
		{
			//create mutation/crossover
			m_mutation = MutationFactory::create(m_algorithm.parameters().m_mutation_type, m_algorithm);
			m_crossover = CrossoverFactory::create(m_algorithm.parameters().m_crossover_type, m_algorithm);
		}
		
		virtual void start_a_subgen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//sort parents
			if (m_mutation->required_sort()) dpopulation.parents().sort();
		}

		virtual void create_a_individual
		(
			  DoubleBufferPopulation& population
			, size_t i_target
			, Individual& i_output
		)
		override
		{
			const Population& parents= population.parents();
			const Individual& target = *parents[i_target];
			//copy
			i_output.m_f  = target.m_f;
			i_output.m_cr = target.m_cr;
			//call muation
			(*m_mutation) (parents, i_target, i_output);
			//call crossover
			(*m_crossover)(parents, i_target, i_output);
		}

		virtual	void selection(DoubleBufferPopulation& population) override
		{
			population.the_best_sons_become_parents();
		}

	private:

		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(DEMethod,"DE")

}
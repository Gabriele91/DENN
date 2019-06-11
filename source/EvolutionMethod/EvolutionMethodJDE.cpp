#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	class JDEMethod : public EvolutionMethod
	{
	public:

		JDEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm) {}
	
		virtual void start() override
		{
			//create mutation/crossover
			m_mutation = MutationFactory::create(parameters().m_mutation_type, m_algorithm);
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, m_algorithm);
		}
		
		virtual void start_a_subgen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//sort parents
			if (m_mutation->required_sort()) dpopulation.parents().sort();
		}

		virtual void create_a_individual
		(     
			  DoubleBufferPopulation& dpopulation
			, size_t i_target
			, Individual& i_output
		) 
		override
		{
			//vectors
			const Population& parents    = dpopulation.parents();
			const Individual& target     = *parents[i_target];
			//f JDE
			if (random(i_target).uniform() < Scalar(parameters().m_jde_f))
				i_output.m_f = Scalar(random(i_target).uniform(0.0, 2.0));
			else
				i_output.m_f = target.m_f;
			//cr JDE
			if (random(i_target).uniform() < Scalar(parameters().m_jde_cr))
				i_output.m_cr = Scalar(random(i_target).uniform());
			else
				i_output.m_cr = target.m_cr;
			//call muation
			(*m_mutation) (parents, i_target, i_output);
			//call crossover
			(*m_crossover)(parents, i_target, i_output);
			//no 0 wights
			i_output.m_network.no_0_weights();
		}

		virtual void selection(DoubleBufferPopulation &population) override
		{
			if (*parameters().m_crowding_selection)
				population.swap_crowding();
			else
				population.the_best_sons_become_parents();
		}

	protected:

		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(JDEMethod, "JDE")
}
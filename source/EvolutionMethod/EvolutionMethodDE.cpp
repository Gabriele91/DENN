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
			  DoubleBufferPopulation& dpopulation
			, size_t id_target
			, Individual& output
		)
		override
		{
			//vectors
			Population& population = dpopulation.parents();
			Individual& target     = *population[id_target];
			//copy
			output.m_f  = target.m_f;
			output.m_cr = target.m_cr;
			output.m_p  = target.m_p;
			//call muation + crossover
			for(size_t l = 0; l < output.size(); ++l)
			for(size_t m = 0; m < output[l].size(); ++m)
			{
				PopulationSlider pop_slider(population, l, m);
				IndividualSlider ind_slider(output, l, m);
				(*m_mutation) (pop_slider, id_target, ind_slider);
				(*m_crossover)(pop_slider, id_target, ind_slider);
			}
			//no 0 wights
			output.m_network.no_0_weights();
			//eval
			output.m_eval = (*m_algorithm.loss_function())((NeuralNetwork&)output, m_algorithm.current_batch());
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
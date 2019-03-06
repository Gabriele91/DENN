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
			, size_t id_target
			, Individual& output
		) 
		override
		{
			//vectors
			Population& population = dpopulation.parents();
			Individual& target     = *population[id_target];
			//f JDE
			if (random(id_target).uniform() < Scalar(parameters().m_jde_f))
				output.m_f = Scalar(random(id_target).uniform(0.0, 2.0));
			else
				output.m_f = target.m_f;
			//cr JDE
			if (random(id_target).uniform() < Scalar(parameters().m_jde_cr))
				output.m_cr = Scalar(random(id_target).uniform());
			else
				output.m_cr = target.m_cr;
			//update p
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

	protected:

		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(JDEMethod, "JDE")
}
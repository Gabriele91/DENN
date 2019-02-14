#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	class PHistoryMethod : public EvolutionMethod
	{
	public:
		//PHistoryMethod
		PHistoryMethod(const DennAlgorithm& algorithm): EvolutionMethod(algorithm)
		{
			//get size
			m_max_size   = parameters().m_history_size;
			//sub method
			m_sub_method = EvolutionMethodFactory::create(parameters().m_sub_evolution_type, m_algorithm);
		}
		//..		
		virtual void start() override
		{ 
			//init 
			m_k = 0;
			m_history.clear();
			//start
			m_sub_method->start(); 
		};
		virtual void start_a_gen_pass(DoubleBufferPopulation& g_population) override
		{ 
			///////////////////////////////////////////////////////////////////////////////
			// compere history with new population
			for(Population& population_tmp : m_history)
			{
				m_algorithm.execute_fitness_on(population_tmp);
				for(size_t i=0; i!=current_np(); ++i)
				{
					if( population_tmp[i]->m_eval < g_population.parents()[i]->m_eval )
					{
						g_population.parents()[i] = population_tmp[i]->copy();
					}
				}
			}
			///////////////////////////////////////////////////////////////////////////////
			//copy
			if(m_history.size() < m_max_size)
			{
				m_history.push_back(g_population.parents().copy());
			}  
			else
			{
				m_history[m_k] = g_population.parents().copy(); 
				m_k = (m_k + 1) % m_max_size;
			} 
			///////////////////////////////////////////////////////////////////////////////
			//start the pass
			m_sub_method->start_a_gen_pass(g_population);
		};
		virtual void start_a_subgen_pass(DoubleBufferPopulation& population) override { m_sub_method->start_a_subgen_pass(population); };
		virtual void create_a_individual(DoubleBufferPopulation& population, size_t target, Individual& i_output) override { m_sub_method->create_a_individual(population, target, i_output); };
		virtual	void selection(DoubleBufferPopulation& population) override           { m_sub_method->selection(population); };
		virtual void end_a_subgen_pass(DoubleBufferPopulation& population) override   { m_sub_method->end_a_subgen_pass(population); };
		virtual void end_a_gen_pass(DoubleBufferPopulation& population) override      { m_sub_method->end_a_gen_pass(population); };
		virtual const VariantRef get_context_data() const override                    { return m_sub_method->get_context_data(); }

	protected:
	
		size_t					  m_k;
		size_t					  m_max_size;
		std::vector< Population > m_history;
		EvolutionMethod::SPtr     m_sub_method;

	};
	REGISTERED_EVOLUTION_METHOD(PHistoryMethod,"PHISTORY")


	class P2HistoryMethod : public PHistoryMethod
	{
	public:

		//PHistoryMethod
		P2HistoryMethod(const DennAlgorithm& algorithm): PHistoryMethod(algorithm) {}

		virtual void start_a_gen_pass(DoubleBufferPopulation& g_population) override
		{ 
			///////////////////////////////////////////////////////////////////////////////
			// compere history with new population
			for(Population& population_tmp : m_history)
			{
				m_algorithm.execute_fitness_on(population_tmp);
			}				
			//best of the best
			for(const Population& population_tmp : m_history)
			{
				for(const Individual::SPtr& individual_tmp : population_tmp)
				for(size_t u=0; u!=current_np(); ++u)				
				{
					if( individual_tmp->m_eval < g_population.parents()[u]->m_eval )
					{
						g_population.parents()[u] = individual_tmp->copy();
						break;
					}
				}
			}
			///////////////////////////////////////////////////////////////////////////////
			//copy
			if(m_history.size() < m_max_size)
			{
				m_history.push_back(g_population.parents().copy());
			}  
			else
			{
				m_history[m_k] = g_population.parents().copy(); 
				m_k = (m_k + 1) % m_max_size;
			} 
			///////////////////////////////////////////////////////////////////////////////
			//start the pass
			m_sub_method->start_a_gen_pass(g_population);
		};

	};
	REGISTERED_EVOLUTION_METHOD(P2HistoryMethod,"P2HISTORY")
}
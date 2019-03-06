#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	class JADEMethod : public EvolutionMethod
	{
	public:

		JADEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm) 
		{
			m_archive_max_size = parameters().m_archive_size;
			m_c_adapt          = parameters().m_f_cr_adapt;
			m_mu_f       = Scalar(0.5);
			m_mu_cr      = Scalar(0.5);
		}
		
		virtual void start() override
		{
			//reinit
			m_mu_f = Scalar(0.5);
			m_mu_cr = Scalar(0.5);
			//clear
			m_archive.clear();
			//create mutation/crossover
			m_mutation = MutationFactory::create(parameters().m_mutation_type, m_algorithm);
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, m_algorithm);
		}

		virtual void start_a_gen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//Update F/CR??
		}

		virtual void start_a_subgen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//sort parents
			if(m_mutation->required_sort()) dpopulation.parents().sort();
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
			//Compute F
			//JADE REF:  Cauchy  distribution  with  location  parameter μF and scale parameter 0.1
			//           Fi=randci(μF,0.1) and  then  truncated  to  be  1  if Fi≥1  or  regenerated  if Fi ≤ 0
			Scalar v;
			do v = random(id_target).cauchy(m_mu_f, 0.1); while (v <= 0);
			//update meta parameters
			output.m_f  = Denn::sature(v);
			output.m_cr = Denn::sature(random(id_target).normal(m_mu_cr, 0.1));
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

		virtual	void selection(DoubleBufferPopulation& dpopulation) override
		{
			Scalar sum_f = 0;
			Scalar sum_f2 = 0;
			Scalar sum_cr = 0;
			size_t n_discarded = 0;
			size_t np = current_np();

			for (size_t i = 0; i != np; ++i)
			{
				Individual::SPtr father = dpopulation.parents()[i];
				Individual::SPtr son = dpopulation.sons()[i];
				if (loss_function_compare(son->m_eval,father->m_eval))
				{
					if (m_archive_max_size) m_archive.push_back(father->copy());
					//else if (random().uniform() < Scalar(m_archive_max_size) / Scalar(m_archive_max_size + n_discarded))
					sum_f += son->m_f;
					sum_f2 += son->m_f * son->m_f;
					sum_cr += son->m_cr;
					++n_discarded;
					dpopulation.swap(i);
				}
			}
			//safe compute muF and muCR 
			if (n_discarded)
			{
				m_mu_cr = Denn::lerp(m_mu_cr, sum_cr / n_discarded, m_c_adapt);
				m_mu_f = Denn::lerp(m_mu_f, sum_f2 / sum_f, m_c_adapt);
			}
			//reduce A
			while (m_archive_max_size < m_archive.size())
			{
				m_archive[random().index_rand(m_archive.size())] = m_archive.last();
				m_archive.pop_back();
			}
		}

		virtual const VariantRef get_context_data() const override
		{
			return VariantRef(m_archive);
		}

	protected:		

		size_t          m_archive_max_size{ false };
		Scalar          m_c_adapt         { Scalar(1.0) };
		Scalar          m_mu_f      { Scalar(0.5) };
		Scalar          m_mu_cr     { Scalar(0.5) };
		Population	    m_archive;
		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(JADEMethod, "JADE")
}
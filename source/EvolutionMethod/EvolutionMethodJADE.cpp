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
			, size_t i_target
			, Individual& i_output
		)
		override
		{
			//Compute F
			//JADE REF:  Cauchy  distribution  with  location  parameter μF and scale parameter 0.1
			//           Fi=randci(μF,0.1) and  then  truncated  to  be  1  if Fi≥1  or  regenerated  if Fi ≤ 0
			Scalar v;
			do v = random(i_target).cauchy(m_mu_f, 0.1); while (v <= 0);
			i_output.m_f = Denn::sature(v);
			//Cr
			i_output.m_cr = Denn::sature(random(i_target).normal(m_mu_cr, 0.1));
			//call muation
			(*m_mutation) (dpopulation.parents(), i_target, i_output);
			//call crossover
			(*m_crossover)(dpopulation.parents(), i_target, i_output);
			//no 0 wights
			i_output.m_network.no_0_weights();
		}

		virtual	void selection(DoubleBufferPopulation& dpopulation) override
		{
			/////////////////////////////////////////////////////////////
			//get swap list
			if(*parameters().m_crowding_selection)
				dpopulation.crowding_swap_list(m_swap_list);
			else
				dpopulation.parent_swap_list(m_swap_list);
			/////////////////////////////////////////////////////////////
			Scalar sum_f = 0;
			Scalar sum_f2 = 0;
			Scalar sum_cr = 0;
			size_t n_discarded = 0;
			size_t np = current_np();

			for (size_t i = 0; i != np; ++i)
			{
				if(m_swap_list[i] < 0) continue;
				//refs
				Individual::SPtr father = dpopulation.parents()[i];
				Individual::SPtr son = dpopulation.sons()[m_swap_list[i]];
				//compare?
				if (m_archive_max_size) m_archive.push_back(father->copy());
				//else if (main_random().uniform() < Scalar(m_archive_max_size) / Scalar(m_archive_max_size + n_discarded))
				sum_f += son->m_f;
				sum_f2 += son->m_f * son->m_f;
				sum_cr += son->m_cr;
				++n_discarded;
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
				m_archive[main_random().index_rand(m_archive.size())] = m_archive.last();
				m_archive.pop_back();
			}
			/////////////////////////////////////////////////////////////
			//swap
			dpopulation.swap(m_swap_list);
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
		std::vector<int> m_swap_list;
	};
	REGISTERED_EVOLUTION_METHOD(JADEMethod, "JADE")
}
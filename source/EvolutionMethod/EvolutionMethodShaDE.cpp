#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	class SHADEMethod : public EvolutionMethod
	{
	public:

		SHADEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm)
		{
			m_archive_max_size = parameters().m_archive_size;
			m_h = parameters().m_shade_h;
		}

		virtual void start() override
		{
			//H size
			m_mu_f = std::vector<Scalar>(m_h, Scalar(0.5));
			m_mu_cr = std::vector<Scalar>(m_h, Scalar(0.5));
			m_k = 0;
			m_pmin = Scalar(2) / Scalar(current_np());
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
			//take tou
			size_t tou_i = random(i_target).index_rand(m_mu_f.size());
			//Compute F
			Scalar v;
			do v = random(i_target).cauchy(m_mu_f[tou_i], 0.1); while (v <= 0);
			i_output.m_f = Denn::sature(v);
			//Cr
			i_output.m_cr = Denn::sature(random(i_target).normal(m_mu_cr[tou_i], 0.1));
			//P
			i_output.m_p = random(i_target).uniform(m_pmin, 0.2);
			//call muation
			(*m_mutation) (dpopulation.parents(), i_target, i_output);
			//call crossover
			(*m_crossover)(dpopulation.parents(), i_target, i_output);
		}

		virtual	void selection(DoubleBufferPopulation& dpopulation) override
		{
			//F
			Scalar sum_f = 0;
			Scalar sum_f2 = 0;
			//CR
			std::vector<Scalar> s_cr;
			std::vector<Scalar> s_delta_f;
			Scalar delta_f = 0;
			Scalar sum_delta_f = 0;
			size_t n_discarded = 0;
			//pop
			size_t np = current_np();

			for (size_t i = 0; i != np; ++i)
			{
				Individual::SPtr father = dpopulation.parents()[i];
				Individual::SPtr son = dpopulation.sons()[i];
				if (loss_function_compare(son->m_eval,father->m_eval))
				{
					if (m_archive_max_size) m_archive.push_back(father->copy());
					//else if (main_random().uniform() < Scalar(m_archive_max_size) / Scalar(m_archive_max_size + n_discarded))
					//F
					sum_f += son->m_f;
					sum_f2 += son->m_f * son->m_f;
					//w_k (for mean of Scr)
					delta_f = std::abs(son->m_eval - father->m_eval);
					s_delta_f.push_back(delta_f);
					sum_delta_f += delta_f;
					//Scr
					s_cr.push_back(son->m_cr);
					++n_discarded;
					//SWAP
					dpopulation.swap(i);
				}
			}
			//safe compute muF and muCR 
			if (n_discarded)
			{
				Scalar mean_w_scr = 0;
				for (size_t k = 0; k != n_discarded; ++k)
				{
					//mean_ca(Scr) = sum_0-k( Scr_k   * w_k )
					mean_w_scr += s_cr[k] * (s_delta_f[k] / sum_delta_f);
				}
				m_mu_cr[m_k] = mean_w_scr;
				m_mu_f[m_k] = sum_f2 / sum_f;
				m_k = (m_k + 1) % m_mu_f.size();
			}
			//reduce A
			while (m_archive_max_size < m_archive.size())
			{
				m_archive[main_random().index_rand(m_archive.size())] = m_archive.last();
				m_archive.pop_back();
			}
		}
		
		virtual const VariantRef get_context_data() const override
		{
			return VariantRef(m_archive);
		}

	protected:

		size_t				m_h{ 0 };
		size_t				m_k{ 0 };
		size_t              m_archive_max_size{ false };
		Scalar				m_pmin   { Scalar(0.0) };
		std::vector<Scalar> m_mu_f;
		std::vector<Scalar> m_mu_cr;
		Population	        m_archive;
		Mutation::SPtr      m_mutation;
		Crossover::SPtr     m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(SHADEMethod, "SHADE")
	
	class L_SHADEMethod : public EvolutionMethod
	{
	public:

		L_SHADEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm)
		{
			m_archive_max_size = parameters().m_archive_size;
			m_h = parameters().m_shade_h;
		}

		virtual void start() override
		{
			//H size
			m_mu_f = std::vector<Scalar>(m_h, Scalar(0.5));
			m_mu_cr = std::vector<Scalar>(m_h, Scalar(0.5));
			m_k = 0;
			m_pmin = Scalar(2) / Scalar(current_np());
			//clear
			m_archive.clear();
			//NFE to 0
			m_curr_nfe = 0;
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
			if (m_mutation->required_sort()) dpopulation.parents().sort();
		}

		virtual void end_a_subgen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//compute NFE
			m_curr_nfe += current_np();
		}

		virtual void create_a_individual
		(
			  DoubleBufferPopulation& dpopulation
			, size_t i_target
			, Individual& i_output
		)
		override
		{
			//take tou
			size_t tou_i = random(i_target).index_rand(m_mu_f.size());
			//Compute F
			Scalar v;
			do v = random(i_target).cauchy(m_mu_f[tou_i], 0.1); while (v <= 0);
			i_output.m_f = Denn::sature(v);
			//Cr
			i_output.m_cr = 
			  m_mu_cr[tou_i] == *parameters().m_mu_cr_terminal_value
			? Scalar(0.0) 
			: Denn::sature(random(i_target).normal(m_mu_cr[tou_i], 0.1));
			//P
			i_output.m_p = random(i_target).uniform(m_pmin, 0.2);
			//call muation
			(*m_mutation) (dpopulation.parents(), i_target, i_output);
			//call crossover
			(*m_crossover)(dpopulation.parents(), i_target, i_output);
		}

		virtual	void selection(DoubleBufferPopulation& dpopulation) override
		{
			//F
			Scalar sum_f = 0;
			Scalar sum_f2 = 0;
			//CR
			std::vector<Scalar> s_cr;
			std::vector<Scalar> s_delta_f;
			Scalar delta_f = 0;
			Scalar sum_delta_f = 0;
			size_t n_discarded = 0;
			//pop
			size_t np = current_np();
			for (size_t i = 0; i != np; ++i)
			{
				Individual::SPtr father = dpopulation.parents()[i];
				Individual::SPtr son = dpopulation.sons()[i];
				if (son->m_eval < father->m_eval)
				{
					if (m_archive_max_size) m_archive.push_back(father->copy());
					//F
					sum_f += son->m_f;
					sum_f2 += son->m_f * son->m_f;
					//w_k (for mean of Scr)
					delta_f = std::abs(son->m_eval - father->m_eval);
					s_delta_f.push_back(delta_f);
					sum_delta_f += delta_f;
					//Scr
					s_cr.push_back(son->m_cr);
					++n_discarded;
					//SWAP
					dpopulation.swap(i);
				}
			}
			//safe compute muF and muCR 
			if (n_discarded)
			{
				//compute mu
				Scalar mu_cr = *parameters().m_mu_cr_terminal_value;
				//isn't terminale?
				if ( m_mu_cr[m_k] != (*parameters().m_mu_cr_terminal_value)  &&
					(*std::max_element(s_cr.begin(), s_cr.end())) != Scalar(0.0))
				{
					//compute mu cr from formula
					Scalar w_k = 0;
					Scalar mean_w_scr = 0;
					Scalar mean_w_scr_pow2 = 0;
					//compute Mcr
					for (size_t k = 0; k != n_discarded; ++k)
					{
						//mean_ca(Scr) = sum_0-k( Scr_k   * w_k )
						w_k = (s_delta_f[k] / sum_delta_f);
						mean_w_scr_pow2 += s_cr[k] * s_cr[k] * w_k;
						mean_w_scr += s_cr[k] * w_k;
					}
					mu_cr = mean_w_scr_pow2 / mean_w_scr;
				}
				//
				m_mu_cr[m_k] = mu_cr;
				m_mu_f[m_k] = sum_f2 / sum_f;
				m_k = (m_k + 1) % m_mu_f.size();
			}
			//reduce A
			reduce_archive();
			//reduce population
			reduce_population(dpopulation);
		}

		virtual const VariantRef get_context_data() const override
		{
			return VariantRef(m_archive);
		}

	protected:

		size_t				m_h{ 0 };
		size_t				m_k{ 0 };
		/////////////////////////////////////////////////////
		size_t              m_archive_max_size{ false };
		Scalar				m_pmin{ Scalar(0.0) };
		//////////////////////////////////////////////////////
		size_t              m_curr_nfe{ size_t(0) };
		//////////////////////////////////////////////////////
		std::vector<Scalar> m_mu_f;
		std::vector<Scalar> m_mu_cr;
		Population	        m_archive;
		Mutation::SPtr      m_mutation;
		Crossover::SPtr     m_crossover;

		//reduce archive
		void reduce_archive()
		{
			while (m_archive_max_size < m_archive.size())
			{
				m_archive[main_random().index_rand(m_archive.size())] = m_archive.last();
				m_archive.pop_back();
			}
		}
		//reduce population
		void reduce_population(DoubleBufferPopulation& dpopulation)
		{
			using FPSize_t = double;
			//compute new np
			size_t new_np = size_t(std::round(
				((FPSize_t(*parameters().m_min_np) - FPSize_t(*parameters().m_np)) / FPSize_t(*parameters().m_max_nfe)) 
				* FPSize_t(m_curr_nfe) 
				+ FPSize_t(*parameters().m_np)
			));
			//max/min safe
			new_np = clamp<size_t>(new_np, *parameters().m_min_np, *parameters().m_np);
			//reduce
			if (new_np < current_np())
			{
				//sort population
				dpopulation.parents().sort();
				//reduce
				dpopulation.resize(new_np);
				//test
				denn_assert(current_np() == new_np);
			}
		}
	};
	REGISTERED_EVOLUTION_METHOD(L_SHADEMethod, "L-SHADE")
}
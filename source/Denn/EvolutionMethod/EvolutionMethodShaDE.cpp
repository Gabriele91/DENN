#include "Denn/Parameters.h"
#include "Denn/Denn/Individual.h"
#include "Denn/Denn/Population.h"
#include "Denn/Denn/Solver.h"
#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Crossover.h"
namespace Denn
{
	class SHADEMethod : public EvolutionMethod
	{
	public:

		SHADEMethod(const Solver& solver, SubPopulation& sub_pop) : EvolutionMethod(solver, sub_pop)
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
			//take tou
			size_t tou_i = random(id_target).index_rand(m_mu_f.size());
			//Compute F
			Scalar v;
			do v = random(id_target).cauchy(m_mu_f[tou_i], 0.1); while (v <= 0);
			output.f() = Denn::sature(v);
			//Cr
			output.cr() = Denn::sature(random(id_target).normal(m_mu_cr[tou_i], 0.1));
			//P
			output.p() = random(id_target).uniform(m_pmin, 0.2);
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
			/////////////////////////////////////////////////////////////
			//get swap list
			performe_selection(m_swap_list);
			/////////////////////////////////////////////////////////////
			// SHADE
			//F
			Scalar sum_f = 0;
			Scalar sum_f2 = 0;
			//CR
			std::vector<Scalar> s_cr;
			std::vector<Scalar> s_delta_f;
			Scalar delta_f = 0;
			Scalar sum_delta_f = 0;
			size_t n_discarded = 0;
			size_t np = current_np();
			//compute shade params
			for (size_t i = 0; i != np; ++i)
			{
				if(m_swap_list[i] < 0) continue;
				//get
				auto father = population(i).parent();
				auto son = population(m_swap_list[i]).son();
				//archivie
				if (m_archive_max_size) m_archive.push_back(father->copy());
				//F
				sum_f += son->f();
				sum_f2 += son->f() * son->f();
				//w_k (for mean of Scr)
				delta_f = std::abs(son->eval() - father->eval());
				s_delta_f.push_back(delta_f);
				sum_delta_f += delta_f;
				//Scr
				s_cr.push_back(son->cr());
				++n_discarded;
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
				m_archive[random().index_rand(m_archive.size())] = m_archive.back();
				m_archive.pop_back();
			}
			/////////////////////////////////////////////////////////////
			//swap
			population().swap(m_swap_list);
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
		IndividualList	    m_archive;
		Mutation::SPtr      m_mutation;
		Crossover::SPtr     m_crossover;
		std::vector<int>    m_swap_list;

	};
	REGISTERED_EVOLUTION_METHOD(SHADEMethod, "SHADE")
	
	class L_SHADEMethod : public EvolutionMethod
	{
	public:

		L_SHADEMethod(const Solver& solver, SubPopulation& sub_pop) : EvolutionMethod(solver, sub_pop)
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
			//get np at the beginning
			m_start_np = population().size();
			//create mutation/crossover
			m_mutation = MutationFactory::create(parameters().m_mutation_type, *this);
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, *this);
		}

		virtual void start_a_subgen_pass() override
		{
			//sort parents
			if (m_mutation->required_sort()) population().sort();
		}

		virtual void end_a_subgen_pass() override
		{
			//compute NFE
			m_curr_nfe += population().size();
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
			//take tou
			size_t tou_i = random(id_target).index_rand(m_mu_f.size());
			//Compute F
			Scalar v;
			do v = random(id_target).cauchy(m_mu_f[tou_i], 0.1); while (v <= 0);
			output.f() = Denn::sature(v);
			//Cr
			output.cr() =  m_mu_cr[tou_i] == *parameters().m_mu_cr_terminal_value
			? Scalar(0.0) 
			: Denn::sature(random(id_target).normal(m_mu_cr[tou_i], 0.1));
			//P
			output.p() = random(id_target).uniform(m_pmin, 0.2);	
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
			/////////////////////////////////////////////////////////////
			//get swap list
			performe_selection(m_swap_list);
			/////////////////////////////////////////////////////////////
			// L-SHADE
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
				auto father = population()[i].parent();
				auto son = population()[i].son();
				if (loss_function_compare(son->eval(),father->eval()))
				{
					if (m_archive_max_size) m_archive.push_back(father->copy());
					//F
					sum_f += son->f();
					sum_f2 += son->f() * son->f();
					//w_k (for mean of Scr)
					delta_f = std::abs(son->eval() - father->eval());
					s_delta_f.push_back(delta_f);
					sum_delta_f += delta_f;
					//Scr
					s_cr.push_back(son->cr());
					++n_discarded;
					//SWAP
					population().swap(i);
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
			/////////////////////////////////////////////////////////////
			//!! SWAP BEFORE TO REDUCE THE POPULATION SIZE !!!
			population().swap(m_swap_list);
			/////////////////////////////////////////////////////////////
			//reduce population
			reduce_population();
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
		size_t				m_start_np{ 0 };
		size_t              m_curr_nfe{ size_t(0) };
		//////////////////////////////////////////////////////
		std::vector<Scalar> m_mu_f;
		std::vector<Scalar> m_mu_cr;
		IndividualList	    m_archive;
		Mutation::SPtr      m_mutation;
		Crossover::SPtr     m_crossover;
		std::vector<int>    m_swap_list;

		//reduce archive
		void reduce_archive()
		{
			while (m_archive_max_size < m_archive.size())
			{
				m_archive[random().index_rand(m_archive.size())] = m_archive.back();
				m_archive.pop_back();
			}
		}
		//reduce population
		void reduce_population()
		{
			using FPSize_t = double;
			//compute new np
			size_t new_np = size_t(std::round(
				((FPSize_t(*parameters().m_min_np) - FPSize_t(m_start_np)) / FPSize_t(*parameters().m_max_nfe)) 
				* FPSize_t(m_curr_nfe) 
				+ FPSize_t(m_start_np)
			));
			//max/min safe
			new_np = clamp<size_t>(new_np, *parameters().m_min_np, m_start_np);
			//reduce
			if (new_np < current_np())
			{
				//sort population
				population().sort();
				//reduce
				population().resize(new_np);
				//test
				denn_assert(population().size() == new_np);
			}
		}
	};
	REGISTERED_EVOLUTION_METHOD(L_SHADEMethod, "L-SHADE")
}
#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	template < typename T >
	class MultiArmedBanditsBAIO
	{
	public:
		using Action = T;
		using ActionPtr = typename T::SPtr;

		MultiArmedBanditsBAIO()
			: m_T(1)
			, m_j(0)
		{}

		MultiArmedBanditsBAIO(const std::vector < ActionPtr >& actions)
		{
			init(actions);
		}

		void init(const std::vector < ActionPtr >& actions)
		{
			m_T		= 0;
			m_j		= 0;
			m_start = true;
			m_actions = actions;
			m_s = std::vector < Scalar >(actions.size(), Scalar(0.0));
			m_n = std::vector < Scalar >(actions.size(), Scalar(0.0));
			m_cost = std::vector < Scalar >(actions.size(), Scalar(std::numeric_limits<Scalar>::max()));
		}

		ActionPtr get()
		{
			return m_actions[m_j];
		}
				
		void update(Scalar rewards, Scalar alpha = 1, Scalar n = 1)
		{
			//update
			m_s[m_j] += rewards;			
			//update count
			m_n[m_j] += n;
			m_T		 += n;
			//first do all
			if (m_start)
			{
				for (size_t i = 0; i != 0; ++i) if (m_n[i] <= 0) { m_j = i; return; };
				m_start = false;
			}
			//update cost
			m_cost[m_j] = (m_s[m_j] / m_n[m_j]) * std::sqrt(alpha * std::log(m_T) / m_n[m_j]);
			//argmax
			m_j = std::max_element(m_cost.begin(), m_cost.end()) - m_cost.begin();
		}

	protected:

		bool					  m_start;
		size_t					  m_j;
		Scalar					  m_T;
		std::vector < ActionPtr > m_actions;
		std::vector < Scalar > 	  m_s;
		std::vector < Scalar > 	  m_n;
		std::vector < Scalar > 	  m_cost;
	};

	class MAB_SHADEMethod : public EvolutionMethod
	{

	public:

		MAB_SHADEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm)
		{
			//init
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
			//clear
			m_mutations_list.clear();
			//create mutation/crossover
			for(const std::string& mut_name : parameters().m_mutations_list_type.get()) 
			{
				m_mutations_list.push_back(MutationFactory::create(mut_name, m_algorithm));
			}
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, m_algorithm);
			//init muts
			m_mutations.init(m_mutations_list);
		}

		virtual void start_a_gen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//Update F/CR??
		}

		virtual void start_a_subgen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//sort parents
			if (m_mutations.get()->required_sort()) dpopulation.parents().sort();
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
			(*m_mutations.get()) (dpopulation.parents(), i_target, i_output);
			//call crossover
			(*m_crossover)(dpopulation.parents(), i_target, i_output);
			//no 0 wights
			i_output.m_network.no_0_weights();
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
			//init
			m_last_rewards = 0.0;
			//
			for (size_t i = 0; i != np; ++i)
			{
				Individual::SPtr father = dpopulation.parents()[i];
				Individual::SPtr son = dpopulation.sons()[i];
				if (loss_function_compare(son->m_eval, father->m_eval))
				{
					if (m_archive_max_size) m_archive.push_back(father->copy());
					//max
					m_last_rewards += std::abs(std::abs(son->m_eval) - std::abs(father->m_eval)); // / std::abs(father->m_eval);
					//F
					sum_f += son->m_f;
					sum_f2 += son->m_f * son->m_f;
					//w_k (for mean of Scr)
					delta_f = std::abs(std::abs(son->m_eval) - std::abs(father->m_eval));
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


		virtual void end_a_gen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//update MAB, chose next
			m_mutations.update(m_last_rewards, current_np(), *parameters().m_sub_gens);
		}

		virtual const VariantRef get_context_data() const override
		{
			return VariantRef(m_archive);
		}

	protected:

		Scalar				m_last_rewards{ 0.0 };
		size_t				m_h{ 0 };
		size_t				m_k{ 0 };
		size_t              m_archive_max_size{ false };
		Scalar				m_pmin{ Scalar(0.0) };
		std::vector<Scalar> m_mu_f;
		std::vector<Scalar> m_mu_cr;
		Population	        m_archive;
		std::vector<Mutation::SPtr>      m_mutations_list;
		MultiArmedBanditsBAIO<Mutation>  m_mutations;
		Crossover::SPtr                  m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(MAB_SHADEMethod, "MAB-SHADE")


}
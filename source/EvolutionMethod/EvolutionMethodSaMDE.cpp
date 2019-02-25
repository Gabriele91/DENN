#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
#include <array>

namespace Denn
{
	//#define DEBUG_SaMDE
	class SaMDEMethod : public EvolutionMethod
	{
	public:

		SaMDEMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm) 
		{
		}
		
		virtual void start() override
		{
			//clear
			m_mutations_list.clear();
			//get mutations
			for(const std::string& mut_name : parameters().m_mutations_list_type.get()) 
			{
				m_mutations_list.push_back(MutationFactory::create(mut_name, m_algorithm));
			}
			//crossover
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, m_algorithm);
			//init metadata
			init_metadata();
			//debug
			#ifdef DEBUG_SaMDE
			m_winners[0].resize(current_np());
			m_winners[1].resize(current_np());
			std::fill(m_winners[0].begin(), m_winners[0].end(), -1);
			std::fill(m_winners[1].begin(), m_winners[1].end(), -1);
			#endif 
		}

		virtual void start_a_subgen_pass(DoubleBufferPopulation& dpopulation) override
		{
			//sort parents
			for(auto mutation : m_mutations_list)
			{
				if(mutation->required_sort())
				{ 		
					//compare
					auto comp =[&](const Individual::SPtr& li, const Individual::SPtr& ri) -> bool 
							   { return loss_function_compare(li->m_eval,ri->m_eval); };
					//pop
					auto& pop  = dpopulation.parents().as_vector();
					auto& meta = metadata_parents();
					//sort
					auto p = sort_permutation(pop, comp);
					apply_permutation_in_place(pop, p);
					apply_permutation_in_place(meta, p);
					break;
				}
			}
		}

		virtual void create_a_individual
		(
			  DoubleBufferPopulation& dpopulation
			, size_t i_target
			, Individual& i_output
		)
		override
		{
			//new metadata
			size_t method = new_metadata(i_target);
			//set
			set_metadata_to_individual(i_target, method, i_output);
			//call muation
			(*m_mutations_list[method]) (dpopulation.parents(), i_target, i_output);
			//call crossover
			(*m_crossover)(dpopulation.parents(), i_target, i_output);
			//no 0 wights
			i_output.m_network.no_0_weights();
			//debug
			#ifdef DEBUG_SaMDE
			m_winners[1][i_target] = method;
			#endif 
		}

		virtual	void selection(DoubleBufferPopulation& dpopulation) override
		{
			size_t np = current_np();
			for (size_t i = 0; i != np; ++i)
			{
				Individual::SPtr father = dpopulation.parents()[i];
				Individual::SPtr son = dpopulation.sons()[i];
				if (loss_function_compare(son->m_eval,father->m_eval))
				{
					dpopulation.swap(i);
					metadata_swap(i);	
					//debug
					#ifdef DEBUG_SaMDE
					m_winners[0][i] = m_winners[1][i];
					#endif 
				}
			}
			//debug
			#ifdef DEBUG_SaMDE
			//counting
			std::vector< long long > count_by_methods(m_mutations_list.size());
			for (long long i : m_winners[0]) if(i >= 0) ++count_by_methods[i];
			//print
			std::cout << "SaMDE: ";
			for (long long c : count_by_methods) std::cout << c << "\t";
			std::cout << std::endl;
			#endif 
		}

	protected:		
		//SaDE
		struct Metadata
		{
			Scalar m_f;
			Scalar m_cr;
			Scalar m_v;
		};
		//mdata alias
		using ListScalar		 = std::vector<Scalar>;
		using ListMetadata       = std::vector<Metadata>;
		using ListListMetadata   = std::vector<ListMetadata>;
		using DoubleListMetadata = std::array<ListListMetadata, 2>;
		//population
		DoubleListMetadata  m_double_list_mdata;
		//help metadata
		void init_metadata()
		{
			//for parent & son
			for(ListListMetadata& list_list_mdata : m_double_list_mdata)
			{
				list_list_mdata.clear();
				list_list_mdata.resize(current_np());
				//for all individuals
				for(ListMetadata& list_mdata : list_list_mdata)
				{
					list_mdata.clear();
					list_mdata.resize(m_mutations_list.size());
					//for all methods
					for(Metadata& mdata : list_mdata)
					{
						mdata.m_f = parameters().m_default_f;
						mdata.m_cr = parameters().m_default_cr;
						mdata.m_v = 1.0;
					}
				}
			} 
		}
		//swap
		void metadata_swap(size_t i)
		{
			std::swap(m_double_list_mdata[0][i],m_double_list_mdata[1][i]);
		}
		//chouse v id 
		size_t stochastic_roulette_wheel(ListMetadata& values, size_t i)
		{
			//max
			Scalar max_v = values.begin()->m_v;
			for (auto& v : values) max_v = std::max(max_v, v.m_v);
			//roulette
			for (;;)
			{
				//id rand		
				size_t id = random(i).uirand(values.size());
				//test
				if (random(i).uniform(0, 1.0) < values[id].m_v / max_v)
					return id;
			}
		}
		size_t roulette_wheel(ListMetadata& values, size_t i)
		{
			//test
			if (!values.size()) return 0;
			//max
			Scalar sum_v{ 0.0 };
			for (auto v : values) sum_v += v.m_v;
			//rand size
			Scalar value = random(i).uniform() * sum_v;
			//roulette
			for (size_t i = 0; i != values.size(); ++i)
			{
				value -= values[i].m_v;
				if (value < 0) return i;
			}
			return values.size() - 1;
		}
		//update metadata
		size_t new_metadata(size_t i)
		{
			//get vector[ f, cr, v ]
			ListListMetadata& l_m_parents = metadata_parents();
			ListListMetadata& l_m_sons = metadata_sons();
			//ref to dest
			ListMetadata& mdata_son = l_m_sons[i];
			//metadata
			Scalar f_prime = random(i).uniform(0.7, 1.0);
			//get deck
			auto& deck = random(i).deck();
			//reinit
			deck.reinit(current_np());
			deck.reset();
			//rand 
			size_t r1 = deck.get_random_id(i);
			size_t r2 = deck.get_random_id(i);
			size_t r3 = deck.get_random_id(i);
			//random m data
			const ListMetadata& mdata_r1  = l_m_parents[r1];
			const ListMetadata& mdata_r2  = l_m_parents[r2];
			const ListMetadata& mdata_r3  = l_m_parents[r3];
			//Update V[0-k]
			for(size_t k = 0; k!=m_mutations_list.size(); ++k)
			{
				//new
				mdata_son[k].m_v = mdata_r1[k].m_v + f_prime * (mdata_r2[k].m_v - mdata_r3[k].m_v);
			}
			//chouse winner
			size_t w = roulette_wheel(mdata_son, i);
			//update f and cr
			mdata_son[w].m_f  = mdata_r1[w].m_f + f_prime * (mdata_r2[w].m_f - mdata_r3[w].m_f );
			mdata_son[w].m_cr = mdata_r1[w].m_cr + f_prime * (mdata_r2[w].m_cr - mdata_r3[w].m_cr );
			//return winner
			return w;
		}
		//son/parent
		ListListMetadata& metadata_parents()
		{
			return m_double_list_mdata[0];
		}
		ListListMetadata& metadata_sons()
		{
			return m_double_list_mdata[1];
		}
		ListMetadata& metadata_parent(size_t i)
		{
			return m_double_list_mdata[0][i];
		}
		ListMetadata& metadata_son(size_t i)
		{
			return m_double_list_mdata[1][i];
		}
		//set value to son
		void set_metadata_to_individual(size_t i, size_t w, Individual& ind)
		{
			//meta
			auto& meta = metadata_son(i)[w];
			ind.m_f  = meta.m_f ;
			ind.m_cr = meta.m_cr ;
		}
		//mutation pool
		std::vector<Mutation::SPtr>  m_mutations_list;
		Crossover::SPtr				 m_crossover;

		#ifdef DEBUG_SaMDE
		std::vector< long long > m_winners[2];
		#endif 

	};
	REGISTERED_EVOLUTION_METHOD(SaMDEMethod, "SAMDE")
}
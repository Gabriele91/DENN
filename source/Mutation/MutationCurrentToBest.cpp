#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class CurrentToBest : public Mutation
	{
	public:

		CurrentToBest(const DennAlgorithm& algorithm):Mutation(algorithm) 
		{ 
		}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			//target
			const Individual& i_target = *population[id_target];
			//best (n.b. JADE sort population from best to worst)
			const Individual& i_best = *population.best();
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//do rand
					rand_deck.reset();
					//do cross + mutation
					const Individual& nn_a = *population[rand_deck.get_random_id(id_target)];
					const Individual& nn_b = *population[rand_deck.get_random_id(id_target)];
					//from_archive ? archive[r2] : father(r2);
					auto  w_final  = i_final[i_layer][m];
					auto& w_target = i_target[i_layer][m];
					auto& w_best   = i_best[i_layer][m];
					auto& x_a = nn_a[i_layer][m];
					auto& x_b = nn_b[i_layer][m];
					w_final = ( w_target + ((w_best - w_target) + (x_a - x_b)) * f ).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}

	};
	REGISTERED_MUTATION(CurrentToBest, "curr_best")

	class CurrentToPBest : public Mutation
	{
	public:

		//required sort
		virtual bool required_sort() const override { return true; }

		CurrentToPBest(const DennAlgorithm& algorithm):Mutation(algorithm) 
		{ 
			//Get archive
			if(m_algorithm.evolution_method().get_context_data().get_type() == static_variant_type<Population>())
			{
				m_archive = m_algorithm.evolution_method().get_context_data().get_ptr<Population>();
			}
			//Get percentage of best individuals
			m_perc_of_best = m_algorithm.parameters().m_perc_of_best;
		}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			const auto& p = i_final.m_p;
			//target
			const Individual& i_target = *population[id_target];
			//best (n.b. sort population from best to worst)
			size_t			range_best = size_t(p*Scalar(current_np()));
			size_t           id_best   = range_best ? random(id_target).index_rand(range_best) : size_t(0);
			const Individual& i_best   = *population[id_best];
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(current_np());
			rand_deck.reset();
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//do rand
					rand_deck.reset();
					//do cross + mutation
					const Individual& nn_a = *population[rand_deck.get_random_id(id_target)];
					//b from archive  or pop
					Individual::SPtr nn_b  = nullptr;
					//b from archive (JADE)
					if(m_archive)
					{
						size_t rand_b = random(id_target).index_rand(m_archive->size() + population.size() - 2);
						bool get_from_archive = rand_b < m_archive->size();
						nn_b = get_from_archive ? (*m_archive)[rand_b] : population[rand_deck.get_random_id(id_target)];
					}
					else 
					{
						nn_b =  population[rand_deck.get_random_id(id_target)];
					}
					//from_archive ? archive[r2] : father(r2);
					auto  w_final  = i_final[i_layer][m];
					auto& w_target = i_target[i_layer][m];
					auto& w_best   = i_best[i_layer][m];
					auto& x_a 	   = nn_a[i_layer][m];
					auto  x_b 	   = (*nn_b)[i_layer][m];
					w_final = ( w_target + ((w_best - w_target) + (x_a - x_b)) * f ).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}

	protected:

		Scalar m_perc_of_best;
		const Population* m_archive{ nullptr };

	};
	REGISTERED_MUTATION(CurrentToPBest, "curr_p_best")
}
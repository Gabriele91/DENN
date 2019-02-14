#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class BestOne : public Mutation
	{
	public:

		BestOne(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			//target
			const Individual& i_target = *population[id_target];			
			//best
			size_t id_best;
			Scalar eval_best;
			population.best(id_best,eval_best);
			const Individual& i_best = *population[id_best];
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
					const Individual& nn_a = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_b = *population[rand_deck.get_random_id(id_best)];
					//
					auto w_final  = i_final[i_layer][m];
					auto& x_best = i_best[i_layer][m];
					auto& x_a    = nn_a[i_layer][m];
					auto& x_b    = nn_b[i_layer][m];
					w_final = (x_best + (x_a - x_b) * f).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(BestOne, "best/1")

	class BestTwo : public Mutation
	{
	public:

		BestTwo(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			//target
			const Individual& i_target = *population[id_target];
			//best
			size_t id_best;
			Scalar eval_best;
			population.best(id_best,eval_best);
			const Individual& i_best = *population[id_best];
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
					const Individual& nn_a = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_b = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_c = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_d = *population[rand_deck.get_random_id(id_best)];
					//
					auto w_final = i_final[i_layer][m];
					auto& w_best = i_best[i_layer][m];
					auto& x_a = nn_a[i_layer][m];
					auto& x_b = nn_b[i_layer][m];
					auto& x_c = nn_c[i_layer][m];
					auto& x_d = nn_d[i_layer][m];
					w_final = (w_best + ((x_a - x_b) + (x_c - x_d)) * f).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(BestTwo, "best/2")

	class GlobalBestOne : public Mutation
	{
	public:

		GlobalBestOne(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			//target
			const Individual& i_target = *population[id_target];
			//id best
			size_t id_best =  std::numeric_limits<size_t>::max();
			//best
			Individual::SPtr best = m_algorithm.best_context().m_best;
			//best
			if(!best)
			{
				Scalar eval_best;
				population.best(id_best,eval_best);
				best = population[id_best];
			}
			//ref to best
			const Individual& i_best = *best;
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
					const Individual& nn_a = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_b = *population[rand_deck.get_random_id(id_best)];
					//
					auto w_final = i_final[i_layer][m];
					auto& w_best = i_best[i_layer][m];
					auto& x_a = nn_a[i_layer][m];
					auto& x_b = nn_b[i_layer][m];
					w_final = (w_best + (x_a - x_b) * f).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(GlobalBestOne, "global_best/1")

	class GlobalBestTwo : public Mutation
	{
	public:

		GlobalBestTwo(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//alias
			const auto& f = i_final.m_f;
			//target
			const Individual& i_target = *population[id_target];
			//id best
			size_t id_best =  std::numeric_limits<size_t>::max();
			//best
			Individual::SPtr best = m_algorithm.best_context().m_best;
			//best
			if(!best)
			{
				Scalar eval_best;
				population.best(id_best,eval_best);
				best = population[id_best];
			}
			//ref to best
			const Individual& i_best = *best;
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
					const Individual& nn_a = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_b = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_c = *population[rand_deck.get_random_id(id_best)];
					const Individual& nn_d = *population[rand_deck.get_random_id(id_best)];
					//
					auto w_final = i_final[i_layer][m];
					auto& w_best = i_best[i_layer][m];
					auto& x_a = nn_a[i_layer][m];
					auto& x_b = nn_b[i_layer][m];
					auto& x_c = nn_c[i_layer][m];
					auto& x_d = nn_d[i_layer][m];
					w_final = (w_best + ((x_a - x_b) + (x_c - x_d)) * f).unaryExpr(m_algorithm.clamp_function());
				}
			}
		}
	};
	REGISTERED_MUTATION(GlobalBestTwo, "global_best/2")
}
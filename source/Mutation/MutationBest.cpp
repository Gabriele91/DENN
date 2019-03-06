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

		virtual void operator()
		(
			const PopulationSlider& population,			
			size_t id_target,
			IndividualSlider& output
		) override
		{		
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//id best
			size_t id_best = population.best_id();
			//
			auto& x_best = population[id_best];
			auto& x_a    = population[rand_deck.get_random_id(id_best)];
			auto& x_b    = population[rand_deck.get_random_id(id_best)];
			(*output) 	 = (x_best + (x_a - x_b) * output.f()).unaryExpr(m_algorithm.clamp_function());
		}
	};
	REGISTERED_MUTATION(BestOne, "best/1")

	class BestTwo : public Mutation
	{
	public:

		BestTwo(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()
		(
			const PopulationSlider& population, 		
			size_t id_target,
			IndividualSlider& output
		) override
		{
			
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//id best
			size_t id_best = population.best_id();
			//
			auto& x_best  = population[id_best];
			auto& x_a     = population[rand_deck.get_random_id(id_best)];
			auto& x_b     = population[rand_deck.get_random_id(id_best)];
			auto& x_c     = population[rand_deck.get_random_id(id_best)];
			auto& x_d     = population[rand_deck.get_random_id(id_best)];
			(*output) = (x_best + ((x_a - x_b) + (x_c - x_d)) * output.f()).unaryExpr(m_algorithm.clamp_function());
		}
	};
	REGISTERED_MUTATION(BestTwo, "best/2")

}
#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class RandOne : public Mutation
	{
	public:

		RandOne(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()
		(
			const PopulationSlider& population, 		
			size_t id_target,
			IndividualSlider& output
		) override 
		{
			//init generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//
			auto& x_a = population[rand_deck.get_random_id(id_target)];
			auto& x_b = population[rand_deck.get_random_id(id_target)];
			auto& x_c = population[rand_deck.get_random_id(id_target)];
			(*output) = (x_a + (x_b - x_c) * output.f()).unaryExpr(m_algorithm.clamp_function());
		}
	};
	REGISTERED_MUTATION(RandOne, "rand/1")

	class RandTwo : public Mutation
	{
	public:

		RandTwo(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()
		(
			const PopulationSlider& population, 		
			size_t id_target,
			IndividualSlider& output
		) override 
		{
			//init generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//
			auto& x_a = population[rand_deck.get_random_id(id_target)];
			auto& x_b = population[rand_deck.get_random_id(id_target)];
			auto& x_c = population[rand_deck.get_random_id(id_target)];
			auto& x_d = population[rand_deck.get_random_id(id_target)];
			auto& x_e = population[rand_deck.get_random_id(id_target)];
			(*output) = (x_a + ((x_b - x_c) + (x_d - x_e)) * output.f()).unaryExpr(m_algorithm.clamp_function());
		}
	};
	REGISTERED_MUTATION(RandTwo, "rand/2")
}
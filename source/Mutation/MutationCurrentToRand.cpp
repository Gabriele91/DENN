#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class CurrToRandOne : public Mutation
	{
	public:

		CurrToRandOne(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

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
			//
			auto& w_target = population[id_target];
			auto& x_a = population[rand_deck.get_random_id(id_target)];
			auto& x_b = population[rand_deck.get_random_id(id_target)];
			auto& x_c = population[rand_deck.get_random_id(id_target)];
			(*output) = (w_target + ((x_a - w_target) + (x_b - x_c)) * output.f()).unaryExpr(m_algorithm.clamp_function());
		}
	};
	REGISTERED_MUTATION(CurrToRandOne, "curr_to_rand/1")
}
#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class Trigonometric : public Mutation
	{
	public:

		Trigonometric(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

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
			//do rand
			rand_deck.reset();
			//do cross + mutation
			IndividualSlider nn_a = population.individual(rand_deck.get_random_id(id_target));
			IndividualSlider nn_b = population.individual(rand_deck.get_random_id(id_target));
			IndividualSlider nn_c = population.individual(rand_deck.get_random_id(id_target));
			//parameters
			Scalar p = std::abs(nn_a.eval()) + std::abs(nn_b.eval()) + std::abs(nn_c.eval());
			Scalar p_a = nn_a.eval() / p;
			Scalar p_b = nn_b.eval()/ p;
			Scalar p_c = nn_c.eval() / p;
			//mutation
			auto x_a = *nn_a;
			auto x_b = *nn_b;
			auto x_c = *nn_c;
			(*output) = (x_a + x_b + x_c) / Scalar(3) + (p_b -p_a) * (x_a - x_b) + (p_c - p_b) * (x_b - x_c) + (p_a - p_c) * (x_c - x_a);
			(*output) = (*output).unaryExpr(m_algorithm.clamp_function());
        }
	};
	REGISTERED_MUTATION(Trigonometric, "trig")
}
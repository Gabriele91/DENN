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

		virtual void operator()(const Population& population, size_t id_target, Individual& i_final) override
		{
			//target
			const Individual& i_target = *population[id_target];
			//init generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			//do rand
			rand_deck.reset();
			//do cross + mutation
			const Individual& nn_a = *population[rand_deck.get_random_id(id_target)];
			const Individual& nn_b = *population[rand_deck.get_random_id(id_target)];
			const Individual& nn_c = *population[rand_deck.get_random_id(id_target)];
			//parameters
			Scalar p = std::abs(nn_a.m_eval) + std::abs(nn_b.m_eval) + std::abs(nn_c.m_eval);
			Scalar p_a = nn_a.m_eval / p;
			Scalar p_b = nn_b.m_eval / p;
			Scalar p_c = nn_c.m_eval / p;
			//for each layer
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and biases
				for ( size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//mutation
					auto  w_final = i_final[i_layer][m];
					auto& x_a = nn_a[i_layer][m];
					auto& x_b = nn_b[i_layer][m];
					auto& x_c = nn_c[i_layer][m];
					w_final = (x_a + x_b + x_c) / Scalar(3) + (p_b -p_a) * (x_a - x_b) + (p_c - p_b) * (x_b - x_c) + (p_a - p_c) * (x_c - x_a);
					w_final = w_final.unaryExpr(m_algorithm.clamp_function());
				}
			}
        }
	};
	REGISTERED_MUTATION(Trigonometric, "trig")
}
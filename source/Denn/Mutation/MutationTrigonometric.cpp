#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class Trigonometric : public Mutation
	{
	public:

		Trigonometric(const EvolutionMethod& emethod) : Mutation(emethod) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		) override
		{
			//init generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population().size());
			//do rand
			rand_deck.reset();
			//do cross + mutation
			auto nn_a = parent(rand_deck.get_random_id(id_target));
			auto nn_b = parent(rand_deck.get_random_id(id_target));
			auto nn_c = parent(rand_deck.get_random_id(id_target));
			//parameters
			Scalar p = std::abs(nn_a.eval()) + std::abs(nn_b.eval()) + std::abs(nn_c.eval());
			Scalar p_a = nn_a.eval() / p;
			Scalar p_b = nn_b.eval()/ p;
			Scalar p_c = nn_c.eval() / p;
			//mutation
			auto x_a = nn_a.matrix();
			auto x_b = nn_b.matrix();
			auto x_c = nn_c.matrix();
			output.matrix() = (x_a + x_b + x_c) / Scalar(3) + (p_b -p_a) * (x_a - x_b) + (p_c - p_b) * (x_b - x_c) + (p_a - p_c) * (x_c - x_a);
			output.matrix() = output.matrix().unaryExpr(solver().clamp_function());
        }
	};
	REGISTERED_MUTATION(Trigonometric, "trig")
}
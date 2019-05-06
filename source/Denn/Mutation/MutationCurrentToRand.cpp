#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class CurrToRandOne : public Mutation
	{
	public:

		CurrToRandOne(const EvolutionMethod& emethod) : Mutation(emethod) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		) override
		{
			//get generator
			auto& rand_deck = random(id_target).deck();
			auto  id_best   = population().best_parent_id();
			//set population size in deck
			rand_deck.reinit(population().size());
			rand_deck.reset();
			//
			auto& w_target = parent(id_target).array();
			auto& x_a = parent(rand_deck.get_random_id(id_best)).array();
			auto& x_b = parent(rand_deck.get_random_id(id_best)).array();
			auto& x_c = parent(rand_deck.get_random_id(id_best)).array();
			output.array() = (w_target + ((x_a - w_target) + (x_b - x_c)) * output.f()).unaryExpr(solver().clamp_function());
		}
	};
	REGISTERED_MUTATION(CurrToRandOne, "curr_to_rand/1")
}
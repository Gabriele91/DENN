#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class BestOne : public Mutation
	{
	public:

		BestOne(const EvolutionMethod& emethod) : Mutation(emethod) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		) override
		{		
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population().size());
			rand_deck.reset();
			//id best
			size_t id_best = best_parent_id();
			//
			auto& x_best    = parent(id_best).matrix();
			auto& x_a       = parent(rand_deck.get_random_id(id_best)).matrix();
			auto& x_b       = parent(rand_deck.get_random_id(id_best)).matrix();
			output.matrix() = (x_best + (x_a - x_b) * output.f()).unaryExpr(solver().clamp_function());
		}
	};
	REGISTERED_MUTATION(BestOne, "best/1")

	class BestTwo : public Mutation
	{
	public:

		BestTwo(const EvolutionMethod& emethod) : Mutation(emethod) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		) override
		{
			
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population().size());
			rand_deck.reset();
			//id best
			size_t id_best = best_parent_id();
			//
			auto& x_best = parent(id_best).matrix();
			auto& x_a    = parent(rand_deck.get_random_id(id_best)).matrix();
			auto& x_b    = parent(rand_deck.get_random_id(id_best)).matrix();
			auto& x_c    = parent(rand_deck.get_random_id(id_best)).matrix();
			auto& x_d    = parent(rand_deck.get_random_id(id_best)).matrix();
			output.matrix() = (x_best + ((x_a - x_b) + (x_c - x_d)) * output.f()).unaryExpr(solver().clamp_function());
		}
	};
	REGISTERED_MUTATION(BestTwo, "best/2")

}
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class RandOne : public Mutation
	{
	public:

		RandOne(const EvolutionMethod& emethod) : Mutation(emethod) {}

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
			rand_deck.reset();
			//
			auto& x_a = parent(rand_deck.get_random_id(id_target)).matrix();
			auto& x_b = parent(rand_deck.get_random_id(id_target)).matrix();
			auto& x_c = parent(rand_deck.get_random_id(id_target)).matrix();
			output.matrix() = (x_a + (x_b - x_c) * output.f()).unaryExpr(solver().clamp_function());
		}
	};
	REGISTERED_MUTATION(RandOne, "rand/1")

	class RandTwo : public Mutation
	{
	public:

		RandTwo(const EvolutionMethod& emethod) : Mutation(emethod) {}

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
			rand_deck.reset();
			//
			auto& x_a = parent(rand_deck.get_random_id(id_target)).matrix();
			auto& x_b = parent(rand_deck.get_random_id(id_target)).matrix();
			auto& x_c = parent(rand_deck.get_random_id(id_target)).matrix();
			auto& x_d = parent(rand_deck.get_random_id(id_target)).matrix();
			auto& x_e = parent(rand_deck.get_random_id(id_target)).matrix();
			output.matrix()  = (x_a + ((x_b - x_c) + (x_d - x_e)) * output.f()).unaryExpr(solver().clamp_function());
		}
	};
	REGISTERED_MUTATION(RandTwo, "rand/2")
}
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class NoneMutation : public Mutation
	{
	public:
		NoneMutation(const EvolutionMethod& emethod) : Mutation(emethod) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		) override
		{
			//target
			output.matrix() = parent(id_target).matrix();
		}
	};
	REGISTERED_MUTATION(NoneMutation, "none")
}
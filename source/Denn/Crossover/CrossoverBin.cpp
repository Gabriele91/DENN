#include "Denn/Parameters.h"
#include "Denn/Denn/Crossover.h"

namespace Denn
{
	class Bin : public Crossover
	{
	public:
		Bin(const EvolutionMethod& method) : Crossover(method) {}

		virtual void operator()
		(
			  size_t id_target,
				Individual& output
	  ) override
		{
			//elements
			auto w_target = parent(id_target).array();
			auto w_mutant = output.array();
			//random i
			size_t e_rand = random(id_target).index_rand(w_target.size());
			//cross
			for (decltype(w_target.size()) e = 0; e != w_target.size(); ++e)
			{
				//crossover
				//!(RandomIndices::random() < cr || e_rand == e)
				if (e_rand != e && output.cr() <= random(id_target).uniform())
				{
					w_mutant(e) = w_target(e);
				}
			}
		}
    };
	REGISTERED_CROSSOVER(Bin,"bin")
}
#include "Denn/Denn/Crossover.h"
#include "Denn/Parameters.h"

namespace Denn
{
	class Interm : public Crossover
	{
	public:
		Interm(const EvolutionMethod& method) : Crossover(method) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		)
		{
			auto w_target = parent(id_target).matrix().array();
			auto w_mutant = output.matrix().array();
			//CROSS
			for (decltype(w_target.size()) e = 0; e != w_target.size(); ++e)
			{
				Scalar factor = random(id_target).uniform();
				w_mutant(e) = w_target(e) + factor * (w_mutant(e) - w_target(e));
			}
		}
	};
	REGISTERED_CROSSOVER(Interm, "interm")

}

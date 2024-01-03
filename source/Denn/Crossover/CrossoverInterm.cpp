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
			auto w_target = parent(id_target).array();
			auto w_mutant = output.array();
			//CROSS
			for (decltype(w_target.size()) e = 0; e != w_target.size(); ++e)
			{
				Scalar factor = random(id_target).uniform();
				w_mutant(e) = w_target(e) + factor * (w_mutant(e) - w_target(e));
			}
		}
	};
	REGISTERED_CROSSOVER(Interm, "interm")

	class Interm2 : public Crossover
	{
	public:
		Interm2(const EvolutionMethod& method) : Crossover(method) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		)
		{
			auto w_target = parent(id_target).array();
			auto w_mutant = output.array();
			//CROSS
			for (decltype(w_target.size()) e = 0; e != w_target.size(); ++e)
			{
				w_mutant(e) = (Scalar(1)-output.cr()) * w_target(e) + output.cr() * w_mutant(e);
			}
		}
	};
	REGISTERED_CROSSOVER(Interm2, "interm2")

}
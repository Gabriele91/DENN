#include "Denn/Denn/Crossover.h"
#include "Denn/Parameters.h"

namespace Denn
{
	class None : public Crossover
	{
	public:
		None(const EvolutionMethod& emethod) : Crossover(emethod) {}

		virtual void operator()
		(
			size_t id_target,
			Individual& output
		) override
		{
			//none
		}
	};
	REGISTERED_CROSSOVER(None, "none")
}
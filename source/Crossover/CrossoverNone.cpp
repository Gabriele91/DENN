#include "Denn/Crossover.h"
#include "Denn/Parameters.h"

namespace Denn
{
	class None : public Crossover
	{
	public:
		None(const DennAlgorithm& algorithm) : Crossover(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_mutant)
		{
			//none
		}
	};
	REGISTERED_CROSSOVER(None, "none")
}
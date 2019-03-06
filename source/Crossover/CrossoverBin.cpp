#include "Denn/Crossover.h"
#include "Denn/Parameters.h"

namespace Denn
{
	class Bin : public Crossover
	{
	public:
		Bin(const DennAlgorithm& algorithm) : Crossover(algorithm) {}

		virtual void operator()
		(
				const PopulationSlider& population, 
				size_t id_target,
				IndividualSlider& output
	  ) override
		{
			//elements
			auto w_target = population[id_target].array();
			auto w_mutant = (*output).array();
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
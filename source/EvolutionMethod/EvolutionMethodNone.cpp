#include "Denn/Parameters.h"
#include "Denn/Individual.h"
#include "Denn/Population.h"
#include "Denn/Algorithm.h"
#include "Denn/EvolutionMethod.h"
#include "Denn/Mutation.h"
#include "Denn/Crossover.h"
namespace Denn
{
	class NoneMethod : public EvolutionMethod
	{
	public:

		NoneMethod(const DennAlgorithm& algorithm) : EvolutionMethod(algorithm) {}

		virtual void create_a_individual
		(
			  DoubleBufferPopulation& population
			, size_t i_target
			, Individual& i_output
		)
		override
		{
			//none
		}

		virtual	void selection(DoubleBufferPopulation& population) override
		{
			//none
		}

	private:

	};
	REGISTERED_EVOLUTION_METHOD(NoneMethod,"NONE")
}
#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class NoneMutation : public Mutation
	{
	public:

		NoneMutation(const DennAlgorithm& algorithm) :Mutation(algorithm) {}

		virtual void operator()
		(
			const PopulationSlider& population, 		
			size_t id_target,
			IndividualSlider& output
		) override 
		{
			//target
			(*output) = population[id_target];
		}
	};
	REGISTERED_MUTATION(NoneMutation, "none")
}
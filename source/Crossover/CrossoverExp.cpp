#include "Denn/Crossover.h"
#include "Denn/Parameters.h"

namespace Denn
{
	class Exp : public Crossover
	{
	public:
		Exp(const DennAlgorithm& algorithm) : Crossover(algorithm) {}

		virtual void operator()(const Population& population, size_t id_target, Individual& i_mutant)
		{
			//baias
			const auto& i_target = *population[id_target];
			const auto& cr = i_mutant.m_cr;
			//for each layers
			for (size_t i_layer = 0; i_layer != i_target.size(); ++i_layer)
			{
				//weights and baias
				for (size_t m = 0; m != i_target[i_layer].size(); ++m)
				{
					//elements
					auto w_target = i_target[i_layer][m].array();
					auto w_mutant = i_mutant[i_layer][m].array();
					//random i
					size_t e_rand = random(id_target).index_rand(w_target.size());
					size_t e_start = random(id_target).index_rand(w_target.size());
					//event
					bool copy_event = false;
					//CROSS
					for (decltype(w_target.size()) e = 0; e != w_target.size(); ++e)
					{
						//id circ
						size_t e_circ = (e_start + e) % w_target.size();
						//crossover
						//!(RandomIndices::random() < cr || e_rand == e)
						copy_event |= (e_rand != e_circ && cr <= random(id_target).uniform());
						//copy all vector
						if (copy_event)
						{
							w_mutant(e_circ) = w_target(e_circ);
						}
					}
				}
			}
		}
	};
	REGISTERED_CROSSOVER(Exp, "exp")
}
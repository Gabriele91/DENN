#include "Denn/Parameters.h"
#include "Denn/Denn/Solver.h"

namespace Denn
{
	NeuralNetwork::SPtr Solver::build_neural() const
	{
		NeuralNetwork::SPtr newnn = m_start_network.copy();
		for(auto subpop : population())
		{
			Individual::SPtr individual = nullptr;
			switch (best.build_type)
			{
				default:
				case BuildNetwork::BN_BEST:
					individual = subpop->parents()[
						subpop->best_parent_id(loss_function()->minimize())
					];
				break;
				case BuildNetwork::BN_PBEST:
					individual = subpop->parents()[
						subpop->pbest_parent_id(random(), loss_function()->minimize())
					];
				break;
				case BuildNetwork::BN_ROULETTE:
					individual = subpop->parents()[
						subpop->roulette_wheel_selection_parent_id(random(), loss_function()->minimize())
					];
				case BuildNetwork::BN_RANDOM:
					individual = subpop->parents()[random().index_rand(subpop->size())];
				break;
			}
			individual->copy_to(*newnn);
		}
		return newnn;
	}
	NeuralNetwork::SPtr Solver::build_neural_network(Individual& ind, size_t thread_id) const 
	{
		NeuralNetwork::SPtr newnn = nullptr;
		{
			std::lock_guard<std::mutex>  lock(m_mutex);
			newnn = m_start_network.copy();
			newnn->random() = &random(thread_id);
		}
		//subpop id
        size_t subpop_id = 0;
		size_t subpop_ind_id = 0;
		//search id
		if(best.build_type == BuildNetwork::BN_RANDOM)
		{
			for(auto subpop : population())
			for(size_t i = 0 ; i < subpop->size(); ++i)
			{
				if(ind.get_ptr() == subpop->parents()[i])
				{
					subpop_ind_id = subpop_id;
					break;
				}
				else if(ind.get_ptr() == subpop->sons()[i])
				{
					subpop_ind_id = subpop_id;
					break;
				}
			}
		}
		//pop
		for(auto subpop : population())
		{
			//cases
			if(ind.subpopulation() == subpop.get())
			{
				ind.copy_to(*newnn);
			}
			else
			{
				std::lock_guard<std::mutex>  lock(m_mutex);
				Individual::SPtr individual = nullptr;
				switch (best.build_type)
				{
					default:
					case BuildNetwork::BN_BEST:
						individual = subpop->parents()[
							subpop->best_parent_id(loss_function()->minimize())
						];
					break;
					case BuildNetwork::BN_PBEST:
						individual = subpop->parents()[
							subpop->pbest_parent_id(random(thread_id), loss_function()->minimize())
						];
					break;
					case BuildNetwork::BN_ROULETTE:
						individual = subpop->parents()[
							subpop->roulette_wheel_selection_parent_id(random(thread_id), loss_function()->minimize())
						];
					break;
					case BuildNetwork::BN_RANDOM:
						individual = subpop->parents()[random().index_rand(subpop->size())];
					break;
					case BuildNetwork::BN_LINE:
						individual = subpop->parents()[subpop_ind_id];
					break;
				}
				individual->copy_to(*newnn);
			}
			++subpop_id;
		}
		return newnn;
	}

	//all nets
	std::vector<NeuralNetwork::SPtr> Solver::build_line_np_neural() const
	{
		//nests
		std::vector<NeuralNetwork::SPtr>  nets;
		//compute np
		size_t np = compute_min_np(population().description());
		//combine
		for (size_t n = 0; n < np; ++n)
		{
			//copy network
			auto newnn = m_start_network.copy();
			newnn->random() = &random();
			//create the i-network
			for (auto subpop : population())
				subpop->parents()[n]->copy_to(*newnn);
			//..
			nets.push_back(newnn);
		}
		return nets;
	}

	static std::vector<NeuralNetwork::SPtr> gen_cross_neural_networks
	(
		Random& random,
		const NeuralNetwork& start_network,
		const Denn::Population& population,
		size_t spop = 0
	)
	{
		std::vector<NeuralNetwork::SPtr> nnets;
		if(spop == population.size() - 1)
		{
			for (size_t n = 0; n < population[spop]->size(); ++n)
			{
				auto newnn = start_network.copy();
				newnn->random() = &random;
				population[spop]->parents()[n]->copy_to(*newnn);
				nnets.push_back(newnn);
			}
		}
		else 
		{
			std::vector<NeuralNetwork::SPtr>  sub_nnets = gen_cross_neural_networks(random, start_network, population, spop+1);
			for (size_t n = 0; n < population[spop]->size(); ++n)
			{
				for (size_t s = 0; s < sub_nnets.size(); ++s)
				{
					auto newnn = sub_nnets[s]->copy();
					newnn->random() = &random;
					population[spop]->parents()[n]->copy_to(*newnn);
					nnets.push_back(newnn);
				}
			}
		}
		return nnets;
	}

	std::vector<NeuralNetwork::SPtr> Solver::build_cross_np_neural() const
	{
		return gen_cross_neural_networks(random(), m_start_network, population());
	}
}
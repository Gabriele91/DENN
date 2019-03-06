#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class CurrentToBest : public Mutation
	{
	public:

		CurrentToBest(const DennAlgorithm& algorithm):Mutation(algorithm) 
		{ 
		}

		virtual void operator()		
		(
			const PopulationSlider& population, 		
			size_t id_target,
			IndividualSlider& output
		) 
		override
		{
			//deck
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population.size());
			rand_deck.reset();
			//alias
			auto& w_target = population[id_target];
			auto& w_best   = population[population.best_id()];
			auto& x_a 	   = population[rand_deck.get_random_id(id_target)];
			auto& x_b 	   = population[rand_deck.get_random_id(id_target)];
			(*output)	   = ( w_target + ((w_best - w_target) + (x_a - x_b)) * output.f() ).unaryExpr(m_algorithm.clamp_function());
		}

	};
	REGISTERED_MUTATION(CurrentToBest, "curr_best")

	class CurrentToPBest : public Mutation
	{
	public:

		//required sort
		virtual bool required_sort() const override { return true; }

		CurrentToPBest(const DennAlgorithm& algorithm):Mutation(algorithm) 
		{ 
			//Get archive
			if(m_algorithm.evolution_method().get_context_data().get_type() == static_variant_type<Population>())
			{
				m_archive = m_algorithm.evolution_method().get_context_data().get_ptr<Population>();
			}
			//Get percentage of best individuals
			m_perc_of_best = m_algorithm.parameters().m_perc_of_best;
		}

		virtual void operator()
		(
			const PopulationSlider& population, 		
			size_t id_target,
			IndividualSlider& output
		) override
		{		
			//get generator
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(current_np());
			rand_deck.reset();
			//best (n.b. sort population from best to worst)
			size_t range_best = size_t(output.p() * Scalar(current_np()));
			size_t id_best    = range_best ? random(id_target).index_rand(range_best) : size_t(0);
			//id a
			size_t id_a = rand_deck.get_random_id(id_target);
			//from_archive ? archive[r2] : father(r2);
			auto& w_target = population[id_target];
			auto& w_best   = population[id_best];
			auto& x_a 	   = population[id_a];
			//b from archive (JADE/SHADE)
			if(m_archive)
			{
				size_t rand_b = random(id_target).index_rand(m_archive->size() + population.size() - 2);
				auto x_b = rand_b < m_archive->size()
				? population.apply_slider(*(*m_archive)[rand_b])
				: population[rand_deck.get_random_id(id_target)];
				(*output) = ( w_target + ((w_best - w_target) + (x_a - x_b)) * output.f() ).unaryExpr(m_algorithm.clamp_function());
			}
			else 
			{
				auto x_b = population[rand_deck.get_random_id(id_target)];
				(*output) = ( w_target + ((w_best - w_target) + (x_a - x_b)) * output.f() ).unaryExpr(m_algorithm.clamp_function());
			}
	
		}

	protected:

		Scalar m_perc_of_best;
		const Population* m_archive{ nullptr };

	};
	REGISTERED_MUTATION(CurrentToPBest, "curr_p_best")
}
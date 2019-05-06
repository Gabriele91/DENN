#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>
namespace Denn
{
	class CurrentToBest : public Mutation
	{
	public:

		CurrentToBest(const EvolutionMethod& emethod) : Mutation(emethod) {}

		virtual void operator()		
		(
			size_t id_target,
			Individual& output
		) 
		override
		{
			//deck
			auto& rand_deck = random(id_target).deck();
			//set population size in deck
			rand_deck.reinit(population().size());
			rand_deck.reset();
			//id best
			size_t id_best = population().best_parent_id();
			//alias
			auto& w_target = parent(id_target).array();
			auto& w_best   = parent(id_best).array();
			auto& x_a 	   = parent(rand_deck.get_random_id(id_best)).array();
			auto& x_b 	   = parent(rand_deck.get_random_id(id_best)).array();
			output.array()= ( w_target + ((w_best - w_target) + (x_a - x_b)) * output.f() ).unaryExpr(solver().clamp_function());
		}

	};
	REGISTERED_MUTATION(CurrentToBest, "curr_best")

	class CurrentToPBest : public Mutation
	{
	public:

		//required sort
		virtual bool required_sort() const override { return true; }

		CurrentToPBest(const EvolutionMethod& emethod) : Mutation(emethod) 
		{ 
			//Get archive
			if(evolution_method().get_context_data().get_type() == static_variant_type<Population>())
			{
				m_archive = evolution_method().get_context_data().get_ptr<IndividualList>();
			}
			//Get percentage of best individuals
			m_perc_of_best = parameters().m_perc_of_best;
		}

		virtual void operator()		
		(
			size_t id_target,
			Individual& output
		) 
		override
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
			auto& w_target = parent(id_target).array();
			auto& w_best   = parent(id_best).array();
			auto& x_a 	   = parent(id_a).array();
			//b from archive (JADE/SHADE)
			if(m_archive)
			{
				size_t rand_b = random(id_target).index_rand(m_archive->size() + population().size() - 2);
				auto x_b = rand_b < m_archive->size()
						  ? archive(rand_b).array() 
						  : parent(rand_deck.get_random_id(id_target)).array();
				output.array() = ( w_target + ((w_best - w_target) + (x_a - x_b)) * output.f() ).unaryExpr(solver().clamp_function());
			}
			else 
			{
				auto x_b = parent(rand_deck.get_random_id(id_best)).array();
				output.array() = ( w_target + ((w_best - w_target) + (x_a - x_b)) * output.f() ).unaryExpr(solver().clamp_function());
			}
	
		}

	protected:

		Scalar m_perc_of_best;
		const Individual& archive(size_t i) const { return *(*m_archive)[i]; } 
		const IndividualList* m_archive{ nullptr };

	};
	REGISTERED_MUTATION(CurrentToPBest, "curr_p_best")
}
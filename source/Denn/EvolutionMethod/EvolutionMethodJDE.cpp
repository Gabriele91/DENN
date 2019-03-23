#include "Denn/Parameters.h"
#include "Denn/Denn/Individual.h"
#include "Denn/Denn/Population.h"
#include "Denn/Denn/Solver.h"
#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Crossover.h"

namespace Denn
{
	class JDEMethod : public EvolutionMethod
	{
	public:

		JDEMethod(const Solver& solver, SubPopulation& sub_pop) : EvolutionMethod(solver, sub_pop) {}
	
		virtual void start() override
		{
			//create mutation/crossover
			m_mutation = MutationFactory::create(parameters().m_mutation_type, *this);
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, *this);
		}
		
		virtual void start_a_subgen_pass() override
		{
			//sort parents			
			if (m_mutation->required_sort())  population_sort();
		}

		virtual void create_a_individual
		(     
			  size_t id_target
			, Individual& output
		)
		override
		{
			//vectors
			Individual& target = *population()[id_target].parent();
			//f JDE
			if (random(id_target).uniform() < Scalar(parameters().m_jde_f))
				output.f() = Scalar(random(id_target).uniform(0.0, 2.0));
			else
				output.f() = target.f();
			//cr JDE
			if (random(id_target).uniform() < Scalar(parameters().m_jde_cr))
				output.cr() = Scalar(random(id_target).uniform());
			else
				output.cr() = target.cr();
			//update p
			output.p()  = target.p();
			//call muation + crossover
			(*m_mutation) (id_target, output);
			(*m_crossover)(id_target, output);		
			//no 0
			output.no_0_weights();	
			//eval
			output.eval() = solver().loss_function_eval(output, id_target);		
		}

		virtual	void selection() override
		{
			population().swap_best();
		}

	protected:

		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(JDEMethod, "JDE")
}
#include "Denn/Parameters.h"
#include "Denn/Denn/Individual.h"
#include "Denn/Denn/Population.h"
#include "Denn/Denn/Solver.h"
#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/Crossover.h"
namespace Denn
{
	class JADEMethod : public EvolutionMethod
	{
	public:

		JADEMethod(const Solver& solver, SubPopulation& sub_pop) : EvolutionMethod(solver, sub_pop)
		{
			m_archive_max_size = parameters().m_archive_size;
			m_c_adapt          = parameters().m_f_cr_adapt;
			m_mu_f       = Scalar(0.5);
			m_mu_cr      = Scalar(0.5);
		}
		
		virtual void start() override
		{
			//reinit
			m_mu_f = Scalar(0.5);
			m_mu_cr = Scalar(0.5);
			//clear
			m_archive.clear();
			//create mutation/crossover
			m_mutation = MutationFactory::create(parameters().m_mutation_type, *this);
			m_crossover = CrossoverFactory::create(parameters().m_crossover_type, *this);
		}
		
		void start_a_subgen_pass() override
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
			//Compute F
			//JADE REF:  Cauchy  distribution  with  location  parameter μF and scale parameter 0.1
			//           Fi=randci(μF,0.1) and  then  truncated  to  be  1  if Fi≥1  or  regenerated  if Fi ≤ 0
			Scalar v;
			do v = random(id_target).cauchy(m_mu_f, 0.1); while (v <= 0);
			//update meta parameters
			output.f()  = Denn::sature(v);
			output.cr() = Denn::sature(random(id_target).normal(m_mu_cr, 0.1));
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
			Scalar sum_f = 0;
			Scalar sum_f2 = 0;
			Scalar sum_cr = 0;
			size_t n_discarded = 0;
			size_t np = current_np();

			for (size_t i = 0; i != np; ++i)
			{
				auto father = population()[i].parent();
				auto son = population()[i].son();
				if (loss_function_compare(son->eval(),father->eval()))
				{
					if (m_archive_max_size) m_archive.push_back(father->copy());
					//else if (random().uniform() < Scalar(m_archive_max_size) / Scalar(m_archive_max_size + n_discarded))
					sum_f += son->f();
					sum_f2 += son->f() * son->f();
					sum_cr += son->cr();
					++n_discarded;
					population().swap(i);
				}
			}
			//safe compute muF and muCR 
			if (n_discarded)
			{
				m_mu_cr = Denn::lerp(m_mu_cr, sum_cr / n_discarded, m_c_adapt);
				m_mu_f = Denn::lerp(m_mu_f, sum_f2 / sum_f, m_c_adapt);
			}
			//reduce A
			while (m_archive_max_size < m_archive.size())
			{
				m_archive[random().index_rand(m_archive.size())] = m_archive.back();
				m_archive.pop_back();
			}
		}

		virtual const VariantRef get_context_data() const override
		{
			return VariantRef(m_archive);
		}

	protected:		

		size_t          m_archive_max_size{ false };
		Scalar          m_c_adapt         { Scalar(1.0) };
		Scalar          m_mu_f      { Scalar(0.5) };
		Scalar          m_mu_cr     { Scalar(0.5) };
		IndividualList	m_archive;
		Mutation::SPtr  m_mutation;
		Crossover::SPtr m_crossover;

	};
	REGISTERED_EVOLUTION_METHOD(JADEMethod, "JADE")
}
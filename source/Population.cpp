#include "Denn/Config.h"
#include "Denn/Population.h"
namespace Denn
{
	////////////////////////////////////////////////////////////////////////
	//Population
	//vector methods 		
	size_t Population::size() const                         { return m_individuals.size(); }
	void   Population::resize(size_t i)                     { m_individuals.resize(i);     }
	void   Population::push_back(const Individual::SPtr& i) { m_individuals.push_back(i);  }
	void   Population::pop_back()                           { m_individuals.pop_back();    }
	void   Population::clear()                              { m_individuals.clear();	   }		

	//vector operator
	Individual::SPtr&       Population::first() { return m_individuals[0];        }
	Individual::SPtr&       Population::last()  { return m_individuals[size()-1]; }
	Individual::SPtr&       Population::operator[](size_t i) { return m_individuals[i]; }
	const Individual::SPtr& Population::operator[](size_t i) const { return m_individuals[i]; }

	//iterator
	typename std::vector < Individual::SPtr >::iterator       Population::begin()       { return m_individuals.begin(); }
	typename std::vector < Individual::SPtr >::const_iterator Population::begin() const { return m_individuals.begin(); }
	typename std::vector < Individual::SPtr >::iterator       Population::end()         { return m_individuals.end(); }
	typename std::vector < Individual::SPtr >::const_iterator Population::end() const   { return m_individuals.end(); }

	//costum
	void Population::best(size_t& out_i, Scalar& out_eval) const
	{
		//best
		size_t	   best_i;
		Scalar best_eval;
		//find best
		for (size_t i = 0; i != m_individuals.size(); ++i)
		{
			//
			if 
			(!i 
			 || ( m_minimize_loss_function && m_individuals[i]->m_eval < best_eval)
			 || (!m_minimize_loss_function && m_individuals[i]->m_eval > best_eval)
			)
			{
				best_i = i;
				best_eval = m_individuals[i]->m_eval;
			}
		}
		out_i = best_i;
		out_eval = best_eval;
	}
	Individual::SPtr Population::best() const
	{
		//values
		size_t best_i;
		Scalar best_eval;
		//get best id
		best(best_i, best_eval);
		//return
		return m_individuals[best_i];
	}

	static bool compare_individual(const Individual::SPtr& li, const Individual::SPtr& ri)
	{
		return li->m_eval < ri->m_eval;
	}

	void Population::sort()
	{
		std::sort(m_individuals.begin(), m_individuals.end(), compare_individual);
	}

	Population Population::copy() const
	{
		//malloc 
		Population new_pop;
		new_pop.resize(size());
		//init
		for(size_t i = 0;i != size(); ++i)
		{
			new_pop[i] = m_individuals[i]->copy();
		}
		//return
		return new_pop;
	}

	std::vector < Individual::SPtr >& Population::as_vector()
	{
		return m_individuals;
	}

	const std::vector < Individual::SPtr >& Population::as_vector() const
	{
		return m_individuals;
	}
	////////////////////////////////////////////////////////////////////////
	//init population
	void DoubleBufferPopulation::init(
		  size_t np
		, const Individual::SPtr& i_default
		, const DataSet& dataset
		, const RandomFunction random_func
		, Evaluation& loss_function
		, ThreadPool* thread_pool
		, RandomFunctionThread thread_random
	)
	{
		//minimize?
		m_minimize_loss_function = loss_function.minimize();
		parents().m_minimize_loss_function = m_minimize_loss_function;
		sons().m_minimize_loss_function = m_minimize_loss_function;
		//init
		if(thread_pool && thread_random)
		{
			//alloc promises
			PromiseList promises;
			//eval
			Population& p_ref = parents();
			Population& s_ref = sons();
			//size
			p_ref.resize(np);
			s_ref.resize(np);
			//parallel
			for (size_t i = 0; i != np; ++i)
			{
				promises.push_back(thread_pool->push_task([&,i]()
				{
					//copy layout
					p_ref[i] = i_default->copy();
					s_ref[i] = i_default->copy();
					//build fun
					auto rand_weight = [=](Scalar weight) -> Scalar
					{ 
						const Scalar eps = SCALAR_EPS;
						while(std::abs(weight) <= eps) weight = thread_random(weight, i);
						return weight;
					};
					//init
					for (Layer::SPtr layer : p_ref[i]->m_network)
					for (AlignedMapMatrix matrix : *layer)
					{
						matrix = matrix.unaryExpr(rand_weight);
					}
					//eval
					p_ref[i]->m_eval = loss_function((NeuralNetwork&)*p_ref[i], dataset);
				}));
			}
			//wait
			for (auto& p : promises) p.wait();
		}
		else 
		{
			//eval
			Population& p_ref = parents();
			Population& s_ref = sons();
			//size
			p_ref.resize(np);
			s_ref.resize(np);
			//parallel
			for (size_t i = 0; i != np; ++i)
			{
				//copy layout
				p_ref[i] = i_default->copy();
				s_ref[i] = i_default->copy();
				//build
				auto rand_weight = [=](Scalar weight) -> Scalar
				{ 
					const Scalar eps = SCALAR_EPS;
					while(std::abs(weight) <= eps) weight = random_func(weight);
					return weight;
				};
				//init
				for (Layer::SPtr layer : p_ref[i]->m_network)
				for (AlignedMapMatrix matrix : *layer)	
				{
					matrix = matrix.unaryExpr(rand_weight);
				}
				//eval
				p_ref[i]->m_eval = loss_function((NeuralNetwork&)*p_ref[i], dataset);
			}
		}
	}
	//size
	size_t DoubleBufferPopulation::size() const
	{
		return m_pop_buffer[0].size();
	}
	void DoubleBufferPopulation::resize(size_t new_np)
	{
		for (Population& population : m_pop_buffer)
		{
			//new size
			population.resize(new_np);
		}
	}
	//current
	Population& DoubleBufferPopulation::parents()
	{
		return m_pop_buffer[size_t(PopulationType::PT_PARENTS)];
	}
	const Population& DoubleBufferPopulation::parents() const
	{
		return m_pop_buffer[size_t(PopulationType::PT_PARENTS)];
	}
	const Population& DoubleBufferPopulation::const_parents() const
	{
		return m_pop_buffer[size_t(PopulationType::PT_PARENTS)];
	}
	//next
	Population& DoubleBufferPopulation::sons()
	{
		return m_pop_buffer[size_t(PopulationType::PT_SONS)];
	}
	const Population& DoubleBufferPopulation::sons() const
	{
		return m_pop_buffer[size_t(PopulationType::PT_SONS)];
	}
	const Population& DoubleBufferPopulation::const_sons() const
	{
		return m_pop_buffer[size_t(PopulationType::PT_SONS)];
	}
	//get best
	void DoubleBufferPopulation::best(size_t& best_i, Scalar& out_eval) const
	{
		parents().best(best_i, out_eval);
	}
	Individual::SPtr DoubleBufferPopulation::best() const
	{
		return parents().best();
	}
	//swap
	void DoubleBufferPopulation::the_best_sons_become_parents()
	{
		//minimize
		if(m_minimize_loss_function)
		{
			for (size_t i = 0; i != parents().size(); ++i)
			{
				if (sons()[i]->m_eval < parents()[i]->m_eval)
				{
					auto individual_tmp = parents()[i];
					parents()[i] = sons()[i];
					sons()[i] = individual_tmp;
				}
			}
		}
		else 
		{
			for (size_t i = 0; i != parents().size(); ++i)
			{
				if (sons()[i]->m_eval > parents()[i]->m_eval)
				{
					auto individual_tmp = parents()[i];
					parents()[i] = sons()[i];
					sons()[i] = individual_tmp;
				}
			}
		}
	}
	void DoubleBufferPopulation::swap(size_t i)
	{
		auto individual_tmp = parents()[i];
		parents()[i] = sons()[i];
		sons()[i] = individual_tmp;
	}

	void DoubleBufferPopulation::swap_all()
	{
		for (size_t i = 0; i != parents().size(); ++i)
			swap(i);
	}

}
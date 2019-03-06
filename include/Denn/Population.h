#pragma once
#include "Config.h"
#include "DataSet.h"
#include "NeuralNetwork.h"
#include "Individual.h"
#include "Evaluation.h"

namespace Denn
{
	////////////////////////////////////////////////////////////////////////
	class Population;
	class DoubleBufferPopulation;
    ////////////////////////////////////////////////////////////////////////
	//Population
	class Population
	{
	public:
		//vector methods 		
		size_t size() const;
		void   resize(size_t i);
		void   push_back(const Individual::SPtr& i);
		void   pop_back();
		void   clear();

		//vector operator
		Individual::SPtr& first();
		Individual::SPtr& last();
		Individual::SPtr& operator[](size_t i);
		const Individual::SPtr& operator[](size_t i) const;
		
		//iterator
		typename std::vector < Individual::SPtr >::iterator       begin();
		typename std::vector < Individual::SPtr >::const_iterator begin() const;
		typename std::vector < Individual::SPtr >::iterator       end();
		typename std::vector < Individual::SPtr >::const_iterator end() const;

		//costum
		void best(size_t& out_i, Scalar& out_eval) const;
		Individual::SPtr best() const;
		
		//sort
		void sort();

		//copy
		Population copy() const;
	
		//as vector
		std::vector < Individual::SPtr >& as_vector();
		const std::vector < Individual::SPtr >& as_vector() const;

	protected:

		std::vector < Individual::SPtr > m_individuals;
		bool m_minimize_loss_function { true };
		friend class DoubleBufferPopulation;

	};
    ////////////////////////////////////////////////////////////////////////
	class IndividualSlider 
	{
		Individual& m_individual;
		const size_t m_layer_index;
		const size_t m_matrix_index;

	public:	

		IndividualSlider(Individual& individual, size_t layer_index, size_t matrix_index)
		: m_individual(individual)
		, m_layer_index(layer_index)
		, m_matrix_index(matrix_index)
		{}

		virtual Scalar f() const 
		{
			return m_individual.m_f;
		}

		virtual Scalar cr() const 
		{
			return m_individual.m_cr;
		}

		virtual Scalar p() const
		{
			return m_individual.m_p;
		}

		virtual Scalar eval() const
		{
			return m_individual.m_eval;
		}
	
		virtual AlignedMapMatrix operator * ()
		{
			return m_individual[m_layer_index][m_matrix_index];
		}

		virtual ConstAlignedMapMatrix operator * () const
		{
			const Individual& individual = m_individual;
			return individual[m_layer_index][m_matrix_index];
		}

		IndividualSlider& operator = (AlignedMapMatrix value)
		{
			m_individual[m_layer_index][m_matrix_index] = value;
			return *this;
		}


	};

	class PopulationSlider
	{
		Population& m_population;
		const size_t m_layer_index;
		const size_t m_matrix_index;

	public:

		PopulationSlider(Population& population, size_t layer_index, size_t matrix_index)
		: m_population(population)
		, m_layer_index(layer_index)
		, m_matrix_index(matrix_index)
		{}

		size_t size() const
		{
			return m_population.size();
		}

		virtual Scalar f(size_t i) const
		{
			return (*m_population[i]).m_f;
		}

		virtual Scalar cr(size_t i) const 
		{
			return (*m_population[i]).m_cr;
		}

		virtual Scalar p(size_t i) const 
		{
			return (*m_population[i]).m_p;
		}


		virtual Scalar eval(size_t i) const 
		{
			return (*m_population[i]).m_eval;
		}

		virtual size_t best_id() const
		{
			Scalar scalar;
			size_t id;
			m_population.best(id, scalar);	
			return id;
		}

		virtual IndividualSlider get_slider(Individual& individual) const
		{
			return IndividualSlider(individual, m_layer_index, m_matrix_index);
		}

		virtual ConstAlignedMapMatrix apply_slider(const Individual& individual) const
		{
			return individual[m_layer_index][m_matrix_index];
		}


		virtual IndividualSlider individual(size_t i) const
		{
			return IndividualSlider((*m_population[i]), m_layer_index, m_matrix_index);
		}

		virtual IndividualSlider best()
		{
			return IndividualSlider((*m_population.best()), m_layer_index, m_matrix_index);
		}

		virtual const IndividualSlider best() const
		{
			return IndividualSlider((*m_population.best()), m_layer_index, m_matrix_index);
		}

		virtual AlignedMapMatrix operator[](size_t i)
		{
			return (*m_population[i])[m_layer_index][m_matrix_index];
		}

		virtual ConstAlignedMapMatrix operator[](size_t i) const
		{
			const Individual& individual = (*m_population[i]);
			return individual[m_layer_index][m_matrix_index];
		}
	};
	////////////////////////////////////////////////////////////////////////
	enum class PopulationType : size_t
	{
		PT_PARENTS=0,
		PT_SONS,
		PT_SIZE
	};
	//Double Population buffer
	class DoubleBufferPopulation
	{
	public:
		//Pointer
		using RandomFunction = std::function<Scalar(Scalar)>;
		using RandomFunctionThread = std::function<Scalar(Scalar,size_t)>;
        //attributes
		Population m_pop_buffer[ size_t(PopulationType::PT_SIZE) ];
		bool m_minimize_loss_function { true };
		//init population
		void init(
			  size_t np
			, const Individual::SPtr& i_default
			, const DataSet&          dataset
			, const RandomFunction    random_func
			, Evaluation&		      loss_function
			, ThreadPool*			  thread_pool = nullptr
			, RandomFunctionThread    thread_random = nullptr
		);
		//size
		size_t size() const;
		//resize all pops
		void resize(size_t new_np);
		//get
		Population&       operator[] (size_t i)      { return m_pop_buffer[i]; }
		const Population& operator[] (size_t i) const{ return m_pop_buffer[i]; }
		//current
		Population& parents();
		const Population& parents() const;
		const Population& const_parents() const;
		//next
		Population& sons();
		const Population& sons() const;
		const Population& const_sons() const;
		//get best
		void best(size_t& best_i, Scalar& out_eval) const;
		Individual::SPtr best() const;
		//swap
		void the_best_sons_become_parents();
		void swap(size_t i);
		void swap_all();
	};
}
#pragma once
#include "Denn/Config.h"
#include "Individual.h"

namespace Denn
{
	//Random engine
	class Random;
	class NeuralNetwork;
	//Population
	class SubPopulation : public std::enable_shared_from_this< SubPopulation >
	{
	public:
		//alias
		using Scalar     	  = Denn::Scalar;
		using SPtr       	  = std::shared_ptr< SubPopulation >;
		//Pair
		struct Pair
		{
			using Iterator = std::array<Individual::SPtr,2>::iterator;
			using ConstIterator = std::array<Individual::SPtr,2>::const_iterator;

			Pair(Individual::SPtr parent, Individual::SPtr son)
			{
				m_pair[0] = parent;
				m_pair[1] = son;
			}
			

			Individual::SPtr& parent(){ return m_pair[0]; };
			Individual::SPtr& son()   { return m_pair[1]; };

			Individual::SPtr parent() const { return m_pair[0]; };
			Individual::SPtr son()    const { return m_pair[1]; };

			size_t size() const { return 2; }
			Individual::SPtr& operator [](size_t i){ return m_pair[i]; }

			Iterator begin() { return m_pair.begin(); }
			Iterator end()   { return m_pair.end(); }
			ConstIterator begin() const { return m_pair.begin(); }
			ConstIterator end()   const { return m_pair.end(); }

		protected:	

			std::array<Individual::SPtr,2> m_pair;

		};
		//iterator
		class Iterator
		{
		public:

			Iterator(const Iterator& it);
			Iterator(SubPopulation& sub_population, size_t index);
			Iterator(const SubPopulation& sub_population, size_t index);

			Iterator& operator++();
			Iterator operator++(int);

			bool operator==(const Iterator& rhs) const;
			bool operator!=(const Iterator& rhs) const;

			Pair operator*();
			const Pair operator*() const;

		protected:

			friend class Denn::Layer;
			SubPopulation* m_sub_population;
			size_t m_index;
		};

		//pop init
		SubPopulation
		(
			size_t np,
			//inf network
			const IndividualMap& map,
			const NeuralNetwork& nnet
		);

		SubPopulation
		(
			//np & attributes
			size_t np,
			const Attributes& attrs, 
			//inf network
			const IndividualMap& map,
			const NeuralNetwork& nnet
		);

		//return ptr
		SPtr get_ptr();

		//shared copy
		SPtr copy() const;

		//vector methods 		
		size_t size() const;
		void   resize(size_t i);
		void   push_back(const Pair&);
		void   pop_back();
		void   clear();

		//info
		const IndividualMap& map() const;

		//vector operator
		IndividualList& parents();
		IndividualList& sons();

		const IndividualList& parents() const;
		const IndividualList& sons() const;

		Pair operator[](size_t i);
		const Pair operator[](size_t i) const;
		
		//iterator
		Iterator begin();
		Iterator end();
		const Iterator begin() const;
		const Iterator end() const;

		//easy access to last element
		Iterator last();
		const Iterator last() const;

		//costum
		size_t best_parent_id(bool minimize = true) const;
		size_t best_son_id(bool minimize = true) const;

		//roulette wheel selection
		size_t roulette_wheel_selection_parent_id(Random& random,bool minimize = true) const;
		size_t roulette_wheel_selection_son_id(Random& random,bool minimize = true) const;

		//pbest
		size_t pbest_parent_id(Random& random,bool minimize = true, Scalar prob=0.1);
		size_t pbest_son_id(Random& random,bool minimize = true, Scalar prob=0.1);

		//sort
		void sort(bool minimize = true);
		void sort_sons(bool minimize = true);

		//swap
        void swap(size_t ind_id);
        void swap(const std::vector<int>& swap_list);
        void swap_best(bool minimize = true);
        void swap_best_no_double(bool minimize = true);
        void swap_crowding(bool minimize = true);

		//swap list
		void swap_best_list(std::vector<int>& list, bool minimize = true) const;
		void swap_best_no_double_list(std::vector<int>& list, bool minimize = true) const;
		void swap_crowding_list(std::vector<int>& list, bool minimize = true) const;
		std::vector<int> swap_best_list(bool minimize = true) const;
		std::vector<int> swap_best_no_double_list(bool minimize = true) const;
		std::vector<int> swap_crowding_list(bool minimize = true) const;

		//stats computation
		Individual::SPtr avg()			 const;
		Individual::SPtr variance()		 const;
		double 			 distance_avg()  const;
		
	protected:
		
		IndividualMap  m_map;
		IndividualList m_parents;
		IndividualList m_sons;

		//Variant
		friend class Variant;
		SubPopulation(){}

	};
	//alias vector of SubPopulations
	using SubPopulationList = std::vector< SubPopulation::SPtr >;


}
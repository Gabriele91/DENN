#include "Denn/Denn/SubPopulation.h"
#include "Denn/NeuralNetwork/NeuralNetwork.h"
namespace Denn
{
	///////////////////////////////////////////////////////////////////////////
	SubPopulation::Iterator::Iterator(const Iterator& it)
	: m_sub_population(it.m_sub_population)
	, m_index(it.m_index) 
	{}
	SubPopulation::Iterator::Iterator(SubPopulation& sub_population, size_t index)
	: m_sub_population((SubPopulation*)&m_sub_population)
	, m_index(index) 
	{}
	SubPopulation::Iterator::Iterator(const SubPopulation& sub_population, size_t index)
	: m_sub_population((SubPopulation*)&m_sub_population)
	, m_index(index) 
	{}

	SubPopulation::Iterator& SubPopulation::Iterator::operator++()    
	{ 
		++m_index; 
		return *this; 
	}
	SubPopulation::Iterator  SubPopulation::Iterator::operator++(int) 
	{ 
		Iterator it(*this); 
		operator++(); 
		return it; 
	}

	bool SubPopulation::Iterator::operator==(const SubPopulation::Iterator& rhs) const 
	{ 
		return m_index == rhs.m_index && m_sub_population == rhs.m_sub_population;
	}
	bool SubPopulation::Iterator::operator!=(const SubPopulation::Iterator& rhs) const 
	{
		 return m_index != rhs.m_index || m_sub_population != rhs.m_sub_population;
	}

	SubPopulation::Pair SubPopulation::Iterator::operator*() 
	{
		 return (*m_sub_population)[m_index]; 
	}
	const SubPopulation::Pair  SubPopulation::Iterator::operator*() const 
	{
		 return (*((const SubPopulation*)m_sub_population))[m_index];
	}
	////////////////////////////////////////////////////////////////////////
	//Population
	SubPopulation::SubPopulation
	(
		size_t np,
		const IndividualMap& map,
		const NeuralNetwork& nnet
	)
	: m_map(map)
	{ 
		for(size_t n = 0; n != np; ++n)
		{
			m_parents.push_back(std::make_shared<Individual>(this, nnet));
			m_sons.push_back(std::make_shared<Individual>(this, nnet));
		}
	}

	SubPopulation::SubPopulation
	(
		size_t np,
		const Attributes& attrs, 
		//inf network
		const IndividualMap& map,
		const NeuralNetwork& nnet
	)
	: m_map(map)
	{ 
		for(size_t n = 0; n != np; ++n)
		{
			m_parents.push_back(std::make_shared<Individual>(this, attrs, nnet));
			m_sons.push_back(std::make_shared<Individual>(this, attrs, nnet));
		}
	}
	//vector methods 		
	size_t SubPopulation::size() const                         
	{ 
		return m_parents.size();
	}
	void   SubPopulation::resize(size_t i)                     
	{ 
		m_parents.resize(i); 
		m_sons.resize(i);
	}
	void   SubPopulation::push_back(const Pair& cpair) 
	{
		m_parents.push_back(cpair.parent()); 
		m_sons.push_back(cpair.son());
	}
	void   SubPopulation::pop_back() 
	{
		m_parents.pop_back();
		m_sons.pop_back();
	}
	void   SubPopulation::clear()    
	{ 
		m_parents.clear();	  
		m_sons.clear();	  
	}		

	const IndividualMap& SubPopulation::map()  const 
	{ 
		return m_map; 
	}
	
	//vector operator
	IndividualList& SubPopulation::parents()
	{
		return m_parents;
	}
	IndividualList& SubPopulation::sons()
	{
		return m_sons;
	}

	const IndividualList& SubPopulation::parents() const
	{
		return m_parents;
	}
	const IndividualList& SubPopulation::sons() const
	{
		return m_sons;
	}

	//vector operator
	SubPopulation::Pair SubPopulation::operator[](size_t i) 
	{ 
		return Pair(m_parents[i], m_sons[i]); 
	}
	const SubPopulation::Pair SubPopulation::operator[](size_t i) const 
	{ 
		return Pair(m_parents[i], m_sons[i]);
	}

	//iterator
	SubPopulation::Iterator SubPopulation::begin() { return Iterator(*this,0); }
	SubPopulation::Iterator SubPopulation::end()   { return Iterator(*this,size()); }
	const SubPopulation::Iterator SubPopulation::begin() const { return Iterator(*this,0); }
	const SubPopulation::Iterator SubPopulation::end()   const { return Iterator(*this,size()); }
	
	SubPopulation::Iterator SubPopulation::last(){ return Iterator(*this,size()-1); }
	const SubPopulation::Iterator SubPopulation::last() const{ return Iterator(*this,size()-1); }
	//costum
	size_t SubPopulation::best_parent_id(bool minimize) const
	{
		//best
		size_t best_i;
		Scalar best_eval;
		//find best
		for (size_t i = 0; i != size(); ++i)
		{
			//
			if(!i || ( minimize && m_parents[i]->eval() < best_eval)
			      || (!minimize && m_parents[i]->eval() > best_eval))
			{
				best_i = i;
				best_eval = m_parents[i]->eval();
			}
		}
		return best_i;
	}	
	//costum
	size_t SubPopulation::best_son_id(bool minimize) const
	{
		//best
		size_t best_i;
		Scalar best_eval;
		//find best
		for (size_t i = 0; i != size(); ++i)
		{
			//
			if(!i || ( minimize && m_sons[i]->eval() < best_eval)
			      || (!minimize && m_sons[i]->eval() > best_eval))
			{
				best_i = i;
				best_eval = m_sons[i]->eval();
			}
		}
		return best_i;
	}

	//roulette wheel selection
	size_t SubPopulation::roulette_wheel_selection_parent_id(Random& random,bool minimize) const
	{
		//case minimize
		if(minimize)
		{
			//max of fitness
			Scalar max = m_parents[0]->eval();		
			for (size_t i = 1; i < size(); ++i)
				max  = std::max(max, m_parents[i]->eval());
			//sum all fitness
			Scalar sum = 0;		
			for (size_t i = 0; i < size(); ++i)
				sum += max-m_parents[i]->eval();
			//select
			Scalar value = random.uniform(sum);	
			// locate the random value based on the weights
			for (size_t i = 0; i != size(); ++i)
			{		
				value -= max-m_parents[i]->eval();		
				if(value < 0) return i;
			}
		}
		else 
		{
			//sum all fitness
			Scalar sum = 0;		
			for (size_t i = 0; i != size(); ++i) sum += m_parents[i]->eval();
			//select
			Scalar value = random.uniform(sum);	
			// locate the random value based on the weights
			for (size_t i = 0; i != size(); ++i)
			{		
				value -= m_parents[i]->eval();		
				if(value < 0) return i;
			}
		}
		return size()-1;
	}
	//roulette wheel selection
	size_t SubPopulation::roulette_wheel_selection_son_id(Random& random,bool minimize) const
	{
		//case minimize
		if(minimize)
		{
			//max of fitness
			Scalar max = m_sons[0]->eval();		
			for (size_t i = 1; i < size(); ++i)
				max  = std::max(max, m_sons[i]->eval());
			//sum all fitness
			Scalar sum = 0;		
			for (size_t i = 0; i < size(); ++i)
				sum += max-m_sons[i]->eval();
			//select
			Scalar value = random.uniform(sum);	
			// locate the random value based on the weights
			for (size_t i = 0; i != size(); ++i)
			{		
				value += max-m_sons[i]->eval();		
				if(value > 0) return i;
			}
		}
		else 
		{
			//sum all fitness
			Scalar sum = 0;		
			for (size_t i = 0; i != size(); ++i) sum += m_sons[i]->eval();
			//select
			Scalar value = random.uniform(sum);	
			// locate the random value based on the weights
			for (size_t i = 0; i != size(); ++i)
			{		
				value -= m_sons[i]->eval();		
				if(value < 0) return i;
			}
		}
		return size()-1;
	}

	//pbest
	size_t SubPopulation::pbest_parent_id(Random& random,bool minimize, Scalar prob)
	{
		sort(minimize);
		return size_t(random.uniform(size() * prob));
	}
	size_t SubPopulation::pbest_son_id(Random& random,bool minimize, Scalar prob)
	{
		sort(minimize);
		return size_t(random.uniform(size() * prob));
	}

	static bool sort_minimize(const Individual::SPtr& li, const Individual::SPtr& ri){ return li->eval() < ri->eval(); }
	static bool sort_maximize(const Individual::SPtr& li, const Individual::SPtr& ri){ return li->eval() > ri->eval(); }
	void SubPopulation::sort(bool minimize)
	{
		if(minimize)
			std::sort(m_parents.begin(), m_parents.end(), sort_minimize);
		else
			std::sort(m_parents.begin(), m_parents.end(), sort_maximize);
	}
	void SubPopulation::sort_sons(bool minimize)
	{
		if(minimize)
			std::sort(m_sons.begin(), m_sons.end(), sort_minimize);
		else
			std::sort(m_sons.begin(), m_sons.end(), sort_maximize);
	}
	//swap
	void SubPopulation::swap(size_t ind_id)
	{
		std::swap(m_parents[ind_id], m_sons[ind_id]);
	}
	void SubPopulation::swap_best(bool minimize)
	{
		for (size_t i = 0; i != size(); ++i)
		{
			if(( minimize && m_sons[i]->eval() < m_parents[i]->eval())
			|| (!minimize && m_sons[i]->eval() > m_parents[i]->eval()))
			{
				swap(i);
			}
		}
	}

	SubPopulation::SPtr SubPopulation::copy() const
	{
		//malloc 
		SubPopulation::SPtr new_pop{new SubPopulation()};
		//set layer id/
		new_pop->m_map = map();
		//init
		for(size_t i = 0;i != size(); ++i)
		{ 
			new_pop->parents().push_back(parents()[i]->copy(new_pop.get()));
		    new_pop->sons().push_back(sons()[i]->copy(new_pop.get()));
		}
		//return
		return new_pop;
	}


}
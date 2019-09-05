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
	
    void SubPopulation::swap(const std::vector<int>& swap_list)
	{
		for (size_t i = 0; i != size(); ++i)
			if(swap_list[i] >= 0)
				swap(swap_list[i]);
	}

	void SubPopulation::swap_best(bool minimize)
	{
		for (size_t i = 0; i != size(); ++i)
		{
			if(( minimize && m_sons[i]->eval() <= m_parents[i]->eval())
			|| (!minimize && m_sons[i]->eval() >= m_parents[i]->eval()))
			{
				swap(i);
			}
		}
	}

	void SubPopulation::swap_best_no_double(bool minimize)
	{
		std::vector<int> swap_list; 
		swap_list.reserve(size());
		swap_best_no_double_list(swap_list, minimize);
		swap(swap_list);
	}

	void SubPopulation::swap_crowding(bool minimize)
	{
		std::vector<int> swap_list; 
		swap_list.reserve(size());
		swap_crowding_list(swap_list, minimize);
		swap(swap_list);
	}

	
	//swap list
	void SubPopulation::swap_best_list(std::vector<int>& swap_list,bool minimize) const
	{
		//init all -1
		swap_list.resize(size());
		std::fill(swap_list.begin(),swap_list.end(),-1);
		//swap
		for (size_t i = 0; i != size(); ++i)
		{
			if(( minimize && m_sons[i]->eval() <= m_parents[i]->eval())
			|| (!minimize && m_sons[i]->eval() >= m_parents[i]->eval()))
			{
				swap_list[i] = int(i);
			}
		}
	}

	void SubPopulation::swap_best_no_double_list(std::vector<int>& swap_list,bool minimize) const
	{
		//list of swap
		swap_best_list(swap_list);
		//factor
		Scalar factor = std::pow( Scalar(0.1), m_sons[0]->array().size() );
		//disable double swap
		for (size_t i = 0; i != size(); ++i)
		{
			if(swap_list[i] > -1)
			{
				for (size_t j = 0; j != size(); ++j)
				{
					if(i != j && distance(*m_sons[i], *m_parents[j]) < factor) 
					{
						swap_list[i] = -1;
						break;
					}
				}
			}
		}
	}

	void SubPopulation::swap_crowding_list(std::vector<int>& swap_list, bool minimize) const
	{
		//init all -1
		swap_list.resize(size());
		std::fill(swap_list.begin(),swap_list.end(),-1);
		//swap
		for (size_t i = 1; i < size(); ++i)
		{
			//search target
			int target_i = 0;
			Scalar terget_dist = distance(*m_sons[i], *m_parents[0]);
			for (size_t j = 1; j < size(); ++j)
			{
				Scalar j_dist = distance(*m_sons[i], *m_parents[j]);
				if (j_dist < terget_dist) 
				{
					target_i = j;
					terget_dist = j_dist;
				}
			}
			//target eval default = the closest
			Scalar parent_eval = m_parents[target_i]->eval(); 
			//else the last swapped
			if(swap_list[target_i] >= 0)
				parent_eval = m_sons[swap_list[target_i]]->eval();
			//test
			if(( minimize && m_sons[i]->eval() <= parent_eval)
			|| (!minimize && m_sons[i]->eval() >= parent_eval))
			{
				swap_list[target_i] = i;
			} 
		}
	}

	std::vector<int> SubPopulation::swap_best_list(bool minimize) const
	{
		std::vector<int> swap_list; 
		swap_list.reserve(size());
		swap_best_list(swap_list, minimize);
		return swap_list;
	}

	std::vector<int> SubPopulation::swap_best_no_double_list(bool minimize) const
	{
		std::vector<int> swap_list; 
		swap_list.reserve(size());
		swap_best_no_double_list(swap_list, minimize);
		return swap_list;
	}

	std::vector<int> SubPopulation::swap_crowding_list(bool minimize) const
	{
		std::vector<int> swap_list; 
		swap_list.reserve(size());
		swap_crowding_list(swap_list, minimize);
		return swap_list;
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

	
	Individual::SPtr SubPopulation::avg() const
	{
		if(!size()) return nullptr;
		//copy the first element
		auto iavg = parents()[0]->copy();
		//sum all others elements
		for(size_t i = 1;i < size(); ++i)
			iavg->array() += parents()[i]->array();
		//divide for the pop size
		iavg->array() /= Scalar(size());
		//return
		return iavg;
	}

	Individual::SPtr SubPopulation::variance() const
	{
		if(!size()) return nullptr;
		//compute avg
		auto iavg = avg();
		//init output "ivar"
		auto ivar = parents()[0]->copy();
		//fill to 0
		ivar->array().fill(Scalar(0));
		//sum  (x-avg)^2
		for(size_t i = 0;i < size(); ++i)
			ivar->array() += (parents()[i]->array() - iavg->array()).square();
		//sqrt(sum((x-avg)^2))
		ivar->array() = ivar->array().sqrt();
		//return
		return ivar;
	}

	double SubPopulation::distance_avg() const
	{
		if(!size()) return Scalar(0);
		//init
		Scalar dist = 0;
		//for each element for each other
		for(size_t i = 0;i < size(); ++i)
		for(size_t j = i+1;j < size(); ++j)
		{
			dist = distance(parents()[i]->array(),parents()[j]->array());
		}
		//retrun avg
		return dist / (size()*(size()-1) / Scalar(2));
	}


}
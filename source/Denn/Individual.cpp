#include "Denn/Denn/Individual.h"
#include "Denn/Denn/SubPopulation.h"

namespace Denn
{
	//get subpopulation ref
	const SubPopulation* Individual::subpopulation() const 
	{ 
		return m_subpopulation;
	}
	const IndividualMap* Individual::map() const 
	{
		return subpopulation() ? &subpopulation()->map() : nullptr; 
	}
	//return ptr
	Individual::SPtr Individual::get_ptr() { return this->shared_from_this(); }
	//shared copy
	Individual::SPtr Individual::copy() const
	{
		return std::make_shared<Individual>(*this);
	}
	//shared copy
	Individual::SPtr Individual::copy(SubPopulation* subpop) const
	{
		auto tcopy = copy();
		tcopy->m_subpopulation = subpop;
		return tcopy;
	}

	//init
	Individual::Individual() 
	: m_subpopulation(nullptr)
	{
	}
	//from map
	Individual::Individual(SubPopulation* subpop, const NeuralNetwork& nn)
	: m_subpopulation(subpop)
	{
		//size
		size_t array_size = 0;
		for(auto pair : *map()) array_size += nn[pair.first][pair.second].size();
		//init
		m_weights.resize(array_size);
		//copy all
		copy_from(nn);
	}
	Individual::Individual(SubPopulation* subpop, Attributes attrs, const NeuralNetwork& nn)
	: m_subpopulation(subpop)
	, m_attributes(attrs)
	{
		//size
		size_t array_size = 0;
		for(auto pair : *map()) array_size += nn[pair.first][pair.second].size();
		//init
		m_weights.resize(array_size);
		//copy all
		copy_from(nn);
	}
	
	//copy attributes from a other individual
	void Individual::copy_from(const Individual& individual)
	{
		m_attributes = individual.m_attributes;
		m_weights    = individual.m_weights;
		m_eval       = individual.m_eval;
	}
	void Individual::copy_attributes(const Individual& individual)
	{
		m_attributes = individual.m_attributes;
		m_eval       = individual.m_eval;
	}
	//matrix
	AlignedMapMatrix Individual::matrix()
	{ 
		return AlignedMapMatrix(m_weights.data(), 1, m_weights.size()); 
	}
	ConstAlignedMapMatrix Individual::matrix() const
	{ 
		return ConstAlignedMapMatrix(m_weights.data(), 1, m_weights.size()); 
	}
	AlignedMapMatrix Individual::matrix(const Matrix& msize)
	{ 
		return AlignedMapMatrix(m_weights.data(),msize.rows(),msize.cols()); 
	}
	ConstAlignedMapMatrix Individual::matrix(const Matrix& msize) const
	{ 
		return ConstAlignedMapMatrix(m_weights.data(),msize.rows(),msize.cols()); 
	}

	ColArray& Individual::array() 
	{ 
		return  m_weights;
	}
	const ColArray&  Individual::array() const
	{ 
		return  m_weights;
	}


	void Individual::no_0_weights()
	{
		array() = array().unaryExpr([](Scalar weight) -> Scalar
		{ 			
			const Scalar eps = SCALAR_EPS;
			while(std::abs(weight) <= eps) weight += std::copysign(eps, weight);
			return weight;
		});
	}

	//like Network		
	Scalar& Individual::eval()
	{
		return m_eval;
	}	
	Scalar& Individual::f()
	{
		return m_attributes[size_t(Attribute::F)];
	}	
	Scalar& Individual::cr()
	{
		return m_attributes[size_t(Attribute::CR)];
	}	
	Scalar& Individual::p()
	{
		return m_attributes[size_t(Attribute::P)];
	}
	Scalar& Individual::attribute(Attribute a)
	{
		return m_attributes[size_t(a)];
	}

	Scalar Individual::eval() const
	{
		return m_eval;
	}	
	Scalar Individual::f() const 
	{
		return m_attributes[size_t(Attribute::F)];
	}	
	Scalar Individual::cr() const
	{
		return m_attributes[size_t(Attribute::CR)];
	}	
	Scalar Individual::p() const
	{
		return m_attributes[size_t(Attribute::P)];
	}
	Scalar Individual::attribute(Attribute a) const
	{
		return m_attributes[size_t(a)];
	}

	//apply to network
	void Individual::copy_to(NeuralNetwork& nn)
	{
		if(!map()) return;
		//index
		size_t offset = 0;
		//
		for(auto pair : *map())
		{
			//data
			auto data = nn[pair.first][pair.second].array();
			//data = array().block(id,0,data.size(),1).array();
			for(int i=0; i < data.size(); ++i) 
				data(i) = m_weights(offset+i);
			offset += data.size();
		}
	}

	//copy value to individual
	void Individual::copy_from(const NeuralNetwork& nn)
	{
		if(!map()) return;
		//index
		size_t offset = 0;
		//
		for(auto pair : *map())
		{
			//data
			auto data = nn[pair.first][pair.second].array();
			for(int i=0; i < data.size(); ++i) 
				m_weights(offset+i) = data(i);
			offset += data.size();
		}
	}
}
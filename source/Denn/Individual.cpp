#include "Denn/Denn/Individual.h"

namespace Denn
{
	//return ptr
	Individual::SPtr Individual::get_ptr() { return this->shared_from_this(); }
	//shared copy
	Individual::SPtr Individual::copy() const
	{
		return std::make_shared<Individual>(*this);
	}

	//init
	Individual::Individual() 
	: m_subpopulation(nullptr)
	{
	}
	Individual::Individual(SubPopulation* subpop, AlignedMapMatrix weights)
	: m_subpopulation(subpop)
	, m_weights(weights.array())
	{
	}	
	Individual::Individual(SubPopulation* subpop, ConstAlignedMapMatrix weights)
	: m_subpopulation(subpop)
	, m_weights(weights.array())
	{
	}	
	Individual::Individual(SubPopulation* subpop, Attributes attrs, AlignedMapMatrix weights)
	: m_subpopulation(subpop)
	, m_weights(weights.array())
	, m_attributes(attrs)
	{
	}
	Individual::Individual(SubPopulation* subpop, Attributes attrs, ConstAlignedMapMatrix weights)
	: m_subpopulation(subpop)
	, m_weights(weights.array())
	, m_attributes(attrs)
	{
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

}
#include "Denn/Individual.h"

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
	Individual::Individual() {}
	Individual::Individual(Scalar f, Scalar cr, Scalar p, const NeuralNetwork& network)
	{
		m_f  = f;
		m_cr = cr;
		m_p  = p;
		m_network = network;
	}
	//copy attributes from a other individual
	void Individual::copy_from(const Individual& individual)
	{
		m_f       = individual.m_f;
		m_cr      = individual.m_cr;
		m_p        = individual.m_p;
		m_eval    = individual.m_eval;
		m_network = individual.m_network;
	}
	void Individual::copy_attributes(const Individual& individual)
	{
		m_f    = individual.m_f;
		m_cr   = individual.m_cr;
		m_p    = individual.m_p;
		m_eval = individual.m_eval;
	}
	//cast
	Individual::operator NeuralNetwork&()
	{
		return m_network;
	}

	Individual::operator const NeuralNetwork& () const
	{
		return m_network;
	}
	//like Network
	Layer& Individual::operator[](size_t i)
	{
		return m_network[i];
	}
	const Layer& Individual::operator[](size_t i) const
	{
		return m_network[i];
	}
	size_t Individual::size() const
	{
		return m_network.size();
	}
}
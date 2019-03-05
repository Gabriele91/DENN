#include "Denn/Mutation.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//Mutation
	Mutation::Mutation(const DennAlgorithm& algorithm)
	: m_algorithm(algorithm)
	{
	}

	//easy access
	const Parameters& Mutation::parameters()            const { return m_algorithm.parameters();        }
	const EvolutionMethod& Mutation::evolution_method() const	{ return m_algorithm.evolution_method();  }

	const size_t Mutation::current_np()                  const   { return m_algorithm.current_np(); }
	const DoubleBufferPopulation& Mutation::population() const   { return m_algorithm.population(); }

	Random& Mutation::random(size_t i)			      const { return m_algorithm.random(i); }
	
	//help, how is the best
	bool Mutation::loss_function_compare(Scalar left, Scalar right) const       { return  m_algorithm.loss_function_compare(left,right);  }
	bool Mutation::validation_function_compare(Scalar left, Scalar right) const { return  m_algorithm.validation_function_compare(left,right);  }
	bool Mutation::test_function_compare(Scalar left, Scalar right) const       { return  m_algorithm.test_function_compare(left,right);  }

	//map
	static std::map< std::string, MutationFactory::CreateObject >& m_map()
	{
		static std::map< std::string, MutationFactory::CreateObject > m_map;
		return m_map;
	}
	//public
	Mutation::SPtr MutationFactory::create(const std::string& name, const DennAlgorithm& algorithm)
	{
		//find
		auto it = m_map().find(name);
		//return
		return it == m_map().end() ? nullptr : it->second(algorithm);
	}
	void MutationFactory::append(const std::string& name, MutationFactory::CreateObject fun, size_t size)
	{
		//add
		m_map()[name] = fun;
	}
	//list of methods
	std::vector< std::string > MutationFactory::list_of_mutations()
	{
		std::vector< std::string > list;
		for (const auto & pair : m_map()) list.push_back(pair.first);
		return list;
	}
	std::string  MutationFactory::names_of_mutations(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_mutations();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	//info
	bool MutationFactory::exists(const std::string& name)
	{
		//find
		auto it = m_map().find(name);
		//return 
		return it != m_map().end();
	}
}

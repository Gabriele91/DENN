#include "Denn/Crossover.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//EvolutionMethod
	Crossover::Crossover(const DennAlgorithm& algorithm)
	: m_algorithm(algorithm)
	{
	}
	//easy access
	const Parameters& Crossover::parameters()            const { return m_algorithm.parameters();        }
	const EvolutionMethod& Crossover::evolution_method() const	{ return m_algorithm.evolution_method();  }

	const size_t Crossover::current_np()                  const   { return m_algorithm.current_np(); }
	const DoubleBufferPopulation& Crossover::population() const   { return m_algorithm.population(); }

	Random& Crossover::population_random(size_t i)       const { return m_algorithm.population_random(i);}
	Random& Crossover::random(size_t i)			         const { return m_algorithm.random(i); }
	
	//help, how is the best
	bool Crossover::loss_function_compare(Scalar left, Scalar right) const       { return  m_algorithm.loss_function_compare(left,right);  }
	bool Crossover::validation_function_compare(Scalar left, Scalar right) const { return  m_algorithm.validation_function_compare(left,right);  }
	bool Crossover::test_function_compare(Scalar left, Scalar right) const       { return  m_algorithm.test_function_compare(left,right);  }

	#ifndef RANDOM_SAFE_CROSSOVER
	Random& Crossover::main_random()					 const { return m_algorithm.main_random(); }
	Random& Crossover::random()					         const { return m_algorithm.random(); }
	#endif
	//map
	static std::map< std::string, CrossoverFactory::CreateObject >& c_map()
	{
		static std::map< std::string, CrossoverFactory::CreateObject > c_map;
		return c_map;
	}
	//public
	Crossover::SPtr CrossoverFactory::create(const std::string& name, const DennAlgorithm& algorithm)
	{
		//find
		auto it = c_map().find(name);
		//return
		return it == c_map().end() ? nullptr : it->second(algorithm);
	}
	void CrossoverFactory::append(const std::string& name, CrossoverFactory::CreateObject fun, size_t size)
	{
		//add
		c_map()[name] = fun;
	}
	//list of methods
	std::vector< std::string > CrossoverFactory::list_of_crossovers()
	{
		std::vector< std::string > list;
		for (const auto & pair : c_map()) list.push_back(pair.first);
		return list;
	}
	std::string                CrossoverFactory::names_of_crossovers(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_crossovers();
		std::copy(list.begin(), list.end()-1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	bool CrossoverFactory::exists(const std::string& name)
	{
		//find
		auto it = c_map().find(name);
		//return 
		return it != c_map().end();
	}
}

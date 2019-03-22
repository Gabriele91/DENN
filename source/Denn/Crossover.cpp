#include "Denn/Denn/Crossover.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//EvolutionMethod
	Crossover::Crossover(const EvolutionMethod& emethod)
	: m_emethod(emethod)
	{
	}
	//easy access
	const EvolutionMethod& Crossover::evolution_method() const	{ return m_emethod;  }
	const Solver& Crossover::solver()            		 const  { return evolution_method().solver(); }
	const Parameters& Crossover::parameters()            const  { return solver().parameters(); }

	const SubPopulation& Crossover::population()  const { return evolution_method().population(); }
	const size_t Crossover::current_np()          const { return population().size(); }
	const Individual& Crossover::parent(size_t i) const { return *population().parents()[i]; }

	Random& Crossover::random(size_t i)			         const { return solver().random(i); }
	
	//help, how is the best
	bool Crossover::loss_function_compare(Scalar left, Scalar right) const       { return  solver().loss_function_compare(left,right);  }
	bool Crossover::validation_function_compare(Scalar left, Scalar right) const { return  solver().validation_function_compare(left,right);  }
	bool Crossover::test_function_compare(Scalar left, Scalar right) const       { return  solver().test_function_compare(left,right);  }

	//map
	static std::map< std::string, CrossoverFactory::CreateObject >& c_map()
	{
		static std::map< std::string, CrossoverFactory::CreateObject > c_map;
		return c_map;
	}
	//public
	Crossover::SPtr CrossoverFactory::create(const std::string& name, const EvolutionMethod& emethod)
	{
		//find
		auto it = c_map().find(name);
		//return
		return it == c_map().end() ? nullptr : it->second(emethod);
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
	std::string CrossoverFactory::names_of_crossovers(const std::string& sep)
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

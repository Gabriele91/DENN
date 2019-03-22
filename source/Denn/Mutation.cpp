#include "Denn/Denn/Mutation.h"
#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//Mutation
	Mutation::Mutation(const EvolutionMethod& emethod)
	: m_emethod(emethod)
	{
	}

	//easy access
	const EvolutionMethod& Mutation::evolution_method() const	{ return m_emethod;  }
	const Solver& Mutation::solver()            		 const  { return evolution_method().solver(); }
	const Parameters& Mutation::parameters()            const  { return solver().parameters(); }

	const SubPopulation& Mutation::population()  const { return evolution_method().population(); }
	const size_t Mutation::current_np()          const { return population().size(); }
	const Individual& Mutation::parent(size_t i) const { return *population().parents()[i]; }

	Random& Mutation::random(size_t i)			         const { return solver().random(i); }
	
	//help, how is the best
	bool Mutation::loss_function_compare(Scalar left, Scalar right) const       { return  solver().loss_function_compare(left,right);  }
	bool Mutation::validation_function_compare(Scalar left, Scalar right) const { return  solver().validation_function_compare(left,right);  }
	bool Mutation::test_function_compare(Scalar left, Scalar right) const       { return  solver().test_function_compare(left,right);  }

	//map
	static std::map< std::string, MutationFactory::CreateObject >& m_map()
	{
		static std::map< std::string, MutationFactory::CreateObject > m_map;
		return m_map;
	}
	//public
	Mutation::SPtr MutationFactory::create(const std::string& name, const EvolutionMethod& emethod)
	{
		//find
		auto it = m_map().find(name);
		//return
		return it == m_map().end() ? nullptr : it->second(emethod);
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

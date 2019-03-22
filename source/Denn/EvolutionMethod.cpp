#include "Denn/Denn/EvolutionMethod.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//EvolutionMethod
	EvolutionMethod::EvolutionMethod(const Solver& solver, SubPopulation& sub_population)
	: m_solver(solver)
	, m_sub_population(sub_population)
	{
	}
    
    //virtual
    void EvolutionMethod::start() {};
    void EvolutionMethod::start_a_gen_pass() {};
    void EvolutionMethod::start_a_subgen_pass() {};
    void EvolutionMethod::end_a_subgen_pass() {};
    void EvolutionMethod::end_a_gen_pass() {};
    const VariantRef EvolutionMethod::get_context_data() const { return VariantRef(); }
    
	//easy access	
	const Solver& EvolutionMethod::solver()                    const { return m_solver;        }
	const Parameters& EvolutionMethod::parameters()            const { return solver().parameters();        }

	SubPopulation& EvolutionMethod::population() { return m_sub_population;    }
	const SubPopulation& EvolutionMethod::population() const   { return m_sub_population;    }
	const size_t EvolutionMethod::current_np()         const   { return population().size(); }

	Random& EvolutionMethod::random()		  const { return solver().random(); }
	Random& EvolutionMethod::random(size_t i) const { return solver().random(i); }

	//help, how is the best
	bool EvolutionMethod::loss_function_compare(Scalar left, Scalar right) const       { return  solver().loss_function_compare(left,right);  }
	bool EvolutionMethod::validation_function_compare(Scalar left, Scalar right) const { return  solver().validation_function_compare(left,right);  }
	bool EvolutionMethod::test_function_compare(Scalar left, Scalar right) const       { return  solver().test_function_compare(left,right);  }

	//map
	static std::map< std::string, EvolutionMethodFactory::CreateObject >& em_map()
	{
		static std::map< std::string, EvolutionMethodFactory::CreateObject > em_map;
		return em_map;
	}
	//public
	EvolutionMethod::SPtr EvolutionMethodFactory::create(const std::string& name, const Solver& solver,  SubPopulation& population)
	{
		//find
		auto it = em_map().find(name);
		//return
		return it == em_map().end() ? nullptr : it->second(solver, population);
	}
	void EvolutionMethodFactory::append(const std::string& name, EvolutionMethodFactory::CreateObject fun, size_t size)
	{
		//add
		em_map()[name] = fun;
	}
	//list of methods
	std::vector< std::string > EvolutionMethodFactory::list_of_evolution_methods()
	{
		std::vector< std::string > list;
		for (const auto & pair : em_map()) list.push_back(pair.first);
		return list;
	}
	std::string  EvolutionMethodFactory::names_of_evolution_methods(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_evolution_methods();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	//info
	bool EvolutionMethodFactory::exists(const std::string& name)
	{
		//find
		auto it = em_map().find(name);
		//return 
		return it != em_map().end();
	}
}

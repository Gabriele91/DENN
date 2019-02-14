#include "Denn/RuntimeOutput.h"
#include "Denn/Algorithm.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn 
{
	//easy access		
	const EvolutionMethod&        RuntimeOutput::evolution_method() const{ return m_algorithm.evolution_method();  }
	const Parameters&             RuntimeOutput::parameters()       const{ return m_algorithm.parameters();        }
	const DoubleBufferPopulation& RuntimeOutput::population()       const{ return m_algorithm.population(); }
	const size_t                  RuntimeOutput::current_np()       const{ return m_algorithm.current_np(); }
	//map
	static std::map< std::string, RuntimeOutputFactory::CreateObject >& ro_map()
	{
		static std::map< std::string, RuntimeOutputFactory::CreateObject > ro_map;
		return ro_map;
	}
	//public
	RuntimeOutput::SPtr RuntimeOutputFactory::create(const std::string& name, std::ostream& stream, const DennAlgorithm& algorithm)
	{
		//find
		auto it = ro_map().find(name);
		//return
		return it == ro_map().end() ? nullptr : it->second(stream,algorithm);
	}
	void RuntimeOutputFactory::append(const std::string& name, RuntimeOutputFactory::CreateObject fun, size_t size)
	{
		//add
		ro_map()[name] = fun;
	}
	//list of methods
	std::vector< std::string > RuntimeOutputFactory::list_of_runtime_outputs()
	{
		std::vector< std::string > list;
		for (const auto & pair : ro_map()) list.push_back(pair.first);
		return list;
	}
	std::string  RuntimeOutputFactory::names_of_runtime_outputs(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_runtime_outputs();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	//info
	bool RuntimeOutputFactory::exists(const std::string& name)
	{
		//find
		auto it = ro_map().find(name);
		//return 
		return it != ro_map().end();
	}
}

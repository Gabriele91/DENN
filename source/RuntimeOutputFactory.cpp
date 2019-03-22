#include "Denn/RuntimeOutput.h"
#include "Denn/Denn/Solver.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn 
{
	//easy access		
	const Parameters&           RuntimeOutput::parameters()        const{ return solver().parameters();        }
	const Population&           RuntimeOutput::population()        const{ return solver().population(); }
	const Solver&               RuntimeOutput::solver()            const{ return m_solver; }
	//map
	static std::map< std::string, RuntimeOutputFactory::CreateObject >& ro_map()
	{
		static std::map< std::string, RuntimeOutputFactory::CreateObject > ro_map;
		return ro_map;
	}
	//public
	RuntimeOutput::SPtr RuntimeOutputFactory::create(const std::string& name, std::ostream& stream, const Solver& solver)
	{
		//find
		auto it = ro_map().find(name);
		//return
		return it == ro_map().end() ? nullptr : it->second(stream,solver);
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

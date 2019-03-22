#include "Denn/Evaluation.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//Evaluation
	Evaluation::Evaluation(){}
	//map 
	static std::map< std::string, Evaluation::SPtr >& ev_map()
	{
		static std::map< std::string, Evaluation::SPtr > ev_map;
		return ev_map;
	}
	//public
	Evaluation::SPtr EvaluationFactory::get(const std::string& name)
	{
		//find
		auto it = ev_map().find(name);
		//return
		return it == ev_map().end() ? nullptr : it->second;
	}
	void EvaluationFactory::append(const std::string& name, Evaluation::SPtr evaluetor)
	{
		//add
		ev_map()[name] = evaluetor;
	}
	//list of methods
	std::vector< std::string > EvaluationFactory::list_of_evaluators()
	{
		std::vector< std::string > list;
		for (const auto & pair : ev_map()) list.push_back(pair.first);
		return list;
	}
	std::string EvaluationFactory::names_of_evaluators(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_evaluators();
		std::copy(list.begin(), list.end()-1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	bool EvaluationFactory::exists(const std::string& name)
	{
		//find
		auto it = ev_map().find(name);
		//return 
		return it != ev_map().end();
	}
}

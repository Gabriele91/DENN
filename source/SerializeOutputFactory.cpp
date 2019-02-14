#include "Denn/SerializeOutput.h"
#include "Denn/Parameters.h"
#include <algorithm>
#include <sstream>
#include <iterator>

namespace Denn
{
	//map
	static std::map< std::string, SerializeOutputFactory::CreateObject >& s_map()
	{
		static std::map< std::string, SerializeOutputFactory::CreateObject > s_map;
		return s_map;
	}
	//public
	SerializeOutput::SPtr SerializeOutputFactory::create(const std::string& name, std::ostream& stream, const Parameters& params)
	{
		//find
		auto it = s_map().find(name);
		//return
		return it == s_map().end() ? nullptr : it->second(stream, params);
	}
	void SerializeOutputFactory::append(const std::string& name, SerializeOutputFactory::CreateObject fun, size_t size)
	{
		//add
		s_map()[name] = fun;
	}
	//list of methods
	std::vector< std::string > SerializeOutputFactory::list_of_serialize_outputs()
	{
		std::vector< std::string > list;
		for (const auto & pair : s_map()) list.push_back(pair.first);
		return list;
	}
	std::string  SerializeOutputFactory::names_of_serialize_outputs(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_serialize_outputs();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	//info
	bool SerializeOutputFactory::exists(const std::string& name)
	{
		//find
		auto it = s_map().find(name);
		//return 
		return it != s_map().end();
	}
}

#include <iterator>
#include "Denn/Instance.h"

namespace Denn
{
	//ok
	Instance::Instance() {}
	Instance::Instance(const Denn::Parameters& parameters) {}
	Instance::~Instance(){ }
	//map
	static std::map< std::string, InstanceFactory::CreateObject >& i_map()
	{
		static std::map< std::string, InstanceFactory::CreateObject > i_map;
		return i_map;
	}
	//public
	Instance::SPtr InstanceFactory::create(const std::string& name, const Parameters& parameters)
	{
		//find
		auto it = i_map().find(name);
		//return
		return it == i_map().end() ? nullptr : it->second(parameters);
	}
	void InstanceFactory::append(const std::string& name, InstanceFactory::CreateObject fun, size_t size)
	{
		//add
		i_map()[name] = fun;
	}
	//list of methods
	std::vector< std::string > InstanceFactory::list_of_instances()
	{
		std::vector< std::string > list;
		for (const auto & pair : i_map()) list.push_back(pair.first);
		return list;
	}
	std::string  InstanceFactory::names_of_instances(const std::string& sep)
	{
		std::stringstream sout;
		auto list = list_of_instances();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}
	bool InstanceFactory::exists(const std::string& name)
	{
		//find
		auto it = i_map().find(name);
		//return 
		return it != i_map().end();
	}
}
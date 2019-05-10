#pragma once
#include <sstream>
#include <iterator>
#include "Denn/Config.h"

namespace Denn
{
    
class SplitNetwork
{
public:
	enum Value
	{
		SN_ONE,
		SN_LAYER,
		SN_MATRIX,
		SN_SIZE
	};

	static std::vector<std::string> list_of_splitters()
	{
		return { "one", "layer", "matrix" };
	}

	static std::string names_of_splitters(const std::string& sep = ", ")
	{
		std::stringstream sout;
		auto list = list_of_splitters();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}

	static bool exists(const std::string& value)
	{
		for(auto& type : list_of_splitters()) 
			if(type == value)
				return true;
		return false;
	}

	static Value get(const std::string& value)
	{
		int i=0;
		for(auto& type : list_of_splitters()) 
		{
			if(type == value) return Value(i);
			++i;
		}
		return SN_SIZE;
	}
};

class BuildNetwork
{
public:
	enum Value
	{
		BN_BEST,
		BN_PBEST,
		BN_ROULETTE,
		BN_RANDOM,
		BN_LINE,
		BN_SIZE
	};

	static std::vector<std::string> list_of_builders()
	{
		return { "best", "pbest", "roulette", "random", "line" };
	}

	static std::string names_of_builders(const std::string& sep = ", ")
	{
		std::stringstream sout;
		auto list = list_of_builders();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}

	static bool exists(const std::string& value)
	{
		for(auto& type : list_of_builders()) 
			if(type == value)
				return true;
		return false;
	}

	static Value get(const std::string& value)
	{
		int i=0;
		for(auto& type : list_of_builders()) 
		{
			if(type == value) return Value(i);
			++i;
		}
		return BN_SIZE;
	}
};

class GlobalSelectionNetwork
{
public:
	enum Value
	{
		GN_BEST,
		GN_LINE,
		GN_CROSS,
		GN_SIZE
	};

	static std::vector<std::string> list_of_global_selectors()
	{
		return {"best", "line", "cross"};
	}

	static std::string names_of_global_selectors(const std::string &sep = ", ")
	{
		std::stringstream sout;
		auto list = list_of_global_selectors();
		std::copy(list.begin(), list.end() - 1, std::ostream_iterator<std::string>(sout, sep.c_str()));
		sout << *(list.end() - 1);
		return sout.str();
	}

	static bool exists(const std::string &value)
	{
		for (auto &type : list_of_global_selectors())
			if (type == value)
				return true;
		return false;
	}

	static Value get(const std::string &value)
	{
		int i = 0;
		for (auto &type : list_of_global_selectors())
		{
			if (type == value)
				return Value(i);
			++i;
		}
		return GN_SIZE;
	}
};

}
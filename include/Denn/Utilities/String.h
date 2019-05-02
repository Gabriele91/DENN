#pragma once
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
//String
namespace Denn
{

	
	inline std::string str_rtrim(const std::string &s)
	{
		auto wsfront=s.begin();
		auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
		return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
	}
	
	inline std::string str_ltrim(const std::string &s)
	{
		auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
		auto wsback=s.end();
		return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
	}

	inline std::string str_trim(const std::string &s)
	{
		auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
		auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
		return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
	}

	inline std::string str_replace(std::string str, const std::string& old_str, const std::string& new_str)
	{
		std::string::size_type pos = 0u;
		while ((pos = str.find(old_str, pos)) != std::string::npos)
		{
			str.replace(pos, old_str.length(), new_str);
			pos += new_str.length();
		}
		return str;
	}

	inline bool case_insensitive_equal(const std::string& lstr, const std::string& rstr)
	{
		//not equal len
		if (lstr.size() != rstr.size()) return false;
		//test
		for (std::string::const_iterator c1 = lstr.begin(), c2 = rstr.begin(); c1 != lstr.end(); ++c1, ++c2)
		{
			if (std::tolower(*c1) != std::tolower(*c2)) return false;
		}
		//..
		return true;
	}

	template< class T >
	inline std::string to_string_with_precision(const T a_value, const size_t precision = 6)
	{
		std::ostringstream out;
		out.precision(precision);
		out << std::fixed << a_value;
		return out.str();
	}

	std::vector<std::string> str_split(const std::string& s, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream token_stream(s);
		while (std::getline(token_stream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}
}
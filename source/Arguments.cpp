#include "Denn/Arguments.h"
#include <cctype>

namespace Denn
{
	/////////////////////////////////////////////////////////////////////////
	MainArguments::MainArguments(int nargs, const char** vargs)
	: m_n_arg(nargs)
	, m_rem_arg(nargs)
	, m_pointer(vargs)
	{
	}

	const char* MainArguments::get_string()
	{
		denn_assert(m_rem_arg);
		--m_rem_arg;
		return *(m_pointer++);
	}

	bool MainArguments::get_bool()
	{
		std::string arg = get_string();
		std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
		return arg == std::string("true")
			|| arg == std::string("yes")
			|| arg == std::string("t")
			|| arg == std::string("y");
	}

	int MainArguments::get_int()
	{
		return atoi(get_string());
	}

	double MainArguments::get_double()
	{
		return atof(get_string());
	}

	bool MainArguments::eof() const
	{
		return m_rem_arg == 0;
	}

	bool MainArguments::back()
	{
		if (m_n_arg <= m_rem_arg) return false;
		++m_rem_arg;
		--m_pointer;
		return true;
	}

	bool MainArguments::end_vals() const
	{
		return eof() || ((*m_pointer)[0] == '-' && !std::isdigit((*m_pointer)[1]));
	}
	/////////////////////////////////////////////////////////////////////////
	StringArguments::StringArguments(const char* values, const std::vector<char>& end_vals)
	: m_end_vals(end_vals)
	, m_values(values)
	, m_values_start(values)
	{
		m_skip_space = [](StringArguments& self, const char*& ptr)
		{
			 while (std::isspace(*ptr) && !self.end_vals()) ++ptr;
		};
		m_back_skip_space = [](StringArguments& self, const char* start, const char*& ptr)
		{ 
			while (std::isspace(*ptr) && !self.end_vals()) --ptr;
		};
	}

	StringArguments::StringArguments(SkipSpace skip_space, BackSkipSpace back_skip_space, const char* values, const std::vector<char>& end_vals)
	: m_skip_space(skip_space)
	, m_back_skip_space(back_skip_space)
	, m_preprocess(nullptr)
	, m_end_vals(end_vals)
	, m_values(values)
	, m_values_start(values)
	{
	}

	StringArguments::StringArguments(SkipSpace skip_space, BackSkipSpace back_skip_space, InputPreProcessing preprocess, const char* values, const std::vector<char>& end_vals)
	: m_skip_space(skip_space)
	, m_back_skip_space(back_skip_space)
	, m_preprocess(preprocess)
	, m_end_vals(end_vals)
	, m_values(values)
	, m_values_start(values)
	{
	}

	const char* StringArguments::get_string()
	{
		denn_assert(!eof());
		//add into buffer
		m_buffer.clear();
		while (!std::isspace(*m_values))
		{
			m_buffer += *m_values;  
			++m_values;
		}
		//skip space
		m_skip_space(*this,m_values);
		//pre process
		if(m_preprocess) m_preprocess(m_buffer);
		//get
		return m_buffer.c_str();
	}

	bool StringArguments::get_bool()
	{
		std::string arg = get_string();
		std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
		return arg == std::string("true")
			|| arg == std::string("yes")
			|| arg == std::string("t")
			|| arg == std::string("y");
	}

	int StringArguments::get_int()
	{
		return atoi(get_string());
	}

	double StringArguments::get_double()
	{
		return atof(get_string());
	}

	bool StringArguments::eof() const
	{
		return (*m_values) == '\0';
	}
	
	bool StringArguments::back()
	{
		if (m_values == m_values_start) return false;
		//jump first char of last value
		--m_values;
		//jump spaces
		m_back_skip_space(*this,m_values_start, m_values);
		//go to first char of last value
		while (!std::isspace(*(m_values-1)) && !end_vals() && m_values != m_values_start) --m_values;
		//ok
		return true;
	}

	bool StringArguments::end_vals() const
	{
		if (eof()) return true;
		for (char end : m_end_vals) 
			if ((*m_values) == end) return true;
		return false;
	}

	const char* StringArguments::get_ptr() const
	{
		return m_values;
	}
	/////////////////////////////////////////////////////////////////////////
}
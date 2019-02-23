#include "Denn/Parameters.h"
#include "Denn/Utilities/Networks.h"
#include <string>
#include <cctype>
#include <unordered_map>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Denn
{
    //////////////////////////////////////////////////////////////////////////////////
    static inline bool conf_is_digit(char c)
    {
        return (c >= '0' && c <= '9');
    }    

	static inline bool conf_is_variable(char c)
	{
		return c == '$';
	}

	static inline bool conf_is_varname(char c)
	{
		return std::isalpha(c) || c == '_';
	}

    static inline bool conf_is_xdigit(char c)
    {
        return (c >= '0' && c <= '9') || ((c & ~' ') >= 'A' && (c & ~' ') <= 'F');
    }

    static inline bool conf_is_space(char c)
    {
        return c == ' ' || (c >= '\t' && c <= '\r');
    }    
    
    static inline int conf_char_to_int(char c)
    {
        return (c <= '9') ? c - '0' : (c & ~' ') - 'A' + 10;
    }
    
    static long conf_string_to_int(const char*& source)
    {
        //+/-
        char ch = *source;
        if (ch == '-') ++source;
        //integer part
        long result = 0;
        while (conf_is_digit(*source)) result = (result * 10) + (*source++ - '0');
        //result
        return ch == '-' ? -result : result;
    }

    static long conf_string_to_int_no_skip(const char* source)
    {
        //+/-
        char ch = *source;
        if (ch == '-') ++source;
        //integer part
        long result = 0;
        while (conf_is_digit(*source)) result = (result * 10) + (*source++ - '0');
        //result
        return ch == '-' ? -result : result;
    }
	
	static double conf_string_to_double(const char*& source)
	{
		//+/-
		char ch = *source;
		if (ch == '-') ++source;
		//integer part
		double result = 0;
		while (conf_is_digit(*source)) result = (result * 10) + (*source++ - '0');
		//fraction
		if (*source == '.')
		{
			++source;
			double fraction = 1;
			while (conf_is_digit(*source))
			{
				fraction *= 0.1;
				result += (*source++ - '0') * fraction;
			}
		}
		//exponent
		if (*source == 'e' || *source == 'E')
		{
			++source;
			//base of exp
			double base = 10;
			//+/- exp
			if (*source == '+')
			{
				++source;
			}
			else if (*source == '-')
			{
				++source;
				base = 0.1;
			}
			//parsing exponent
			unsigned int exponent = 0;
			while (conf_is_digit(*source)) exponent = (exponent * 10) + (*source++ - '0');
			//compute exponent
			double power = 1;
			for (; exponent; exponent >>= 1, base *= base) if (exponent & 1) power *= base;
			//save result
			result *= power;
		}
		//result
		return ch == '-' ? -result : result;
	}
    //////////////////////////////////////////////////////////////////////////////////        
    static bool conf_skip_line_comment(size_t& line, const char*& inout)
    {
        //not a line comment
        if ((*inout) != '/' || *(inout+1) != '/') return false;
        //skeep
        while (*(inout) != EOF && *(inout) != '\0'&& *(inout) != '\n') ++(inout);
        //jump endline
        if ((*(inout)) == '\n')
        {
            ++line;  ++inout;
        }
        //ok
        return true;
    }
    static bool conf_skip_multilines_comment(size_t& line, const char*& inout)
    {
        //not a multilines comment
        if ((*inout) != '/' || *(inout + 1) != '*') return false;
        //jump
        while 
        (
            *(inout) != EOF &&
            *(inout) != '\0' && 
                ((*inout) != '*' || *(inout + 1) != '/')
        )
        {
            line += (*(inout)) == '\n';
            ++inout;
        }
        //jmp last one
        if ((*inout) == '*' && *(inout + 1) == '/') inout += 2;
        //ok
        return true;
    }
    static bool conf_skip_space(size_t& line, const char*& source)
    {
        bool a_space_is_skipped = false;
        while (conf_is_space(*source))
        {
            //to true
            a_space_is_skipped = true;
            //count line
            if (*source == '\n') ++line;
            //jump
            ++source;
            //exit
            if (!*source) break;
        }
        return a_space_is_skipped;
    }
	static bool conf_skip_comments(size_t& line, const char*& source)
	{
		bool eat = false;
		while (conf_skip_line_comment(line, source)
			|| conf_skip_multilines_comment(line, source)) eat = true;
		return eat;
	}    
    static bool conf_skip_space_and_comments(size_t& line, const char*& source)
    {
        bool eat = false;
        while (conf_skip_space(line, source)
            || conf_skip_line_comment(line, source)
            || conf_skip_multilines_comment(line, source)) eat = true;
        return eat;
    }
    static bool conf_skip_line_space(const char*& source)
    {
        bool a_space_is_skipped = false;
        while (conf_is_space(*source) && *source != '\n')
        {
            //to true
            a_space_is_skipped = true;
            //jump
            ++source;
            //exit
            if (!*source) break;
        }
        return a_space_is_skipped;
    }
    static bool conf_skip_line_comment(const char*& inout)
    {
        //not a line comment
        if ((*inout) != '/' || *(inout + 1) != '/') return false;
        //skeep
        while (*(inout) != EOF && *(inout) != '\0'&& *(inout) != '\n') ++(inout);
        //jump endline
        if ((*(inout)) == '\n') return true;
        //ok
        return true;
    }
    static bool conf_skip_line_space_and_comments(size_t& line, const char*& source)
    {
        bool eat = false;
        while (conf_skip_line_space(source)
            || conf_skip_line_comment(source)
            || conf_skip_multilines_comment(line, source)) eat = true;
        return eat;
    }
    //////////////////////////////////////////////////////////////////////////////////        
    static bool conf_rev_skip_line_comment(size_t& line, const char* start, const char*& inout)
    {
        //copy
        const char* in = inout;
        //stat with endline
        if ((*(in)) != '\n') return false;
        //came back to //
        while (in != start && ((*in) != '/' && *(in-1) != '/')) --in;
        //jump //
        if ((*in) != '/' || *(in-1) != '/')
        {
            //jmp //
            inout =  (in - 1) == (start) 
                    ? in - 1
                    : in - 2;
            //remove line
            --line;
            //ok
            return true;
        }
        return false;
    }
    static bool conf_rev_skip_multilines_comment(size_t& line, const char* start, const char*& inout)
    {
        //copy
        const char* in = inout;
        size_t tmp_line = line;
        //test
        if(in == start) return false;
        //not a multilines comment
        if ((*in) != '/' || *(in - 1) != '*') return false;
        //jump
        while 
        (in != start && ((*in) != '*' || *(in - 1) != '/'))
        {
            tmp_line -= (*(in)) == '\n';
            --in;
        }
        //is /*
        if ((*in) == '*' && *(in - 1) == '/')
        {
            //jmp /*
            inout =  (in - 1) == (start) 
                    ? in - 1
                    : in - 2;
            //update line count
            line += tmp_line;
            //ok
            return true;
        }
        return false;
    }
    static bool conf_rev_skip_space(size_t& line, const char* start, const char*& source)
    {
        bool a_space_is_skipped = false;
        while (conf_is_space(*source))
        {
            //to true
            a_space_is_skipped = true;
            //count line
            if (*source == '\n') --line;
            //jump
            --source;
            //exit
            if (source != start) break;
        }
        return a_space_is_skipped;
    }    
    static void conf_rev_skip_space_and_comments(size_t& line, const char* start, const char*& source)
    {
        while (conf_rev_skip_space(line, start, source)
            || conf_rev_skip_line_comment(line, start, source)
            || conf_rev_skip_multilines_comment(line, start, source))
        {
            if(source == start) break;
        }
    }
    static bool conf_rev_skip_line_space(const char* start, const char*& source)
    {
        bool a_space_is_skipped = false;
        while (conf_is_space(*source) && *source != '\n')
        {
            //to true
            a_space_is_skipped = true;
            //jump
            --source;
            //exit
            if (source != start) break;
        }
        return a_space_is_skipped;
    }
    static bool conf_rev_skip_line_comment(const char* start, const char*& inout)
    {
        //copy
        const char* in = inout;
        //stat with endline
        if ((*(in)) != '\n') return false;
        //came back to //
        while (in != start && ((*in) != '/' && *(in - 1) != '/')) --in;
        //jump //
        if ((*in) != '/' || *(in - 1) != '/')
        {
            //jmp //
            inout = (in - 1) == (start)
                ? in - 1
                : in - 2;
            //ok
            return true;
        }
        return false;
    }
    static bool conf_rev_skip_line_space_and_comments(size_t& line, const char* start, const char*& source)
    {
        bool eat = false;
        while (conf_rev_skip_line_space(start, source)
            || conf_rev_skip_line_comment(start, source)
            || conf_rev_skip_multilines_comment(line, start, source))
        {
            eat = true;
            if (source == start) break;
        } 
        return eat;
    }
    //////////////////////////////////////////////////////////////////////////////////        
    struct conf_string_out { std::string m_str; bool m_success; };
    static conf_string_out conf_string(size_t& line, const char*& source)
    {
        //init
        std::string out;
        //start parse
        if ((*source) == '\"')  //["...."]
        {
            ++source;  //[...."]
            while ((*source) != '\"' && (*source) != '\n')
            {
                if ((*source) == '\\') //[\.]
                {
                    ++source;  //[.]
                    switch (*source)
                    {
                    case 'n':
                        out += '\n';
                        break;
                    case 't':
                        out += '\t';
                        break;
                    case 'b':
                        out += '\b';
                        break;
                    case 'r':
                        out += '\r';
                        break;
                    case 'f':
                        out += '\f';
                        break;
                    case 'a':
                        out += '\a';
                        break;
                    case '\\':
                        out += '\\';
                        break;
                    case '?':
                        out += '\?';
                        break;
                    case '\'':
                        out += '\'';
                        break;
                    case '\"':
                        out += '\"';
                        break;
                    case 'u':
                    {
                        int c = 0;
                        //comput u
                        for (int i = 0; i < 4; ++i)
                        {
                            if (conf_is_xdigit(*++source))
                            {
                                c = c * 16 + conf_char_to_int(*source);
                            }
                            else
                            {
                                return { out, false };
                            }
                        }
                        //ascii
                        if (c < 0x80)
                        {
                            out += c;
                        }
                        //utf 2 byte
                        else if (c < 0x800)
                        {
                            out += 0xC0 | (c >> 6);
                            out += 0x80 | (c & 0x3F);
                        }
                        //utf 3 byte
                        else
                        {
                            out += 0xE0 | (c >> 12);
                            out += 0x80 | ((c >> 6) & 0x3F);
                            out += 0x80 | (c & 0x3F);
                        }
                    }
                    break;
                    case '\n': /* jump unix */
                        ++line;
                        break;
                    case '\r': /* jump mac */
                        ++line;
                        if ((*(source + 1)) == '\n') ++source; /* jump window (\r\n)*/
                        break;
                    default:
                        return { out, false };
                        break;
                    }
                }
                else
                {
                    if ((*source) != '\0') out += (*source);
                    else return { out, false };
                }
                ++source;//next char
            }
            //exit cases
            if ((*source) == '\n')
            {
                return { out, false };
            }
            else if ((*source) == '\"')
            {
                ++source;
                return { out, true };
            }
        }
        return { out, false };
    }
    //name
	static std::string conf_name(const char*& source)
	{
		//init
		std::string out;
		//first
		if (std::isalpha(*source) || *source == '_' || *source == '-' )
		{
			out += (*source);
			++source;
		}
		//parse name
		while ( std::isalnum(*source) || *source == '_' || *source == '-' || *source == '.')
		{
			out += (*source);
			++source;
		}
		//end
		return out;
	}
    ////////////////////////////////////////////////////////////////////////////////////////
    class VariableTable
    {
    public:
        
        void add_vairable(const std::string& variable,const std::string& default_value) const
        {
            m_map[variable] = default_value;
        }
        
        bool change_value(const std::string& variable,const std::string& new_value)
        {
            if(m_map.find(variable) == m_map.end()) return false;       
            m_map[variable] = new_value;
            return true;
        }

        bool exists(const std::string& variable) const
        {
            return m_map.find(variable) != m_map.end();
        }

        std::string get(const std::string& variable) const 
        {
            auto value_it = m_map.find(variable);
            if(value_it == m_map.end()) return "";
            return value_it->second;
        }

        void clear()
        {
            m_map.clear();
        }

    private:

        mutable std::unordered_map< std::string, std::string > m_map;
    };

	class ConfExpParser
	{
	public:
		//value
		struct ExpValue
		{
			//values
			std::string m_str;
			double m_number;
			//init
			ExpValue() : m_number(0) {};
			ExpValue(double value) : m_number(value) {};
			ExpValue(const std::string& value) : m_str(value), m_number(0) {};
			//info
			bool is_string() const { return m_str.size() != 0; }
			bool is_number() const { return m_str.size() == 0; }
			//void
			bool is_false() { return is_number() && !m_number; }
			bool is_true() { return is_string() || m_number; }
			//cast
			//str
			std::string str() const 
			{
				return is_string()
					? m_str 
					: m_number == double(long(m_number))
					? std::to_string(long(m_number))
					: std::to_string(m_number);
			}
			//number
			double number() const
			{
				const char* str_ptr = m_str.data();
				return is_number()
					? m_number
					: conf_string_to_double(str_ptr);
			}
		};
		//alias
		using ListString = std::vector < std::string >;
		using FunctionArgs = std::vector< ExpValue >;
		using FunctionMath = std::function< ExpValue(FunctionArgs&&) >;
		using MapConstants = std::unordered_map < std::string, ExpValue >;
		using MapFunctions = std::unordered_map < std::string, FunctionMath >;
		//init
		ConfExpParser(const VariableTable& table, size_t line, const char* ptr)
		: m_table(table)
		, m_line(line)
		, m_ptr(ptr)
		{
			m_result = expression();
		}

		//result
		const ExpValue& result() const
		{
			return m_result;
		}

		//errors
		const ListString& errors() const
		{
			return m_errors;
		}

		//new ptr
		const char* get_ptr() const
		{
			return m_ptr;
		}
		//new line
		size_t get_line() const
		{
			return m_line;
		}

	protected:
		
		double& number(ExpValue& value, const std::string& opname)
		{
			if (value.is_string()) 
				m_errors.push_back(std::to_string(line()) + ": \'" + opname + "\' not support a string operation");
			return value.m_number;
		}

		std::string& string(ExpValue& value, const std::string& opname)
		{
			if (value.is_number())
				m_errors.push_back(std::to_string(line()) + ": \'" + opname + "\' requires a string as argument");
			return value.m_str;
		}
		
		const double& number(const ExpValue& value, const std::string& opname)
		{
			if (value.is_string())
				m_errors.push_back(std::to_string(line()) + ": \'" + opname + "\' not support a string operation");
			return value.m_number;
		}

		const std::string& string(const ExpValue& value, const std::string& opname)
		{
			if (value.is_number())
				m_errors.push_back(std::to_string(line()) + ": \'" + opname + "\' requires a string as argument");
			return value.m_str;
		}

		ExpValue smart_cast(const std::string& value)
		{
			const char* ptr = value.c_str();
			double result = conf_string_to_double(ptr);
			if (*ptr == '\0') return result;
			return value;
		}
		
		ExpValue name_eval()
		{
			//eat space
			skip();
			//name
			auto name = conf_name(ptr());
			//consts
			MapConstants consts
			{
				  { "false", 0 }
				, { "true", 1 }
				, { "e",  Constants::e()  }
				, { "pi", Constants::pi() }
			};
			//test
			auto const_it = consts.find(name);
			if (const_it != consts.end()) return const_it->second;
			//function?
			auto test = [this](const FunctionArgs& args, size_t n, const std::string& name) -> bool
			{
				if (args.size() == n) return true;
				m_errors.push_back(std::to_string(line()) + ": function \'" + name + "\' get " + std::to_string(n) + " argument[s]");
				return false;
			};
			MapFunctions funs
			{
				//Maths
				  { "log"  , [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "log"))   return std::log(number(args[0],"log")); else return {0}; } }
				, { "log10", [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "log10")) return std::log10(number(args[0],"log10")); else return {0}; } }	
				, { "exp"  , [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "exp"))   return std::exp(number(args[0],"exp")); else return {0}; } }
				//Cast
				, { "floor", [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "floor"))  return std::floor(number(args[0],"floor")); else return {0}; } }
				, { "ceil" , [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "ceil"))   return std::ceil(number(args[0],"ceil")); else return {0}; } }
				, { "str",   [&](FunctionArgs&& args) -> ExpValue 
							 {
									if (!test(args, 1, "str")) return ExpValue("");
									if (args[0].is_number())
									{
										return std::to_string(args[0].m_number);
									}
									//return string
									return args[0];
							 } 
				  }			
				, { "float", [&](FunctionArgs&& args) -> ExpValue
							 {
								if (!test(args, 1, "float")) return 0;
								if (args[0].is_string())
								{
									auto ptr = args[0].m_str.c_str();
									return conf_string_to_double(ptr);
								}
								//return num
								return args[0];
							 }
				  }
				, { "int",   [&](FunctionArgs&& args) -> ExpValue
							 {
								if (!test(args, 1, "int")) return 0;
								if (args[0].is_number())
								{
									return double(int(args[0].m_number));
								}
								auto ptr = args[0].m_str.c_str();
								return double(int(conf_string_to_double(ptr)));
							 }
				  }			
				//Trigonometry 
				, { "radian",[&](FunctionArgs&& args) -> ExpValue  { if (test(args, 1, "radian")) return number(args[0],"str") * (Constants::pi() / Scalar(180.0)); else return {""}; } }
				, { "sin"  , [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "sin"))   return std::sin(number(args[0],"sin")); else return {0}; } }
				, { "cos"  , [&](FunctionArgs&& args) -> ExpValue { if (test(args, 1, "cos"))   return std::cos(number(args[0],"cos")); else return {0}; } }
				, { "tan",   [&](FunctionArgs&& args) -> ExpValue  { if (!test(args, 1, "tan"))  return std::tan(number(args[0], "tan")); else return {0}; } }
				, { "atan",  [&](FunctionArgs&& args) -> ExpValue  { if (!test(args, 1, "atan")) return std::tan(number(args[0], "atan")); else return {0}; } }
				, { "atan2", [&](FunctionArgs&& args) -> ExpValue
							 {
							     if (!test(args, 2, "atan2")) return std::atan2(number(args[0], "atan2"),number(args[1], "atan2")); else return {0};
							 }
				  }
				//Time
				, { "date",  [&](FunctionArgs&& args) -> ExpValue 
							{ 
								 if (test(args, 1, "date"))
								 {
									 std::stringstream stream_data;
									 auto current_time = std::time(nullptr);
									 auto local_time   = *std::localtime(&current_time);
									 stream_data << std::put_time(&local_time, string(args[0],"date").c_str());
									 return stream_data.str();
								 }
								 return ExpValue("");
							} 
				}
				, { "threads", [&](FunctionArgs&& args) -> ExpValue { return double( std::thread::hardware_concurrency() ); } }
			};
			//test
			auto fun_it = funs.find(name);
			if (fun_it != funs.end())
			{
				//get function
				FunctionMath fun = fun_it->second;
				//eat space
				skip();
				//args
				FunctionArgs args;
				//open args // '('
				if (get() != '(')
				{
					m_errors.push_back(std::to_string(line()) + ": '(' of function \'" + name + "\' not found");
					return name;
				}
				//parsing
				while(true)
				{
					//arg
					args.push_back(expression());
					//next
					skip();
					if (peek() != ','){ break; }
					get();
				}
				//close args // ')'
				if (get() != ')')
				{
					m_errors.push_back(std::to_string(line()) + ": ')' of function \'" + name + "\' not found");
					return name;
				}
				//ok
				return fun(std::move(args));
			}
			//or is a string
			return name;
		}

		ExpValue value()
		{
			//eat space
			skip();
			//parsing
			if (peek() == '(')
			{
				get(); // '('
				ExpValue result = expression();
				//need close
				if (get() != ')')
					m_errors.push_back(std::to_string(line()) + ": ')' not found");
				return result;
			}
			else if (peek() == '\"')
			{
				const auto& value = conf_string(line(), ptr());
				if (!value.m_success) m_errors.push_back(std::to_string(line()) + ": expression error, not valid string");
				return value.m_str;
			}
			else if (conf_is_variable(peek()))
			{
				get(); //jmp '$'
				std::string varname = conf_name(ptr());
				//find
				if (!m_table.exists(varname))
				{
					m_errors.push_back(std::to_string(line()) + ": \'" + varname + "\' is not valid variable");
					return 0.0;
				}
				//get value
				return smart_cast(m_table.get(varname));
			}
			else if (conf_is_varname(peek()))
			{
				return name_eval();
			}
			else
			{
				return conf_string_to_double(ptr());
			}
		}

		ExpValue one()
		{
			//eat space
			skip();
			//parsing
			if (peek() == '-')
			{
				get();
				//get result
				ExpValue result = value();
				//name can start with -
				if (result.is_string())
					return '-' + result.m_str;
				else 
					return -result.m_number;
			}
			else if (peek() == '!')
			{
				get();
				return !number(value(), "!");
			}
			else
			{
				return value();
			}
		}

		ExpValue pow()
		{
			//one
			ExpValue result = one();
			//eat space
			skip();
			//test
			if (peek() == '^')
			{
				get();
				return std::pow(number(result, "^"), number(one(), "^"));
			}
			return result;
		}

		ExpValue term()
		{
			//left
			ExpValue result = pow();
			//eat space
			skip();
			//right
			while (peek() == '*' || peek() == '/' || peek() == '%')
			{
				     if (peek() == '*'){ get(); number(result, "*") *= number(pow(), "*"); }
				else if (peek() == '/'){ get();  number(result, "/") /= number(pow(), "/");}
				else 
				{ 
					get();
					double left = number(result, "%");
					double right = number(pow(), "%");
					result.m_number = positive_fmod<double>(left, right);
				}
				//eat space
				skip();
			}
			return result;
		}

		ExpValue mathexp()
		{
			//left
			ExpValue result = term();
			//eat space
			skip();
			//right
			while (peek() == '+' || peek() == '-')
			{
				if (get() == '+')
				{
					//right term
					auto right = term();
					//sum & concatenation 
					if (result.is_number() && right.is_number())
						result.m_number += right.m_number;
					else if (result.is_number() && right.is_string()) 
						result = { std::to_string(result.m_number) + right.m_str };
					else if (result.is_string() && right.is_number()) 
						result.m_str += std::to_string(right.m_number);
					else 
						result.m_str += right.m_str;
				}
				else
				{
					number(result, "-") -= number(term(), "-");
				}
				//eat space
				skip();
			}
			return result;
		}

		ExpValue logicexp()
		{
			//left
			ExpValue result = mathexp();
			//eat space
			skip();
			//right
			while (peek() == '&' || peek() == '|')
			{
				if (get() == '&')
				{ 
					if (result.is_true() && (result = mathexp()).is_true()) break;
				}
				else
				{
					if (result.is_true() || (result = mathexp()).is_true()) break;
				}
				//eat space
				skip();
			}
			return result;
		}

		ExpValue expression()
		{
			//eat space
			skip();
			//return
			return logicexp();
		}

	private:

		const char*& ptr()
		{
			return m_ptr;
		}

		size_t& line()
		{
			return m_line;
		}

		char peek() const
		{
			return *m_ptr;
		}

		char get()
		{
			return *m_ptr++;
		}

		bool skip()
		{
			return conf_skip_line_space_and_comments(line(), ptr());
		}

		//values
		size_t               m_line;
		const char*          m_ptr;
		ListString           m_errors;
		const VariableTable& m_table;
		ExpValue	         m_result;
	};

	class ConfArguments : public Arguments
	{
	public:
		//alias
		using ListChar = std::vector < char >;
		using ListString = std::vector < std::string >;
		//class methods
		ConfArguments(const VariableTable& table, size_t& line, const char* ptr, const ListChar& end_line)
		{
			//init 
			m_ptr = ptr;
			m_index = 0;
			//parsing
			while (*m_ptr && std::find(end_line.begin(), end_line.end(), *m_ptr) == end_line.end())
			{
				//remove space
				conf_skip_line_space_and_comments(line, m_ptr);
				//push in list
				if(!variable_processing(table, line))
                    break; //error
			}
		}

		const char* get_string() override
		{
            //void
			if(eof()) return "";
			//get
			return m_values[m_index++].c_str();
		}

		bool get_bool() override
		{
			std::string arg = get_string();
			std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
			return arg == std::string("true")
				|| arg == std::string("yes")
				|| arg == std::string("t")
				|| arg == std::string("y")
				|| atoi(arg.c_str()) != 0;
		}

		int get_int() override
		{
			return atoi(get_string());
		}

		double get_double() override
		{
			return atof(get_string());
		}

		bool eof() const override
		{
			return m_values.size() <= m_index;
		}

		bool back() override
		{
			//index is 0? It can't go back
			if (!m_index) return false;
			//else go back
			--m_index;
			//ok
			return true;
		}

		bool end_vals() const override
		{
			return eof();
		}

		const ListString& errors() const
		{
			return m_errors;
		}

		const char* get_ptr() const
		{
			return m_ptr;
		}

	protected:

		//ptr
		const char* m_ptr;
		//info buffer
		size_t m_index;
		ListString m_values;
		//errors
		ListString m_errors;
		//variable processing
		bool variable_processing(const VariableTable& table, size_t& line)
		{
			//exp
			ConfExpParser exp(table, line, m_ptr);
			//exp error?
			if (exp.errors().size())
			{
				for (auto&& error : exp.errors()) m_errors.push_back(error);
				//false
				return false;
			}
			//get value
			std::stringstream value(exp.result().str());
			//split by space
			std::istream_iterator<std::string> it(value);
			std::istream_iterator<std::string> end;
			//add all tokens
			for (; it != end; ++it) m_values.push_back(*it);
			//move ptr
			m_ptr = exp.get_ptr();
			line  = exp.get_line();
			//ok
			return true;
		}
	};
    
	class ParametersParseHelp
    {
    public:
		//////////////////////////////////////////////////////////////////////////////////
		static bool conf_parse_cline_args
		(
			  VariableTable& context
			, int nargs
			, const char **vargs
		)
		{
			enum State
			{
			  START
			, NAME
			, EQUAL
			, VALUE
			};
			//Parse state
			State state{ START };
			//values
			std::string name;
			std::string value;
			//parsing
			for (int i = 0; i < nargs; ++i)
			{
				//ptr
				const char* ptr = vargs[i];
				//parse
				while (*ptr)
				{
					switch (state)
					{
					case START:
						//begin
						conf_skip_line_space(ptr);
						name.clear();
						value.clear();
						state = NAME;
					break;
					case NAME:
						name = conf_name(ptr);
						if (!name.size())
						{
							std::cerr << "Name argument is not valid" << std::endl;
							return false;
						}
						state = EQUAL;
					break;
					case EQUAL:
						if (*ptr != '=')
						{
							std::cerr << "\'=\' is not found" << std::endl;
							return false;
						}
						++ptr;
						state = VALUE;
					break;
					case VALUE:
						while(*ptr) value += (*ptr++);
						if (!value.size())
						{
							std::cerr << "Value argument is not valid" << std::endl;
							return false;
						}
						state = START;
						//add value
						context.add_vairable(name, value);
					break;
					default:
						return false;
					break;
					}
				}

			}
			//parsing end state
			return state == START;
		}

		static Parameters::ReturnType main
		(
          Parameters& params
		, const std::vector< ParameterInfo >& params_info
		, VariableTable& context
		, size_t& line
		, const char*& ptr
		)
		{
			//cases
			enum ParserState
			{
				PS_ARG,
				PS_VAR,
				PS_NETWORK,
			};
			//jump
			conf_skip_space_and_comments(line, ptr);
			//ptr before command
			const char* ptr_pre_command = ptr;
            //command
            std::string command = conf_name(ptr);
            //test
            if (!command.size())
			{
                std::cerr << line << ": command not valid" << std::endl;
                return Parameters::FAIL;
			}
			//parser state
			ParserState state;
				 if (command == "network") state = PS_NETWORK;
			else if(command == "var")	   state = PS_VAR;
			else 						   state = PS_ARG;
			//cases
			switch (state)
			{
				case PS_NETWORK:
					// {....}
					if (!conf_parse_net(params, context, line, ptr)) return Parameters::FAIL;
				break;
				case PS_VAR:
					// var {....} or var ..., ...
					if (!conf_parse_variables(params, context, line, ptr)) return Parameters::FAIL;
				break;
				default:
					//reset ptr
					ptr = ptr_pre_command;
					//parse
					if (!conf_parse_arg(params_info, context, line, ptr)) return Parameters::FAIL;
				break;
			}
			return Parameters::SUCCESS;
		}

	protected:
        //////////////////////////////////////////////////////////////////////////////////
        static bool conf_parse_arg
        (
              const std::vector< ParameterInfo >& info
            , const VariableTable& context
            , size_t& line
            , const char*& ptr
            , ParameterInfo ower = ParameterInfo()
        )
        {
			//jump
			conf_skip_space_and_comments(line, ptr);
            //it's close
            if((*ptr) == '}') return true;
            //command
            std::string command = conf_name(ptr);
            //test
            if (!command.size())
            {
                std::cerr << line << ": command not valid" << std::endl;
                return false;
            }
            //jump
            conf_skip_space_and_comments(line, ptr);
            //action
            bool is_a_valid_arg = false;
            //vals            
            for (auto& action : info)
            {
                //test
                if (!action.m_associated_variable) continue;
                if (!action.m_oweners.test(ower)) continue;
                //search param 
                bool found = false;
                //as shell arg or command
                if (command[0] == '-')
                {
                    for (const std::string& key : action.m_arg_key)
                    {
                        if (key == command)
                        {
                            found = true;
                            break;
                        }
                    }
                }
                else
                {
                    found = command == action.m_associated_variable->name();
                }
                //parse a line 
                if (found)
                {
					//line parser
					ConfArguments args(context, line, ptr, { '\n', '}', '{' });
                    //test action
                    if (!action.m_action(args))
                    {
                        //preprocess & fail arg parse?
                        if (args.errors().size())
							for (const auto& error : args.errors()) std::cerr << error << std::endl;
                        else
                            std::cerr << line << ": not valid arguments for command \'" << command << "\'" << std::endl;
                        return false;
                    }
                    //preprocess error
                    if (args.errors().size())
                    {
						for(const auto& error : args.errors()) std::cerr << error << std::endl;
                        return false;
                    }
                    //update
                    ptr = args.get_ptr();
					//next value
                    conf_skip_space_and_comments(line, ptr);
                    //sub args?
                    if (*ptr == '{')
                    {
                        //jump {
                        ++ptr;
                        conf_skip_space_and_comments(line, ptr);
                        //sub args
                        do
                        {
                            if (!conf_parse_arg(info, context, line, ptr, action)) return false;
                        } 
                        while (*ptr && (*ptr) != '}');
                        //test
                        if (*ptr != '}')
                        {
                            std::cerr << line << ": } not found" << std::endl;
                            return false;
                        }
                        //jump }
                        ++ptr;
                        conf_skip_space_and_comments(line, ptr);
                    }
                    //ok
                    is_a_valid_arg = true;
                    break;
                }
            }
            //fail
            if (!is_a_valid_arg)
            {
                std::cerr << line << ": command \'" << command << "\' not found" << std::endl;
                return false;
            }
            //jump
            conf_skip_space_and_comments(line, ptr);
            return true;
        }
		
        //////////////////////////////////////////////////////////////////////////////////
		// network
        static bool conf_parse_net
        (
              Parameters& params
            , const VariableTable& context
            , size_t& line
            , const char*& ptr
        )
        {
			//jump
			conf_skip_space_and_comments(line, ptr);
            //{
            if((*ptr) == '{')
			{
				++ptr;
			}
			else
			{
				std::cerr << line << ": network command need \'{\' " << std::endl;
				return false;
			}
			//buffer
			std::string nnlayout;
			//jump spaces
			conf_skip_space_and_comments(line, ptr);
            //from { to }
			while (*ptr && ((*ptr) != '}'))
			{
			    //exp
			    ConfExpParser exp(context, line, ptr);
				//add exp
				if(!exp.errors().size() && exp.get_ptr() != ptr)
				{
				 	nnlayout += exp.result().str();
					nnlayout += " ";
					ptr = exp.get_ptr();
				}
				else if(!isspace(*ptr))
				{
					nnlayout += (*ptr); ++ptr;
			    }
			    //remove space and comment
			    if(conf_skip_space_and_comments(line, ptr))
				    nnlayout += " ";
			}
			//test '}'
            if((*ptr) == '}')
			{
				++ptr;
			}
			else
			{
				std::cerr << line << ": network command need \'}\' " << std::endl;
				return false;
			}
			//test
			auto test_err_succ = get_network_from_string_test(nnlayout);
			if (!std::get<1>(test_err_succ))
			{
				std::cerr << line << ": network layout error: \'" << std::get<0>(test_err_succ) << std::endl;
				return false;
			}
			//save
			params.m_network = nnlayout;
            return true;
        }
        //////////////////////////////////////////////////////////////////////////////////
		// variables
        static bool conf_parse_variables 
        (
              const Parameters& params
            , VariableTable& context
            , size_t& line
            , const char*& ptr
        )
		{
			//jump
			conf_skip_space_and_comments(line, ptr);
			// cases:
			// 1: var '{' {<name> <exp>} '}'
			// 2: var <name> <exp> {[, <name> <exp> ]}
			if(*ptr == '{')
			{
				//jmp {
				++ptr;
				//jump spaces
				conf_skip_space_and_comments(line, ptr);
				//read all
				while (*ptr && *ptr != '}')
				{
					if (!conf_parse_variable(params, context, line, ptr)) return false;
					//search '}'
					conf_skip_space_and_comments(line, ptr);
				}
				//test '}'
				if((*ptr) == '}')
				{
					++ptr;
				}
				else
				{
					std::cerr << line << ": var list command need \'}\' " << std::endl;
					return false;
				}
			}
			else
			{
				while(true)
				{
					//<name> <exp>
					if (!conf_parse_variable(params, context, line, ptr)) return false;
					//search  ','
					conf_skip_space_and_comments(line, ptr);
					//test
					if((*ptr) == ',') ++ptr;
					else 			  break;
				}
			}
			return true;
		}
        static bool conf_parse_variable
        (
              const Parameters& params
            , VariableTable& context
            , size_t& line
            , const char*& ptr
        )
        {
			//jump
			conf_skip_space_and_comments(line, ptr);
            //variable type
            std::string variable_name = conf_name(ptr);			
			//test name
            if(!variable_name.size())
			{
				std::cerr << line << ": not valid variable name" << std::endl;
				return false;
			}
            //jump spaces
            conf_skip_line_space_and_comments(line, ptr);
            //exp parser
			ConfExpParser exp(context, line, ptr);
            //exp error
            if (exp.errors().size())
            {
                for(const auto& error : exp.errors()) std::cerr << error << std::endl;
                return false;
            }
            //update ptr
            ptr = exp.get_ptr();
            //set
            if(!context.exists(variable_name))
            {
                context.add_vairable(variable_name, exp.result().str());
            }
			//ok
            return true;
        }
        //////////////////////////////////////////////////////////////////////////////////
    };
    ////////////////////////////////////////////////////////////////////////////////// 


	Parameters::ReturnType Parameters::from_config(const std::string& source, int nargs, const char **vargs)
    {
        //ptr
        const char* ptr = source.c_str();
        size_t line = 1;
        //jump
        conf_skip_space_and_comments(line, ptr);
        //Var table
        VariableTable context;
        //parse variables from comand line
		if (!ParametersParseHelp::conf_parse_cline_args(context, nargs, vargs))
			return FAIL;
        //parsing
        while (*ptr)
        {
			//get return
			auto rtype=ParametersParseHelp::main(*this, m_params_info, context, line, ptr);
			//test
			if(rtype!=Parameters::SUCCESS)
			{
                std::cerr << line << ": parser error" << std::endl;
                return FAIL;
			}
        }
		//
        return SUCCESS;
	}
}
#pragma once
#include "Config.h"

namespace Denn
{
//Abstract class args
class Arguments
{
public:

	virtual const char* get_string() = 0;
	virtual int get_int()			 = 0;
	virtual bool get_bool()			 = 0;
	virtual double get_double()		 = 0;
	
	virtual bool eof() const      = 0;
	virtual bool back()			  = 0;
	virtual bool end_vals() const = 0;

private:

	//default, none
	template <typename T> struct Helper
	{
		static T get(Arguments& args) { return T(); }
	};
	//vector type
	template <typename T> struct Helper< std::vector<T> >
	{
		static std::vector<T> get(Arguments& args)
		{
			std::vector< T > strs;
			while (!args.end_vals()) strs.push_back(args.get<T>());
			return strs;
		}
	};

public:

	template < class T > T get() { return Helper<T>::get(*this); }
};
//Stream of main input arguments
class MainArguments : public Arguments
{
public:

	MainArguments(int nargs, const char** vargs);

	const char* get_string() override;
	int get_int() override;
	bool get_bool() override;
	double get_double() override;

	bool eof() const override;
	bool back() override;
	bool end_vals() const override;

protected:

	int			 m_n_arg;
	int			 m_rem_arg;
	const char** m_pointer;

};
//Stream of inputs from a string
class StringArguments : public Arguments
{
public:
	using SkipSpace = std::function< void(StringArguments& self,const char*& ptr) >;
	using BackSkipSpace = std::function< void(StringArguments& self, const char* start_ptr, const char*& ptr) >;
	using InputPreProcessing = std::function< void (std::string& buffer) >;

	StringArguments(const char* values, const std::vector<char>& end_vals);
	StringArguments(SkipSpace skip_space, BackSkipSpace back_skip_space, const char* values, const std::vector<char>& end_vals);
	StringArguments(SkipSpace skip_space, BackSkipSpace back_skip_space, InputPreProcessing preprocess, const char* values, const std::vector<char>& end_vals);

	const char* get_string() override;
	int get_int() override;
	bool get_bool() override;
	double get_double() override;

	bool eof() const override;
	bool back() override;
	bool end_vals() const override;

	const char* get_ptr() const;

protected:

	SkipSpace		   m_skip_space;
	BackSkipSpace      m_back_skip_space;
	InputPreProcessing m_preprocess;
	std::vector<char>  m_end_vals;
	std::string		   m_buffer;
	const char*        m_values;
	const char*        m_values_start;

};
//CPP type argument
template <> struct Arguments::Helper< bool > { static bool get(Arguments& args) { return args.get_bool(); } };
template <> struct Arguments::Helper< char > { static char get(Arguments& args) { return args.get_string()[0]; } };
template <> struct Arguments::Helper< const char* > { static const char* get(Arguments& args) { return args.get_string(); } };
template <> struct Arguments::Helper< std::string > { static std::string get(Arguments& args) { return args.get_string(); } };

template <> struct Arguments::Helper< short > { static short get(Arguments& args) { return (short)args.get_int(); } };
template <> struct Arguments::Helper< int > { static int get(Arguments& args) { return args.get_int(); } };
template <> struct Arguments::Helper< long > { static long get(Arguments& args) { return args.get_int(); } };
template <> struct Arguments::Helper< long long > { static long long get(Arguments& args) { return args.get_int(); } };

template <> struct Arguments::Helper< unsigned short > { static unsigned short get(Arguments& args) { return (unsigned short)args.get_int(); } };
template <> struct Arguments::Helper< unsigned int > { static unsigned int get(Arguments& args) { return (unsigned int)args.get_int(); } };
template <> struct Arguments::Helper< unsigned long > { static unsigned long get(Arguments& args) { return (unsigned long)args.get_int(); } };
template <> struct Arguments::Helper< unsigned long long > { static unsigned long long get(Arguments& args) { return (unsigned long long)args.get_int(); } };

template <> struct Arguments::Helper< float > { static float get(Arguments& args) { return (float)args.get_double(); } };
template <> struct Arguments::Helper< double > { static double get(Arguments& args) { return args.get_double(); } };
template <> struct Arguments::Helper< long double > { static long double get(Arguments& args) { return args.get_double(); } };

}
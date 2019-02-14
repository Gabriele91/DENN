#pragma once
#include "Config.h"
#include "Arguments.h"
#include <string>
#include <vector>

namespace Denn
{
	//////////////////////////////////////////////
	class  Parameters;
	class  GenericReadOnly;
	struct ParameterDomain;
	struct ParameterOwners;
	struct ParameterInfo;
	//////////////////////////////////////////////
	class GenericReadOnly
	{
	public:
		virtual std::string name()  const = 0;
        virtual Variant variant()   const = 0;
        virtual VariantType type()  const = 0;
		virtual bool serializable() const = 0;
    
    protected:
        
        //set       
        virtual void* get_ptr() = 0;
        //get (no const) value
        template< typename T >
        T& get() { return *((T*)get_ptr()); }
        template< typename T >
        void set(const T& value) { get<T>() = value; }
        //friends
        friend class  Parameters;
        friend class  ParametersParseHelp;
        friend struct ParameterInfo;
	};

	template <class T>
	class ReadOnly : public GenericReadOnly
	{
	public:

		//cast
		template < class X >
		inline operator X() const { return X(m_data); }
		//no cast
		const T& operator *() const { return m_data; }
		//no cast
		const T& get() const { return m_data; }
		//get variant
		virtual Variant variant() const override { return get(); }
        //type
        virtual VariantType type() const override { return static_variant_type<T>(); }
		//get name
		virtual std::string name() const  override { return m_name; }
		//get if serializable
		virtual bool serializable() const override { return m_serializable; }

	private:


		//init
		ReadOnly() {}
		ReadOnly(const std::string& name) { m_name = name; }
		//init + value
		template < typename I >
		ReadOnly(const std::string& name, const I& arg, bool serializable = true)
		{
			m_name = name;
			m_data = T(arg);
			m_serializable = serializable;
		}
		//set value
		template < typename I >
		T operator=(const I& arg)
		{
			m_data = T(arg);
			return m_data;
		}
		//get (no const) value
		T& get() { return m_data; }
        //set
        virtual void* get_ptr() override { return &m_data; }
		//name/data/serializable
		std::string m_name;
		T m_data;
		bool m_serializable{ true };
		//friends
		friend class  Parameters;
		friend class  ParametersParseHelp;
		friend struct ParameterInfo;

	};

	struct ParameterDomain
	{
		enum TypeDomain
		{
			NONE,
			MONO,
			CHOISE,
		};
		TypeDomain  m_type{ NONE };
		std::string m_domain{ "void" };
		std::vector< std::string > m_choises;

		ParameterDomain();
		ParameterDomain(const std::string& domain);
		ParameterDomain(const std::string& domain, const std::vector< std::string >& choises);
	};

	struct ParameterOwners
	{
		enum Filter
		{
			NONE,
			ALL,
			EXCEPT,
			ONLY
		};

		struct ParameterOwner
		{
			const Filter			     m_filter_type;
			const std::vector< Variant > m_filter_values;
			const GenericReadOnly*       m_owner_variable;

			ParameterOwner();

			bool test(const ParameterInfo& owner) const;

			template< class T >
			ParameterOwner(const ReadOnly< T >& owner)
			: m_filter_type(ALL)
			, m_owner_variable(&owner)
			{

			}

			template< class T >
			ParameterOwner(const ReadOnly< T >& owner, const std::vector< Variant >& value)
			: m_filter_type(ONLY)
			, m_filter_values(value)
			, m_owner_variable(&owner)
			{

			}

			template< class T >
			ParameterOwner(const ReadOnly< T >& owner, const Filter filter, const std::vector< Variant >& value)
			: m_filter_type(filter)
			, m_filter_values(value)
			, m_owner_variable(&owner)
			{

			}
		};

		ParameterOwners();

		ParameterOwners(const std::vector < ParameterOwner >& owners);

		bool test(const ParameterInfo& owner) const;

		template< class T >
		ParameterOwners(const ReadOnly< T >& owner)
		{
			m_owners.emplace_back(owner);
		}

		template< class T >
		ParameterOwners(const ReadOnly< T >& owner, const std::vector< Variant >& value)
		{
			m_owners.emplace_back(owner, value);
		}

		template< class T >
		ParameterOwners(const ReadOnly< T >& owner, const Filter filter, const std::vector< Variant >& value)
		{
			m_owners.emplace_back(owner, filter, value);
		}
	
	protected:

		std::vector < ParameterOwner > m_owners;

	};

	struct ParameterInfo
	{

		const GenericReadOnly*		      m_associated_variable;
		const ParameterOwners			  m_oweners;
		std::string				   		  m_description;
		std::vector< std::string  > 	  m_arg_key;
		std::function< bool(Arguments&) > m_action;
		ParameterDomain                   m_domain;
		
		ParameterInfo();

		template< class T >
		ParameterInfo
		(
			  ReadOnly< T >&			          associated_variable
			, const std::string&				  description
			, const std::vector< std::string  >&  arg_key
			, std::function< bool(Arguments&) >   action = nullptr
			, const ParameterDomain&              domain = ParameterDomain(cpp_type_to_arg_type<T>())
		)
		: m_associated_variable(&associated_variable)
		, m_description(description)
		, m_arg_key(arg_key)
		, m_domain(domain)
		{
			m_action = action ? action : [&](Arguments& args) -> bool { associated_variable = args.get<T>(); return true; };
			m_arg_key.insert(m_arg_key.begin(), "--" + associated_variable.name());
		}

		template< class T >
		ParameterInfo
		(
			  ReadOnly< T >&			          associated_variable
			, const ParameterOwners			      owener
			, const std::string&				  description
			, const std::vector< std::string  >&  arg_key
			, std::function< bool(Arguments&) >   action = nullptr
			, const ParameterDomain&              domain = ParameterDomain(cpp_type_to_arg_type<T>())
		)
			: m_associated_variable(&associated_variable)
			, m_oweners(owener)
			, m_description(description)
			, m_arg_key(arg_key)
			, m_domain(domain)
		{
			m_action = action ? action : [&](Arguments& args) -> bool { associated_variable = args.get<T>(); return true; };
			m_arg_key.insert(m_arg_key.begin(), "--" + associated_variable.name());
		}

		ParameterInfo
		(
			  const std::string&				   	description
			, const std::vector< std::string  >& 	arg_key
			, std::function< bool(Arguments&) >     action
			, const ParameterDomain&                domain = ParameterDomain(std::string())
		);

		ParameterInfo
		(
			  const std::string&				  description
			, const ParameterOwners			      owener
			, const std::vector< std::string  >&  arg_key
			, std::function< bool(Arguments&) >   action
			, const ParameterDomain&              domain = ParameterDomain(std::string())
		);

		bool has_an_associated_variable() const;

		bool serializable() const;

	protected:

		template< class T > static const char* cpp_type_to_arg_type() { return "void"; }

	};
	//////////////////////////////////////////////
	//CPP type to str param
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<bool>() { return "bool"; }

	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<char>() { return "string"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< const char* >() { return "string"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::string >() { return "string"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector< std::string > >() { return "list(string)"; }

	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<short>() { return "int"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<int>() { return "int"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<long>() { return "int"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<long long>() { return "int"; }

	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<unsigned short>() { return "uint"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<unsigned int>() { return "uint"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<unsigned long>() { return "uint"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<unsigned long long>() { return "uint"; }

	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<float>() { return "float"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<double>() { return "float"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<long double>() { return "float"; }

	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<MatrixF>() { return "matrix"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<MatrixD>() { return "matrix"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type<MatrixLD>() { return "matrix"; }

	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<short> >() { return "list(int)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<int> >() { return "list(int)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<long> >() { return "list(int)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<long long> >() { return "list(int)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<unsigned short> >() { return "list(uint)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<unsigned int> >() { return "list(uint)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<unsigned long> >() { return "list(uint)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<unsigned long long> >() { return "list(uint)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<float> >() { return "list(float)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<double> >() { return "list(float)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<long double> >() { return "list(float)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<MatrixF> >() { return "list(matrix)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<MatrixD> >() { return "list(matrix)"; }
	template<> inline const char* ParameterInfo::cpp_type_to_arg_type< std::vector<MatrixLD> >() { return "list(matrix)"; }
	//////////////////////////////////////////////
}
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Scalar.h"
#include "EigenAlias.h"

namespace Denn
{
	//denn types
	class Individual;
	class Population;
	class SubPopulation;

	//variant types
	class Variant;
	class VariantRef;

	enum VariantType
	{
		VR_NONE,
		VR_BOOL,
		VR_CHAR,
		VR_SHORT,
		VR_INT,
		VR_LONG,
		VR_LONGLONG,

		VR_UCHAR,
		VR_USHORT,
		VR_UINT,
		VR_ULONG,
		VR_ULONGLONG,

		VR_FLOAT,
		VR_DOUBLE,
		VR_LONG_DOUBLE,



		//HEAP
		VR_FLOAT_MATRIX,
		VR_DOUBLE_MATRIX,
		VR_LONG_DOUBLE_MATRIX,

		VR_INDIVIDUAL,
		VR_INDIVIDUAL_LIST,
		VR_SUB_POPULATION,
		VR_POPULATION,

		VR_STD_VECTOR_SHORT,
		VR_STD_VECTOR_INT,
		VR_STD_VECTOR_LONG,
		VR_STD_VECTOR_LONGLONG,
		VR_STD_VECTOR_USHORT,
		VR_STD_VECTOR_UINT,
		VR_STD_VECTOR_ULONG,
		VR_STD_VECTOR_ULONGLONG,
		VR_STD_VECTOR_FLOAT,
		VR_STD_VECTOR_DOUBLE,
		VR_STD_VECTOR_LONG_DOUBLE,
		VR_STD_VECTOR_FLOAT_MATRIX,
		VR_STD_VECTOR_DOUBLE_MATRIX,
		VR_STD_VECTOR_LONG_DOUBLE_MATRIX,

		VR_C_STRING,
		VR_STD_STRING,
		VR_STD_VECTOR_STRING,

		//PTR
		VR_PTR
	};

	//get type
	template < class T > inline VariantType static_variant_type() { return VR_NONE; };
	//template specialization 
	template <> inline VariantType static_variant_type<bool>() { return VR_BOOL; };
	template <> inline VariantType static_variant_type<char>() { return VR_CHAR; };
	template <> inline VariantType static_variant_type<short>() { return VR_SHORT; };
	template <> inline VariantType static_variant_type<int>() { return VR_INT; };
	template <> inline VariantType static_variant_type<long>() { return VR_LONG; };
	template <> inline VariantType static_variant_type<long long>() { return VR_LONGLONG; };

	template <> inline VariantType static_variant_type<unsigned char>() { return VR_UCHAR; };
	template <> inline VariantType static_variant_type<unsigned short>() { return VR_USHORT; };
	template <> inline VariantType static_variant_type<unsigned int>() { return VR_UINT; };
	template <> inline VariantType static_variant_type<unsigned long>() { return VR_ULONG; };
	template <> inline VariantType static_variant_type<unsigned long long>() { return VR_ULONGLONG; };

	template <> inline VariantType static_variant_type<float>() { return VR_FLOAT; };
	template <> inline VariantType static_variant_type<double>() { return VR_DOUBLE; };
	template <> inline VariantType static_variant_type<long double>() { return VR_LONG_DOUBLE; };

	template <> inline VariantType static_variant_type<MatrixF>() { return VR_FLOAT_MATRIX; };
	template <> inline VariantType static_variant_type<MatrixD>() { return VR_DOUBLE_MATRIX; };
	template <> inline VariantType static_variant_type<MatrixLD>() { return VR_LONG_DOUBLE_MATRIX; };

	template <> inline VariantType static_variant_type<Individual>()     { return VR_INDIVIDUAL; };
	template <> inline VariantType static_variant_type<std::vector< std::shared_ptr< Individual > >>() { return VR_INDIVIDUAL_LIST; };
	template <> inline VariantType static_variant_type<SubPopulation>()  { return VR_SUB_POPULATION; };
	template <> inline VariantType static_variant_type<Population>()     { return VR_POPULATION; };

	template <> inline VariantType static_variant_type< std::vector< short > >() { return VR_STD_VECTOR_SHORT; };
	template <> inline VariantType static_variant_type< std::vector< int > >() { return VR_STD_VECTOR_INT; };
	template <> inline VariantType static_variant_type< std::vector< long > >() { return VR_STD_VECTOR_LONG; };
	template <> inline VariantType static_variant_type< std::vector< long long > >() { return VR_STD_VECTOR_LONGLONG; };

	template <> inline VariantType static_variant_type< std::vector< unsigned short > >() { return VR_STD_VECTOR_USHORT; };
	template <> inline VariantType static_variant_type< std::vector< unsigned int > >() { return VR_STD_VECTOR_UINT; };
	template <> inline VariantType static_variant_type< std::vector< unsigned long > >() { return VR_STD_VECTOR_ULONG; };
	template <> inline VariantType static_variant_type< std::vector< unsigned long long > >() { return VR_STD_VECTOR_ULONGLONG; };

	template <> inline VariantType static_variant_type< std::vector< float > >() { return VR_STD_VECTOR_FLOAT; };
	template <> inline VariantType static_variant_type< std::vector< double > >() { return VR_STD_VECTOR_DOUBLE; };
	template <> inline VariantType static_variant_type< std::vector< long double > >() { return VR_STD_VECTOR_LONG_DOUBLE; };
	template <> inline VariantType static_variant_type< std::vector< MatrixF > >()  { return VR_STD_VECTOR_FLOAT_MATRIX; };
	template <> inline VariantType static_variant_type< std::vector< MatrixD > >()  { return VR_STD_VECTOR_DOUBLE_MATRIX; };
	template <> inline VariantType static_variant_type< std::vector< MatrixLD > >() { return VR_STD_VECTOR_LONG_DOUBLE_MATRIX; }

	template <> inline VariantType static_variant_type<const char*>() { return VR_C_STRING; };
	template <> inline VariantType static_variant_type<std::string>() { return VR_STD_STRING; };
	template <> inline VariantType static_variant_type< std::vector< std::string > >() { return VR_STD_VECTOR_STRING; };

	template <> inline VariantType static_variant_type<void*>() { return VR_PTR; };

	class Variant
	{
	public:

		Variant();
		
		~Variant();

		Variant(VariantType type);

		Variant(const Variant& in);

		Variant(bool b);

		Variant(char c);

		Variant(short s);

		Variant(int i);

		Variant(long l);

		Variant(long long l);

		Variant(unsigned char uc);

		Variant(unsigned short us);

		Variant(unsigned int ui);

		Variant(unsigned long ul);

		Variant(unsigned long long ull);

		Variant(float f);

		Variant(double d);
		
		Variant(long double ld);

		Variant(const MatrixF& fm);

		Variant(const MatrixD& dm);

		Variant(const MatrixLD& ldm);

		Variant(const Individual& pop);

		Variant(const std::vector< std::shared_ptr< Individual > >& pop);

		Variant(const Population& pop);

		Variant(const SubPopulation& pop);

		Variant(const std::vector< short > & v_s);

		Variant(const std::vector< int > & v_i);

		Variant(const std::vector< long > & v_l);

		Variant(const std::vector< long long > & v_ll);

		Variant(const std::vector< unsigned short > & v_us);

		Variant(const std::vector< unsigned int > & v_ui);

		Variant(const std::vector< unsigned long > & v_ul);

		Variant(const std::vector< unsigned long long > & v_ull);

		Variant(const std::vector< float > & v_f);

		Variant(const std::vector< double > & v_d);

		Variant(const std::vector< long double > & v_ld);

		Variant(const std::vector< MatrixF > & v_v2);

		Variant(const std::vector< MatrixD > & v_v3);

		Variant(const std::vector< MatrixLD > & v_v4);

		Variant(const char* c_str);

		Variant(const std::string& str);

		Variant(const std::vector< std::string > & v_str);

		Variant(void* ptr);

		Variant(const VariantRef& ref);

		//cast objects
		template < class T >
		T& get()
		{
			if (is_heap_value()) return *((T*)m_ptr);
			else				return *((T*)&m_ptr);
		}
		template < class T >
		const T& get() const
		{
			if (is_heap_value()) return *((T*)m_ptr);
			else				return *((T*)&m_ptr);
		}
		template < class T >
		explicit  operator const T& () const
		{
			if (is_heap_value()) return *((T*)m_ptr);
			else				 return *((T*)&m_ptr);
		}

		//alloc_cast
		template < class T >
		T& get_alloc()
		{
			if (static_variant_type<T>() != m_type)
			{
				if (!is_heap_value())
				{
					std::memset(this, 0, sizeof(Variant));
					m_type = VR_NONE;
				}
				set_type(static_variant_type<T>());
			}
			return *((T*)&m_ptr);
		}

		//type
		VariantType get_type() const;

		bool equal (const Variant& right, bool case_sensitive = true) const;

		bool operator == (const Variant& right) const;

		bool operator != (const Variant& right) const;
		//query
		bool is_none() const;

		bool is_null() const;

		bool is_heap_value() const;

		//get reference pointer
		void* get_ref() const;

		//copy from other variant
		void copy_from(const Variant& in);

	private:

		//native type
		union alignas(16)
		{
			bool       m_b;
			char       m_c;
			short      m_s;
			int        m_i;
			long       m_l;
			long long  m_ll;

			unsigned char       m_uc;
			unsigned short      m_us;
			unsigned int        m_ui;
			unsigned long       m_ul;
			unsigned long long  m_ull;

			float  m_f;
			double m_d;
			long double m_ld;

			void* m_ptr;
		};
		//save type
		VariantType m_type{ VR_NONE };
		//set type
		void set_type(VariantType type);
	};

	class VariantRef
	{
	public:

		VariantRef();

		VariantRef(const bool& b);

		VariantRef(const char& c);

		VariantRef(const short& s);

		VariantRef(const int& i);

		VariantRef(const long& l);

		VariantRef(const long long& l);

		VariantRef(const unsigned char& uc);

		VariantRef(const unsigned short& us);

		VariantRef(const unsigned int& ui);

		VariantRef(const unsigned long& ul);

		VariantRef(const unsigned long long& ull);

		VariantRef(const float& f);

		VariantRef(const double& d);

		VariantRef(const long double& ld);

		VariantRef(const MatrixF& fm);

		VariantRef(const MatrixD& dm);

		VariantRef(const MatrixLD& ldm);

		VariantRef(const Individual& i);

		VariantRef(const std::vector< std::shared_ptr< Individual > >& i);

		VariantRef(const Population& pop);
		
		VariantRef(const SubPopulation& pop);

		VariantRef(const std::vector< short > & v_s);

		VariantRef(const std::vector< int > & v_i);

		VariantRef(const std::vector< long > & v_l);

		VariantRef(const std::vector< long long > & v_ll);	
		
		VariantRef(const std::vector< unsigned short > & v_us);

		VariantRef(const std::vector< unsigned int > & v_ui);

		VariantRef(const std::vector< unsigned long > & v_ul);

		VariantRef(const std::vector< unsigned long long > & v_ull);

		VariantRef(const std::vector< float > & v_f);

		VariantRef(const std::vector< double > & v_d);

		VariantRef(const std::vector< long double > & v_ld);

		VariantRef(const std::vector< MatrixF > & v_fm);

		VariantRef(const std::vector< MatrixD > & v_dm);

		VariantRef(const std::vector< MatrixLD > & v_dm);

		VariantRef(const char*& c_str);

		VariantRef(const std::string& str);

		VariantRef(const std::vector< std::string >& v_str);

		VariantRef(const void* ptr);

		//only if is explicit (shadowing the others constructors)
		explicit VariantRef(const Variant& var);

		template < class T >
		T& get()
		{
			return *((T*)m_ptr);
		}

		template < class T >
		const T& get() const
		{
			return *((T*)m_ptr);
		}


		template < class T >
		explicit  operator const T& () const
		{
			return *((T*)m_ptr);
		}

		VariantType get_type() const
		{
			return m_type;
		}

		template < class T >
		T* get_ptr()
		{
			return (T*)m_ptr;
		}

		template < class T >
		const T* get_ptr() const
		{
			return (T*)m_ptr;
		}

	private:

		void*		 m_ptr;
		VariantType m_type;

	};


}
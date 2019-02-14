#pragma once

namespace Denn
{
	#if defined(USE_LONG_DOUBLE)
	using Scalar	 = long double;
	#elif defined(USE_DOUBLE)
	using Scalar	 = double;
	#else
	using Scalar	 = float;
	#endif

	enum class DataType
	{
		DT_UNKNOWN      =-1,
		DT_FLOAT        = 1,
		DT_DOUBLE       = 2,
		DT_LONG_DOUBLE  = 3 
	};

	template < typename ScalarType >
	inline DataType get_data_type(){ return DataType::DT_UNKNOWN; }
	template <>
	inline DataType get_data_type<float>(){ return DataType::DT_FLOAT; }
	template <>
	inline DataType get_data_type<double>(){ return DataType::DT_DOUBLE; }
	template <>
	inline DataType get_data_type<long double>(){ return DataType::DT_LONG_DOUBLE; }

}
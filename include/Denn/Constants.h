#pragma once
#include "Config.h"

namespace Denn
{
namespace Constants
{

	template < typename ScalarType = Scalar >
	constexpr inline ScalarType pi()
	{
		return ScalarType(3.141592653589793238462643383279502884L);
	}

	template < typename ScalarType = Scalar >
	constexpr inline ScalarType two_pi()
	{
		return ScalarType(6.28318530717958647692528676655900576L);
	}

	template < typename ScalarType = Scalar >
	constexpr inline ScalarType e()
	{
		return ScalarType(std::exp(ScalarType(1))); //ScalarType(2.71828182845904523536);
	}

	template < typename ScalarType = Scalar >
	constexpr inline ScalarType epsilon()
	{
		return std::numeric_limits<ScalarType>::epsilon();
	}

	template  < typename ScalarType = Scalar >
	constexpr inline  ScalarType golden_ratio()
	{
		return ScalarType(1.61803398874989484820458683436563811772031L);
	}
}
}
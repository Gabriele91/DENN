#pragma once
//Math
namespace Denn
{
	template<typename T, class Compare>
	inline constexpr T clamp(const T& value, const T& lo, const T& hi, Compare comp)
	{
		return denn_assert(!comp(hi, lo)), comp(value, lo) ? lo : comp(hi, value) ? hi : value;
	}

	template< typename T >
	inline constexpr T clamp(const T& value, const T& lo, const T& hi)
	{
		return clamp(value, lo, hi, std::less<void>());
	}	

	template< typename T >
	inline constexpr T sature(const T& v)
	{
		return clamp< T >(v, 0.0, 1.0);
	}

	template < typename T, typename S >
	inline constexpr T lerp(const T& a, const T& b, const S& alpha)
	{
		return a * ( T(1.0) - alpha ) + b * alpha;
	}
	
	template < typename T >
	inline constexpr T positive_mod(const T& value,const T& base)
	{
		return (((value % base) + base) % base);
	}
	
	template < typename T >
	inline constexpr T positive_fmod(const T& value,const T& base)
	{
		//from JS
		return std::fmod((std::fmod(value, base) + base), base);
	}

	
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

	template  < typename ScalarType = Scalar >
	constexpr inline  ScalarType golden_ratio()
	{
		return ScalarType(1.61803398874989484820458683436563811772031L);
	}
}

namespace Activation
{
	template < typename ScalarType = double >
	inline ScalarType sigmoid(const ScalarType& a)
	{			
		//(k = 1, x0 = 0, L = 1)
		return ScalarType(1.0) / (ScalarType(1.0) + std::exp(-a));
	}
	template < typename ScalarType = double >
	inline ScalarType sigmoid_derivative(const ScalarType& a)
	{
		ScalarType sigmoid_a = sigmoid(a);
		return sigmoid_a * (ScalarType(1.0) - sigmoid_a);
	}

	template < typename ScalarType = double >
	inline ScalarType logistic(const ScalarType& k, const ScalarType& a, const ScalarType& a0)
	{
		return ScalarType(1.0) / (ScalarType(1.0) + std::exp(-k*(a-a0)));
	}
	template < typename ScalarType = double >
	inline ScalarType logistic_derivative(const ScalarType& k, const ScalarType& a, const ScalarType& a0)
	{
		return logistic(k, a, a0) * (ScalarType(1.0) - logistic(k, a, a0));
	}
	
	template < typename ScalarType = double >
	inline ScalarType log(const ScalarType& a)
	{
		return std::log(a);
	}
	template < typename ScalarType = double >
	inline ScalarType log_derivative(const ScalarType& a)
	{
		return ScalarType(1.0) / a;
	}

	template < typename ScalarType = double >
	inline ScalarType logit(const ScalarType& a)
	{
		return log<ScalarType>(a / (ScalarType(1.0) - ScalarType(a)));
	}
	template < typename ScalarType = double >
	inline ScalarType logit_derivative(const ScalarType& a)
	{
		return ScalarType(1.0) / (a * (ScalarType(1.0) - ScalarType(a)));
	}

	template < typename ScalarType = double >
	inline ScalarType tanh(const ScalarType& a)
	{
		return std::tanh(a);
	}
	template < typename ScalarType = double >
	inline ScalarType tanh_derivative(const ScalarType& a)
	{
		return ScalarType(1.0) - std::pow(std::tanh(a), ScalarType(2.0));
	}

	template < typename ScalarType = double >
	inline ScalarType relu(const ScalarType& a)
	{
		return std::max(a, ScalarType(0));
	}
	template < typename ScalarType = double >
	inline ScalarType relu_derivative(const ScalarType& a)
	{
		return a < ScalarType(0) ? ScalarType(0) : ScalarType(1.0);
	}
}
}

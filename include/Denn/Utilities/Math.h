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
}

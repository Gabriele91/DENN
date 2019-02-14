//String
namespace Denn
{
	template <typename T, typename Compare>
	inline std::vector<size_t> sort_permutation( const std::vector<T>& vec, Compare compare)
	{
		std::vector<size_t> p(vec.size());
		std::iota(p.begin(), p.end(), 0);
		std::sort(p.begin(), p.end(), [&](size_t i, size_t j) -> bool { return compare(vec[i], vec[j]); });
		return p;
	}

	template <typename T>
	inline std::vector<T> apply_permutation(const std::vector<T>& vec, const std::vector<std::size_t>& p)
	{
		std::vector<T> sorted_vec(vec.size());
		std::transform(p.begin(), p.end(), sorted_vec.begin(), [&](size_t i){ return vec[i]; });
		return sorted_vec;
	}

	template<typename T>
	inline void apply_permutation_in_place(std::vector<T>& v,  std::vector<size_t>& p)
	{	
		for (size_t i = 0; i < p.size(); i++) 
		{
			auto current = i;
			while (i != p[current])
			{
				auto next = p[current];
				std::swap(v[current], v[next]);
				p[current] = current;
				current = next;
			}
			p[current] = current;
		}
	}
}
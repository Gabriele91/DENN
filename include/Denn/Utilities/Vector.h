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
	
	
	template <typename T>
	inline std::vector<size_t> sort_indexes(const std::vector<T> &v)
	{
		// initialize original index locations
		std::vector<size_t> idx(v.size());
		std::iota(idx.begin(), idx.end(), 0);

		// sort indexes based on comparing values in v
		std::sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

		return idx;
	}
	
	template <typename T>
	inline std::vector<size_t>& sort_indexes(const std::vector<T> &v,  std::vector<size_t>& idx)
	{
		// initialize original index locations
		idx.resize(v.size());
		std::iota(idx.begin(), idx.end(), 0);

		// sort indexes based on comparing values in v
		std::sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

		return idx;
	}

	template <typename T>
	inline std::vector<size_t>& sort_indexes(Eigen::ArrayWrapper<T> &v,  std::vector<size_t>& idx)
	{
		// initialize original index locations
		idx.resize(v.size());
		std::iota(idx.begin(), idx.end(), 0);

		// sort indexes based on comparing values in v
		std::sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {return v(i1) < v(i2);});

		return idx;
	}
}
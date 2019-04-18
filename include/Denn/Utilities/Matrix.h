//Matrix
namespace Denn
{
	/// Shifts a matrix/vector row-wise.
	/// A negative \a down value is taken to mean shifting up.
	/// When passed zero for \a down, the input matrix is returned unchanged.
	/// The type \a Matrix can be either a fixed- or dynamically-sized matrix.
	template < typename Matrix >
	inline Matrix shift_by_rows(const Matrix& in, typename Matrix::Index down)
	{
		//no swift
		if (!down) return in;
		//result
		Matrix out(in.rows(), in.cols());
		//mod
		if (down > 0) down = down % in.rows();
		else down = in.rows() - (-down % in.rows());
		int rest = in.rows() - down;
		//shif
		out.topRows(down)	 = in.bottomRows(down);
		out.bottomRows(rest) = in.topRows(rest);
		//return
		return out;
	}

	/// Shifts a matrix/vector row-wise.
	/// A negative \a down value is taken to mean shifting up.
	/// When passed zero for \a down, the input matrix is returned unchanged.
	/// The type \a Matrix can be either a fixed- or dynamically-sized matrix.
	template < typename Matrix >
	inline Matrix shift_by_cols(const Matrix& in, typename Matrix::Index down)
	{
		//no swift
		if (!down) return in;
		//result
		Matrix out(in.rows(), in.cols());
		//mod
		if (down > 0) down = down % in.cols();
		else down = in.cols() - (-down % in.cols());
		int rest = in.cols() - down;
		//shif
		out.leftCols(down) = in.rightCols(down);
		out.rightCols(rest) = in.leftCols(rest);
		//return
		return out;
	}

	template < typename Matrix >
	inline Matrix shift_bottom(const Matrix& in, unsigned int down)
	{
		return shift_by_rows(in, int(down));
	}

	template < typename Matrix >
	inline Matrix shift_top(const Matrix& in, unsigned int top)
	{
		return shift_by_rows(in, -1*int(top));
	}


	template < typename Matrix >
	inline Matrix shift_right(const Matrix& in, unsigned int down)
	{
		return shift_by_cols(in, int(down));
	}

	template < typename Matrix >
	inline Matrix shift_left(const Matrix& in, unsigned int top)
	{
		return shift_by_cols(in, -1 * int(top));
	}

	template < typename Matrix >
	inline bool append_rows(Matrix& matrix, const Matrix& rows_to_append)
	{
		if (matrix.cols() != rows_to_append.cols()) return false;
		//alloc
		matrix.conservativeResize(matrix.rows() + rows_to_append.rows(), matrix.cols());
		//copy
		matrix.bottomRows(rows_to_append.rows()) = rows_to_append;
		//return
		return true;
	}

	template < typename MatrixA, typename MatrixB >
	inline typename Matrix::Scalar distance_pow2(MatrixA& a, MatrixB& b)
	{
		return (a.array() - b.array()).square().sum();
	} 

	template < typename MatrixA, typename MatrixB >
	inline typename Matrix::Scalar distance(MatrixA& a, MatrixB& b)
	{
		return std::sqrt(distance_pow2(a,b));
	}

	template < typename Matrix >
	void sort_ascending(Matrix& m) 
	{
  	std::sort(m.derived().data(), m.derived().data() + m.derived().size());
	}

	template < typename Matrix >
	void sort_rows_ascending(Matrix& m)
	{
		m.transposeInPlace();
		for (size_t r = 0; r < m.cols(); ++r)
			std::sort(m.col(r).data(), m.col(r).data() + m.col(r).size());
		m.transposeInPlace();
	}

	template < typename Matrix >
	void sort_descending(Matrix& m) 
	{
  	std::sort(m.derived().data(), m.derived().data() + m.derived().size());
		std::reverse(m.derived().data(), m.derived().data() + m.derived().size());
	}

	template < typename Matrix >
	void sort_rows_descending(Matrix& m)
	{
		m.transposeInPlace();
		for (size_t r = 0; r < m.cols(); ++r)
		{
			std::sort(m.col(r).data(), m.col(r).data() + m.col(r).size());
			std::reverse(m.col(r).data(), m.col(r).data() + m.col(r).size());
		}
		m.transposeInPlace();
	}
	
}

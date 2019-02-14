#pragma once
#include "Config.h"
#include "PointFunction.h"

namespace Denn
{
namespace CostFunction
{

	template < typename Scalar >
	Scalar safe_log(Scalar x)
	{
		const Scalar min_const_ep = 1e-10 ;
		return Denn::PointFunction::log<Scalar>(Denn::clamp(x,min_const_ep,Scalar(1.0)));
	}
	template < typename Scalar >
	Scalar neg_safe_log(Scalar x)
	{
		const Scalar min_const_ep = 1e-10 ;
		return -Denn::PointFunction::log<Scalar>(Denn::clamp(x,min_const_ep,Scalar(1.0)));
	}
	template < typename Scalar >
	Scalar inverse_probability(Scalar x)
	{
		return Scalar(1.0) - x;
	}

	template < typename Matrix >
	Matrix cross_entropy_by_rows(const Matrix& x, const Matrix& y)
	{
		Matrix log_y = y.unaryExpr(&safe_log<typename Matrix::Scalar>);
		return -((x.array() * log_y.array()).matrix().rowwise()).sum();
	}	

	template < typename Matrix >
	typename Matrix::Scalar cross_entropy(const Matrix& x, const Matrix& y)
	{
		Matrix log_y = y.unaryExpr(&safe_log<typename Matrix::Scalar>);
		return -(x.array() * log_y.array()).sum() / (typename Matrix::Scalar)y.rows();
	}
    
    template < typename Matrix >
    Matrix cross_entropy_logistic_by_rows (const Matrix& labels, const Matrix& predict)
    {
        using Scalar = typename Matrix::Scalar ;
        Matrix evalout(labels.rows(),labels.cols());
        for(typename Matrix::Index x = 0; x!=labels.cols(); ++x)
        for(typename Matrix::Index y = 0; y!=labels.rows(); ++y)
        {
            Scalar log_y       = safe_log<typename Matrix::Scalar>(predict(y,x));
            Scalar log_one_m_y = safe_log<typename Matrix::Scalar>(Scalar(1.0)-predict(y,x));
            evalout(y,x)       = (labels(y,x) * log_y + (Scalar(1.0)-labels(y,x)) * (log_one_m_y));
        }
        return -(evalout.rowwise().mean());
	}
	
    template < typename Matrix >
    Matrix cross_entropy_logistic_by_cols (const Matrix& labels, const Matrix& predict)
    {
        using Scalar = typename Matrix::Scalar ;
        Matrix evalout(labels.rows(),labels.cols());
        for(typename Matrix::Index x = 0; x!=labels.cols(); ++x)
        for(typename Matrix::Index y = 0; y!=labels.rows(); ++y)
        {
            Scalar log_y       = safe_log<typename Matrix::Scalar>(predict(y,x));
            Scalar log_one_m_y = safe_log<typename Matrix::Scalar>(Scalar(1.0)-predict(y,x));
            evalout(y,x)       = (labels(y,x) * log_y + (Scalar(1.0)-labels(y,x)) * (log_one_m_y));
        }
        return -(evalout.colwise().mean());
    }
    
	template < typename Matrix >
	typename Matrix::Scalar cross_entropy_logistic_regression (const Matrix& labels, const Matrix& predict)
	{
		using Scalar = typename Matrix::Scalar ;
		Matrix evalout(labels.rows(),labels.cols()); 
		for(typename Matrix::Index x = 0; x!=labels.cols(); ++x)
		for(typename Matrix::Index y = 0; y!=labels.rows(); ++y)
		{
			Scalar log_y       = safe_log<typename Matrix::Scalar>(predict(y,x));
			Scalar log_one_m_y = safe_log<typename Matrix::Scalar>(Scalar(1.0)-predict(y,x));
			evalout(y,x)       = (labels(y,x) * log_y + (Scalar(1.0)-labels(y,x)) * (log_one_m_y));
		}
		return (-(evalout.rowwise().mean())).mean();
	}
	

	template < typename Matrix >
	inline Matrix& softmax(Matrix& inout_matrix)
	{
		const auto N   = inout_matrix.rows();
		const auto max = inout_matrix.maxCoeff();
		//compute e^(M-max)
		inout_matrix = (inout_matrix.array() - max).exp();
		// M(r,n)/ SUM_r(M(n))
		for (int n = 0; n < N; n++)
		{
			//reduce
			auto sum = inout_matrix.row(n).sum();
			//no nan
			if (sum) inout_matrix.row(n) /= sum;
		}
		return inout_matrix;
	}

	template < typename Scalar >
	inline ColVectorT< Scalar >& softmax_col(ColVectorT< Scalar >& inout_col)
	{
		const auto max = inout_col.maxCoeff();
		//compute e^(M-max)
		inout_col = (inout_col.array() - max).exp();
		//reduce
		auto sum = inout_col.sum();
		//no nan
		if (sum) inout_col /= sum;
		//return
		return inout_col;
	}

	template < typename Scalar >
	inline RowVectorT< Scalar >& softmax_row(RowVectorT< Scalar >& inout_row)
	{
		const auto max = inout_row.maxCoeff();
		//compute e^(M-max)
		inout_row = (inout_row.array() - max).exp();
		//reduce
		auto sum = inout_row.sum();
		//no nan
		if (sum) inout_row /= sum;
		//return
		return inout_row;
	}
	
	template < typename Matrix >
	inline Matrix& implace_softmax_col_samples(Matrix& inout_matrix)
	{
		const auto N = inout_matrix.cols();
		// for each examples
		for (int n = 0; n < N; n++)
		{
			const auto max = inout_matrix.col(n).maxCoeff();
			//compute e^(M-max)
			inout_matrix.col(n) = (inout_matrix.col(n).array() - max).exp();
			//reduce
			auto sum = inout_matrix.col(n).sum();
			//no nan
			if (sum) inout_matrix.col(n) /= sum;
		}
		//return
		return inout_matrix;
	}

	template < typename Matrix >
	inline Matrix& implace_softmax_row_samples(Matrix& inout_matrix)
	{
		const auto N = inout_matrix.rows();
		// for each examples
		for (int n = 0; n < N; n++)
		{
			const auto max = inout_matrix.row(n).maxCoeff();
			//compute e^(M-max)
			inout_matrix.row(n) = (inout_matrix.row(n).array() - max).exp();
			//reduce
			auto sum = inout_matrix.row(n).sum();
			//no nan
			if (sum) inout_matrix.row(n) /= sum;
		}
		//return
		return inout_matrix;
	}	

	template < typename Matrix >
	inline Matrix softmax_col_samples(const Matrix& in)
	{
		Matrix in_softmax = in;
		in_softmax = implace_softmax_col_samples(in_softmax);
		return in_softmax;
	}

	template < typename Matrix >
	inline Matrix softmax_row_samples(const Matrix& in)
	{
		Matrix in_softmax = in;
		in_softmax = implace_softmax_row_samples(in_softmax);
		return in_softmax;
	}

	template < typename Matrix >
	typename Matrix::Scalar softmax_cross_entropy(const Matrix& x, const Matrix& y)
	{
		Matrix y_softmax(y);
		softmax(y_softmax);
		return cross_entropy(x, y_softmax);
	}

	template < typename Matrix >
	typename Matrix::Scalar softmax_cross_entropy_with_logit(const Matrix& x, const Matrix& y)
	{
		Matrix y_softmax(y);
		softmax(y_softmax);
		return cross_entropy_logistic_regression(x, y_softmax);
	}

	template < typename Matrix >
	Matrix rows_argmax(const Matrix& x, const Matrix& y)
	{
		//output
		Matrix output(x.rows(), 1);
		////values
		typename Matrix::Index  max_index, i;
		//max-max
		for (typename Matrix::Index j = 0; j < x.rows(); ++j)
		{
			x.row(j).maxCoeff(&i, &max_index);
			output(j) = Matrix::Scalar(max_index);
		}
		//
		return output;
	}

	template < typename Matrix >
	Matrix correct_prediction(const Matrix& x, const Matrix& y)
	{
		//output
		Matrix output(x.rows(),1);
		////values
		typename Matrix::Index  max_index_x, max_index_y, i;
		//max-max
		for (typename Matrix::Index j = 0; j < x.rows(); ++j)
		{
			x.row(j).maxCoeff(&i, &max_index_x);
			y.row(j).maxCoeff(&i, &max_index_y);
			output(j) = typename Matrix::Scalar(max_index_x == max_index_y);
		}
		//
		return output;
	}

	template < typename Matrix >
	typename Matrix::Scalar accuracy(const Matrix& x, const Matrix& y)
	{
		#if 0
		return correct_prediction <Matrix>(x, y).mean();
		#else
		using Scalar = typename Matrix::Scalar;
		//output
		Scalar output = Scalar(0.0);;
		////values
		typename Matrix::Index  max_index_x, max_index_y, i;
		//max-max
		for (typename Matrix::Index j = 0; j < x.rows(); ++j)
		{
			x.row(j).maxCoeff(&i, &max_index_x);
			y.row(j).maxCoeff(&i, &max_index_y);
			output += Scalar(max_index_x == max_index_y);
		}
		//
		return output/Scalar(x.rows());
		#endif
	}
	template < typename Matrix >
	typename Matrix::Scalar accuracy_cols(const Matrix& x, const Matrix& y)
	{
		using Index = typename Matrix::Index;
		using Scalar = typename Matrix::Scalar;
		//output
		Scalar output = Scalar(0.0);
		//values
		Index  max_index_x, max_index_y;
		//max-max
		for (Index j = 0; j < x.cols(); ++j)
		{
			x.col(j).maxCoeff(&max_index_x);
			y.col(j).maxCoeff(&max_index_y);
			output += Scalar(max_index_x == max_index_y);
		}
		//
		return output / Scalar(x.cols());
	}

	template < typename Matrix >
	typename Matrix::Scalar inverse_accuracy(const Matrix& x, const Matrix& y)
	{
		using Scalar = typename Matrix::Scalar;
		return Scalar(1.0) - accuracy<Matrix>(x,y);
	}

	template < typename Matrix >
	typename Matrix::Scalar inverse_accuracy_cols(const Matrix& x, const Matrix& y)
	{
		using Scalar = typename Matrix::Scalar;
		return Scalar(1.0) - accuracy_cols<Matrix>(x, y);
	}
}
}

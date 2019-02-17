#pragma once
#include <Eigen/Eigen>
#include "Scalar.h"

namespace Denn 
{

	template < typename T >
	using RowArrayT = Eigen::Array<T, 1, Eigen::Dynamic>;
	using RowArrayLD = Eigen::Array<long double, 1, Eigen::Dynamic>;
	using RowArrayD = Eigen::Array<double, 1, Eigen::Dynamic>;
	using RowArrayF = Eigen::Array<float, 1, Eigen::Dynamic>;
	using RowArray = Eigen::Array<Scalar, 1, Eigen::Dynamic>;

	template < typename T >
	using ColArrayT = Eigen::Array<T, Eigen::Dynamic, 1>;
	using ColArrayLD = Eigen::Array<long double, Eigen::Dynamic, 1>;
	using ColArrayD = Eigen::Array<double, Eigen::Dynamic, 1>;
	using ColArrayF = Eigen::Array<float, Eigen::Dynamic, 1>;
	using ColArray = Eigen::Array<Scalar, Eigen::Dynamic, 1>;

	template < typename T >
	using RowVectorT  = typename Eigen::Matrix<T, 1, Eigen::Dynamic>;
	using RowVectorLD = typename Eigen::Matrix<long double, 1, Eigen::Dynamic>;
	using RowVectorD  = typename Eigen::Matrix<double, 1, Eigen::Dynamic>;
	using RowVectorF  = typename Eigen::Matrix<float,  1, Eigen::Dynamic>;
	using RowVector   = typename Eigen::Matrix<Scalar,  1, Eigen::Dynamic>;

	template < typename T >
	using ColVectorT  = typename Eigen::Matrix<T, Eigen::Dynamic, 1>;
	using ColVectorLD = typename Eigen::Matrix<long double, Eigen::Dynamic, 1>;
	using ColVectorD  = typename Eigen::Matrix<double, Eigen::Dynamic, 1>;
	using ColVectorF  = typename Eigen::Matrix<float,  Eigen::Dynamic, 1>;
	using ColVector   = typename Eigen::Matrix<Scalar,  Eigen::Dynamic, 1>;

	template < typename T >
	using MatrixT	 = typename Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
	using MatrixLD	 = typename Eigen::Matrix<long double, Eigen::Dynamic, Eigen::Dynamic>;
	using MatrixD	 = typename Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;
	using MatrixF	 = typename Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;
	using Matrix 	 = typename Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic > ;

	template < typename T >
	using MatrixListT  = std::vector < MatrixT< T > >;
	using MatrixListF  = std::vector < MatrixF >;
	using MatrixListD  = std::vector < MatrixD >;
	using MatrixListLD = std::vector < MatrixLD >;
	using MatrixList   = std::vector < Matrix >;


	/* Map Version */
	template < typename T >
	using MapRowVectorT  = typename Eigen::Map< RowVectorT<T> >;
	using MapRowVectorLD = typename Eigen::Map< RowVectorLD >;
	using MapRowVectorD  = typename Eigen::Map< RowVectorD >;
	using MapRowVectorF  = typename Eigen::Map< RowVectorF >;
	using MapRowVector   = typename Eigen::Map< RowVector >;

	template < typename T >
	using MapColVectorT  = typename Eigen::Map< ColVectorT<T> >;
	using MapColVectorLD = typename Eigen::Map< ColVectorLD >;
	using MapColVectorD  = typename Eigen::Map< ColVectorD >;
	using MapColVectorF  = typename Eigen::Map< ColVectorF >;
	using MapColVector   = typename Eigen::Map< ColVector >;

	template < typename T >
	using MapMatrixT	 = typename Eigen::Map< MatrixT<T> >;
	using MapMatrixLD	 = typename Eigen::Map< MatrixLD >;
	using MapMatrixD	 = typename Eigen::Map< MatrixD >;
	using MapMatrixF	 = typename Eigen::Map< MatrixF >;
	using MapMatrix 	 = typename Eigen::Map< Matrix > ;

	template < typename T >
	using MapMatrixListT  = std::vector < MapMatrixT< T > >;
	using MapMatrixListF  = std::vector < MapMatrixF >;
	using MapMatrixListD  = std::vector < MapMatrixD >;
	using MapMatrixListLD = std::vector < MapMatrixLD >;
	using MapMatrixList   = std::vector < MapMatrix >;
	   
	//rows to map
	template < typename T >
	inline MapRowVectorT< T > as_map(RowVectorT< T >& value)
	{ return MapRowVectorT< T >(value.data(), value.size()); }
	inline MapRowVectorLD as_map(RowVectorLD& value)
	{ return MapRowVectorLD(value.data(), value.size()); }
	inline MapRowVectorD as_map(RowVectorD& value)
	{ return MapRowVectorD(value.data(), value.size()); }
	inline MapRowVectorF as_map(RowVectorF& value)
	{ return MapRowVectorF(value.data(), value.size()); }

	//cols to map
	template < typename T >
	inline MapColVectorT< T > as_map(ColVectorT< T >& value)
	{ return MapColVectorT< T >(value.data(), value.size()); }
	inline MapColVectorLD as_map(ColVectorLD& value)
	{ return MapColVectorLD(value.data(), value.size()); }
	inline MapColVectorD as_map(ColVectorD& value)
	{ return MapColVectorD(value.data(), value.size()); }
	inline MapColVectorF as_map(ColVectorF& value)
	{ return MapColVectorF(value.data(), value.size()); }

	//matrix to map
	template < typename T >
	inline MapMatrixT< T > as_map(MatrixT< T >& value)
	{ return MapMatrixT< T >(value.data(), value.rows(), value.cols()); }
	inline MapMatrixLD as_map(MatrixLD& value)
	{ return MapMatrixLD(value.data(), value.rows(), value.cols()); }
	inline MapMatrixD as_map(MatrixD& value)
	{ return MapMatrixD(value.data(), value.rows(), value.cols()); }
	inline MapMatrixF as_map(MatrixF& value)
	{ return MapMatrixF(value.data(), value.rows(), value.cols()); }

	/*Aligned map version */
	template < typename T >
	using AlignedMapMatrixT = typename MatrixT<T>::AlignedMapType;
	using AlignedMapMatrixLD = typename MatrixLD::AlignedMapType;
	using AlignedMapMatrixD = typename MatrixD::AlignedMapType;
	using AlignedMapMatrixF = typename MatrixF::AlignedMapType;
	using AlignedMapMatrix = typename Matrix::AlignedMapType;

	template < typename T >
	using AlignedMapRowVectorT = typename RowVectorT<T>::AlignedMapType;
	using AlignedMapRowVectorLD = typename RowVectorLD::AlignedMapType;
	using AlignedMapRowVectorD = typename RowVectorD::AlignedMapType;
	using AlignedMapRowVectorF = typename RowVectorF::AlignedMapType;
	using AlignedMapRowVector = typename RowVector::AlignedMapType;

	template < typename T >
	using AlignedMapColVectorT = typename ColVectorT<T>::AlignedMapType;
	using AlignedMapColVectorLD = typename ColVectorLD::AlignedMapType;
	using AlignedMapColVectorD = typename ColVectorD::AlignedMapType;
	using AlignedMapColVectorF = typename ColVectorF::AlignedMapType;
	using AlignedMapColVector = typename ColVector::AlignedMapType;

	template < typename T >
	using ConstAlignedMapMatrixT = typename MatrixT<T>::ConstAlignedMapType;
	using ConstAlignedMapMatrixLD = typename MatrixLD::ConstAlignedMapType;
	using ConstAlignedMapMatrixD = typename MatrixD::ConstAlignedMapType;
	using ConstAlignedMapMatrixF = typename MatrixF::ConstAlignedMapType;
	using ConstAlignedMapMatrix = typename Matrix::ConstAlignedMapType;

	template < typename T >
	using ConstAlignedMapRowVectorT = typename RowVectorT<T>::ConstAlignedMapType;
	using ConstAlignedMapRowVectorLD = typename RowVectorLD::ConstAlignedMapType;
	using ConstAlignedMapRowVectorD = typename RowVectorD::ConstAlignedMapType;
	using ConstAlignedMapRowVectorF = typename RowVectorF::ConstAlignedMapType;
	using ConstAlignedMapRowVector = typename RowVector::ConstAlignedMapType;

	template < typename T >
	using ConstAlignedMapColVectorT = typename ColVectorT<T>::ConstAlignedMapType;
	using ConstAlignedMapColVectorLD = typename ColVectorLD::ConstAlignedMapType;
	using ConstAlignedMapColVectorD = typename ColVectorD::ConstAlignedMapType;
	using ConstAlignedMapColVectorF = typename ColVectorF::ConstAlignedMapType;
	using ConstAlignedMapColVector = typename ColVector::ConstAlignedMapType;

	//Alios ref
	template < typename PlainObjectType >
	using RefT = Eigen::Ref< PlainObjectType >;
	template < typename PlainObjectType, int Options, typename StrideType >
	using RefTSride = Eigen::Ref< PlainObjectType, Options, StrideType >;

	//alias
	using RefColVectorLD = RefT< ColVectorLD >;
	using RefColVectorD = RefT< ColVectorD >;
	using RefColVectorF = RefT< ColVectorF >;
	using RefColVector = RefT< ColVector >;

	using RefConstColVectorLD = RefT<const ColVectorLD >;
	using RefConstColVectorD = RefT<const ColVectorD >;
	using RefConstColVectorF = RefT<const ColVectorF >;
	using RefConstColVector = RefT<const ColVector >;

	using RefRowVectorLD = RefTSride< RowVectorLD, 0, Eigen::InnerStride<> >;
	using RefRowVectorD = RefTSride< RowVectorD, 0, Eigen::InnerStride<> >;
	using RefRowVectorF = RefTSride< RowVectorF, 0, Eigen::InnerStride<> >;
	using RefRowVector = RefTSride< RowVector, 0, Eigen::InnerStride<> >;

	using RefConstRowVectorLD = RefTSride<const RowVectorLD, 0, Eigen::InnerStride<> >;
	using RefConstRowVectorD = RefTSride<const RowVectorD, 0, Eigen::InnerStride<> >;
	using RefConstRowVectorF = RefTSride<const RowVectorF, 0, Eigen::InnerStride<> >;
	using RefConstRowVector = RefTSride<const RowVector, 0, Eigen::InnerStride<> >;

	using RefMatrixLD = RefT< MatrixLD >;
	using RefMatrixD = RefT< MatrixD >;
	using RefMatrixF = RefT< MatrixF >;
	using RefMatrix = RefT< Matrix >;

	using RefConstMatrixLD = RefT<const MatrixLD >;
	using RefConstMatrixD = RefT<const MatrixD >;
	using RefConstMatrixF = RefT<const MatrixF >;
	using RefConstMatrix = RefT<const Matrix >;
}
#pragma once
#include "Denn/Config.h"
#include "Denn/Shape.h"


namespace Denn
{
	class DataSet
	{
	public:
		virtual void* ptr_features() const = 0;
		virtual void* ptr_labels()   const = 0;

		virtual void* data_features() const = 0;
		virtual void* data_labels()   const = 0;

		virtual const Shape& features_shape() const = 0;
		virtual const Shape& labels_shape() const = 0;
		
		virtual DataType get_data_type() const { return DataType::DT_UNKNOWN; }

		//auto cast
		template<class T =  Scalar>
		const Denn::MatrixT<T>&  features() const
		{
			return *((const Denn::MatrixT<T>*)(ptr_features()));
		}
		template<class T = Scalar>
		const Denn::MatrixT<T>&  labels() const
		{
			return *((const Denn::MatrixT<T>*)(ptr_labels()));
		}
		template<class T = Scalar>
		Denn::MatrixT<T>& features()
		{
			return *((Denn::MatrixT<T>*)(ptr_features()));
		}
		template<class T = Scalar>
		Denn::MatrixT<T>& labels()
		{
			return *((Denn::MatrixT<T>*)(ptr_labels()));
		}

		template < typename ScalarType = Scalar >
		Denn::MatrixT< ScalarType >&  features_as_type()
		{
			return *((Denn::MatrixT< ScalarType >*)(ptr_features()));
		}

		template < typename ScalarType = Scalar >
		Denn::MatrixT< ScalarType >&  labels_as_type()
		{
			return *((Denn::MatrixT< ScalarType >*)(ptr_labels()));
		}

	};

	template < typename ScalarType >
	class DataSetX : public DataSet
	{
	public:
		Denn::MatrixT< ScalarType > m_features;
		Denn::MatrixT< ScalarType > m_labels;
		Shape m_features_shape;
		Shape m_labels_shape;

		inline DataSetX<ScalarType>() {};

		Denn::MatrixT< ScalarType >&  features() { return m_features; }
		Denn::MatrixT< ScalarType >&  labels() { return m_labels; }
		
		const Denn::MatrixT< ScalarType >&  features() const { return m_features; }
		const Denn::MatrixT< ScalarType >&  labels() const { return m_labels; }

		virtual void* ptr_features()  const override { return (void*)&m_features; }
		virtual void* ptr_labels()	  const override { return (void*)&m_labels; }

		virtual void* data_features() const override { return (void*)m_features.data(); }
		virtual void* data_labels()   const override { return (void*)m_labels.data(); }

		virtual const Shape& features_shape() const override { return m_features_shape; }
		virtual const Shape& labels_shape() const override { return m_labels_shape; }

		virtual DataType get_data_type() const override { return Denn::get_data_type<ScalarType>(); }
	};

	using DataSetScalar = DataSetX<Scalar>;
	using DataSetF = DataSetX<float>;
	using DataSetD = DataSetX<double>;
}
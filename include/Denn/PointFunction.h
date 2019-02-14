#pragma once 
#include "Config.h"
#include "Constants.h"

namespace Denn
{
	namespace PointFunction
	{
		template < typename ScalarType >
		using Ptr = std::function< ScalarType(const ScalarType& input) >;

		template < typename ScalarType = double >
		inline ScalarType identity(const ScalarType& a)
		{
			return a;
		}
		template < typename ScalarType = double >
		inline ScalarType dx_identity(const ScalarType& a)
		{
			return ScalarType(1);
		}

		template < typename ScalarType = double >
		inline ScalarType linear(const ScalarType& a)
		{
			return a;
		}
		template < typename ScalarType = double >
		inline ScalarType dx_linear(const ScalarType& a)
		{
			return ScalarType(1);
		}

		template < typename ScalarType = double >
		inline ScalarType sigmoid(const ScalarType& a)
		{			
			//(k = 1, x0 = 0, L = 1)
			return ScalarType(1.0) / (ScalarType(1.0) + std::exp(-a));
		}
		template < typename ScalarType = double >
		inline ScalarType dx_sigmoid(const ScalarType& a)
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
		inline ScalarType dx_logistic(const ScalarType& k, const ScalarType& a, const ScalarType& a0)
		{
			return logistic(k, a, a0) * (ScalarType(1.0) - logistic(k, a, a0));
		}
		
		template < typename ScalarType = double >
		inline ScalarType log(const ScalarType& a)
		{
			return std::log(a);
		}
		template < typename ScalarType = double >
		inline ScalarType dx_log(const ScalarType& a)
		{
			return ScalarType(1.0) / a;
		}

		template < typename ScalarType = double >
		inline ScalarType logit(const ScalarType& a)
		{
			return log<ScalarType>(a / (ScalarType(1.0) - ScalarType(a)));
		}
		template < typename ScalarType = double >
		inline ScalarType dx_logit(const ScalarType& a)
		{
			return ScalarType(1.0) / (a * (ScalarType(1.0) - ScalarType(a)));
		}

		template < typename ScalarType = double >
		inline ScalarType tanh(const ScalarType& a)
		{
			return std::tanh(a);
		}
		template < typename ScalarType = double >
		inline ScalarType dx_tanh(const ScalarType& a)
		{
			return ScalarType(1.0) - std::pow(std::tanh(a), ScalarType(2.0));
		}

		template < typename ScalarType = double >
		inline ScalarType relu(const ScalarType& a)
		{
			return std::max(a, ScalarType(0));
		}
		template < typename ScalarType = double >
		inline ScalarType dx_relu(const ScalarType& a)
		{
			return a < ScalarType(0) ? ScalarType(0) : ScalarType(1.0);
		}

		template < typename ScalarType = double >
		inline ScalarType binary(const ScalarType& a)
		{
			return (a < ScalarType(0) ?  ScalarType(0) :  ScalarType(1));
		}
	}
}

#pragma once
#include "Denn/Config.h"
#include "Denn/Layer.h"

namespace Denn
{
	class Sigmoid : public ActivationLayer
	{
	public:
		Sigmoid(const Shape& shape, const Inputs& inputs) : ActivationLayer("sigmoid", shape, shape) {}

		virtual Layer::SPtr copy() const override
		{
			return std::static_pointer_cast<Layer>(std::make_shared<Sigmoid>(*this));
		}

		virtual const Matrix& feedforward(const Matrix& bottom) override
		{
			m_top = Scalar(1.0) / (Scalar(1.0) + (-bottom).array().exp());
			//return feed
			return m_top;
		}
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override
		{
			// d(L)/d(z_i) = d(L)/d(a_i) * d(a_i)/d(z_i)
			// d(a_i)/d(z_i) = a_i * (1-a_i)
			Matrix da_dz = m_top.array().cwiseProduct(Scalar(1.0) - m_top.array());
			m_grad_bottom = grad.cwiseProduct(da_dz);
			//return grad
			return m_grad_bottom;
		}
	};
	REGISTERED_ACTIVATION_LAYER(Sigmoid, LAYER_NAMES("sigmoid", "sd"))

	class ReLU : public ActivationLayer
	{
	public:
		ReLU(const Shape& shape, const Inputs& inputs) : ActivationLayer("relu", shape, shape) {}

		virtual Layer::SPtr copy() const override
		{
			return std::static_pointer_cast<Layer>(std::make_shared<ReLU>(*this));
		}

		virtual const Matrix& feedforward(const Matrix& bottom) override
		{  
			// a = z*(z>0)
			m_top = bottom.cwiseMax(0.0);
			//return feed
			return m_top;
		}
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override
		{
			Matrix positive = (bottom.array() > 0.0).cast<Scalar>();
			m_grad_bottom = grad.cwiseProduct(positive);
			//return grad
			return m_grad_bottom;
		}
	};
	REGISTERED_ACTIVATION_LAYER(ReLU, LAYER_NAMES("relu"))

	class Softmax : public ActivationLayer
	{
	public:
		Softmax(const Shape& shape, const Inputs& inputs) : ActivationLayer("softmax", shape, shape) {}

		virtual Layer::SPtr copy() const override
		{
			return std::static_pointer_cast<Layer>(std::make_shared<Softmax>(*this));
		}

		virtual const Matrix& feedforward(const Matrix& bottom) override
		{ 
			// a = exp(z) / \sum{ exp(z) }
			m_top = (bottom.rowwise() - bottom.colwise().maxCoeff()).array().exp();
			RowArray z_exp_sum = m_top.colwise().sum();  // \sum{ exp(z) }
			m_top.array().rowwise() /= z_exp_sum;
			//return feed
			return m_top;
		}
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override
		{
			// d(L)/d(z_i) = \sum{ d(L)/d(a_j) * d(a_j)/d(z_i) }
			// = \sum_(i!=j){ d(L)/d(a_j) * d(a_j)/d(z_i) } + d(L)/d(a_i) * d(a_i)/d(z_i)
			// = a_i * ( d(L)/d(a_i) - \sum{a_j * d(L)/d(a_j)} )
			RowArray temp_sum = m_top.cwiseProduct(grad).colwise().sum();
			m_grad_bottom = m_top.array().cwiseProduct(grad.array().rowwise() - temp_sum);
			//return grad
			return m_grad_bottom;
		}
	};
	REGISTERED_ACTIVATION_LAYER(Softmax, LAYER_NAMES("softmax", "sm"))

}
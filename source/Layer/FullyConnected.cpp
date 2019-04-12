#include "Denn/Layer/FullyConnected.h"

namespace Denn
{
	///////////////////////////////////////	
	FullyConnected::FullyConnected
	(
		  int features
		, int clazz
	)
	: DerivableLayer("fully_connected",{ features }, { clazz })
	{
		//weight
		m_weight.resize(int(this->m_in_size), int(this->m_out_size));
		m_weight.setZero();
		m_bias.resize(int(this->m_out_size));
		m_bias.setZero();
		//derivate
		CODE_BACKPROPAGATION(
			m_grad_w.resize(int(this->m_in_size), int(this->m_out_size));
			m_grad_w.setZero();
			m_grad_b.resize(int(this->m_out_size));
			m_grad_b.setZero();
		)
	}
	FullyConnected::FullyConnected
	(
		  const Shape& in
		, const Inputs& metadata
	)
	: FullyConnected(in.size3D(), metadata[0])
	{
	}
	//////////////////////////////////////////////////
	const Inputs FullyConnected::inputs() const
	{
		return make_inputs<int>({ out_size().width() });
	}
	//////////////////////////////////////////////////
	Layer::SPtr FullyConnected::copy() const
	{
		return std::static_pointer_cast<Layer>(std::make_shared<FullyConnected>(*this));
	}

	//////////////////////////////////////////////////
	const Matrix& FullyConnected::predict(const Matrix& bottom)
	{
		return feedforward(bottom);
	}
	const Matrix& FullyConnected::feedforward(const Matrix& bottom)
	{
		const int n_sample = bottom.cols();
		// top = w' * x + b
		m_top.resize(int(out_size()), n_sample);
		m_top.noalias() = m_weight.transpose() * bottom;
		m_top.colwise() += m_bias;
		//return value
		return m_top;
	}
	const Matrix&  FullyConnected::backpropagate(const Matrix& bottom, const Matrix& grad)
    {
		CODE_BACKPROPAGATION(
			const int n_sample = bottom.cols();
			// d(L)/d(w') = d(L)/d(z) * x'
			// d(L)/d(b) = \sum{ d(L)/d(z_i) }
			m_grad_w = bottom * grad.transpose();
			m_grad_b = grad.rowwise().sum();
			// Compute d(L) / d_in = W * [d(L) / d(z)]
			m_grad_bottom.resize(int(in_size()), n_sample);
			m_grad_bottom.noalias() = m_weight * grad;
			//return gradient
		)
		RETURN_BACKPROPAGATION(m_grad_bottom);
    }
	void FullyConnected::update(const Optimizer& optimize)
	{
		CODE_BACKPROPAGATION(
			AlignedMapColVector  w(m_weight.data(), m_weight.size());
			AlignedMapColVector  b(m_bias.data(), m_bias.size());
			ConstAlignedMapColVector dw(m_grad_w.data(), m_grad_w.size());
			ConstAlignedMapColVector db(m_grad_b.data(), m_grad_b.size());

			optimize.update(w, dw);
			optimize.update(b, db);
		)
		BACKPROPAGATION_ASSERT
	}
    //////////////////////////////////////////////////
	size_t FullyConnected::size() const
	{
		return 2;
	}
	AlignedMapMatrix FullyConnected::operator[](size_t i)
	{
		denn_assert(i < 2);
		switch (i)
		{
		default:
		case 0: return  AlignedMapMatrix(m_weight.data(), m_weight.rows(), m_weight.cols());
		case 1: return  AlignedMapMatrix(m_bias.data(), m_bias.rows(), m_bias.cols());
		}
	}
	ConstAlignedMapMatrix FullyConnected::operator[](size_t i) const
	{
		denn_assert(i < 2);
		switch (i)
		{
		default:
		case 0: return  ConstAlignedMapMatrix(m_weight.data(), m_weight.rows(), m_weight.cols());
		case 1: return  ConstAlignedMapMatrix(m_bias.data(), m_bias.rows(), m_bias.cols());
		}
	}
	//////////////////////////////////////////////////
}

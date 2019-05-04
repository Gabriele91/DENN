#pragma once
#include "Denn/Config.h"
#include "Denn/Layer.h"
#include "Denn/Utilities/Convolution.h"

namespace Denn
{

	class FullyConnected : public DerivableLayer
	{
	public:
		///////////////////////////////////////
		/// FullyConnected
		///
		/// \param features      Width of the input.
		/// \param clazz         Width of the output.
		///
		FullyConnected
		(
			  int features
			, int clazz
		);
		FullyConnected
		(
			  const Shape& in
			, const Inputs& metadata
		);
		//////////////////////////////////////////////////
		virtual Layer::SPtr copy() const override;
		//////////////////////////////////////////////////
		virtual const Inputs inputs() const override;
		//////////////////////////////////////////////////
		virtual const Matrix& predict(const Matrix& input) override;
		virtual const Matrix& feedforward(const Matrix& input) override;
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override;
		//////////////////////////////////////////////////
		virtual void update(const Optimizer& optimize) override;
		//////////////////////////////////////////////////
		virtual size_t size() const operator_override;
		virtual AlignedMapMatrix      operator[](size_t i) operator_override;
		virtual ConstAlignedMapMatrix operator[](size_t i) const operator_override;
		//////////////////////////////////////////////////
	protected:    
		//weight
		Matrix m_weight;  // Weight parameters, W(in_size x out_size)
		ColVector m_bias; // Bias parameters, b(out_size x 1)
		//backpropagation
		CODE_BACKPROPAGATION(
			Matrix m_grad_w;    // Derivative of weights
			ColVector m_grad_b; // Derivative of bias
		)
	};

	REGISTERED_LAYER(
		FullyConnected,
		LAYER_NAMES("fully_connected", "fc"),
		LayerShapeType(SHAPE_1D),		//shape type
		LayerDescription::MinMax { 1 },	//1 argument
	)
}
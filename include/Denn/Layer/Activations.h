#pragma once
#include "Denn/Config.h"
#include "Denn/Layer.h"

namespace Denn
{
	class Sigmoid : public ActivationLayer
	{
	public:
		Sigmoid(const Shape& shape, const Inputs& inputs);
		virtual Layer::SPtr copy() const override;
		virtual const Matrix& feedforward(const Matrix& bottom) override;
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override;
	};
	REGISTERED_ACTIVATION_LAYER(Sigmoid, LAYER_NAMES("sigmoid", "sd"))

	class ReLU : public ActivationLayer
	{
	public:
		ReLU(const Shape& shape, const Inputs& inputs);
		virtual Layer::SPtr copy() const override;
		virtual const Matrix& feedforward(const Matrix& bottom) override;
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override;
	};
	REGISTERED_ACTIVATION_LAYER(ReLU, LAYER_NAMES("relu"))

	class LeakyReLU : public ActivationLayer
	{
	public:
		LeakyReLU(const Shape& shape, const Inputs& inputs);
		virtual Layer::SPtr copy() const override;
		virtual const Matrix& feedforward(const Matrix& bottom) override;
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override;
	protected:
		Scalar m_alpha;
	};
	REGISTERED_ACTIVATION_LAYER_WITH_INPUTS(LeakyReLU, LAYER_NAMES("leaky_relu"), LayerDescription::MinMax{ 0,1 })

	class Softmax : public ActivationLayer
	{
	public:
		Softmax(const Shape& shape, const Inputs& inputs);
		virtual Layer::SPtr copy() const override;
		virtual const Matrix& feedforward(const Matrix& bottom) override;
		virtual const Matrix& backpropagate(const Matrix& bottom, const Matrix& grad) override;
	};
	REGISTERED_ACTIVATION_LAYER(Softmax, LAYER_NAMES("softmax", "sm"))

}
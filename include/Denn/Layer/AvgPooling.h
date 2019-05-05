#pragma once
#include "Denn/Config.h"
#include "Denn/NeuralNetwork/Layer.h"
#include "Denn/Utilities/Convolution.h"

namespace Denn
{
	class AvgPooling : public PoolingLayer
	{
	public:
		///////////////////////////////////////
		/// Constructor
		///
		/// \param in_width      Width of the input image in each channel.
		/// \param in_height     Height of the input image in each channel.
		/// \param in_channels   Number of input channels.
		/// \param window_width  Width of the filter.
		/// \param window_height Height of the filter.
		/// \param stride        Kernel stride (x, y).
		///
		AvgPooling 
		(
			  int in_width, int in_height, int in_channels
			, int window_width, int window_height, int stride = 1
		);
		AvgPooling 
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
		virtual const Matrix& backpropagate(const Matrix& prev_layer_data, const Matrix& next_layer_data) override;
		//////////////////////////////////////////////////
		virtual void update(const Optimizer& optimize) override;
		//////////////////////////////////////////////////
	protected:    
		//shape conv
		internal::ConvDims m_dim;		           // dimensions of convolution
	};

	REGISTERED_LAYER(
		AvgPooling,
		LAYER_NAMES("avg_pooling", "avgp"),
		LayerShapeType(SHAPE_2D_3D),    //shape 2D/3D
		LayerDescription::MinMax{ 2,3 } //min args filter size, max args filter size + stride
	)
}
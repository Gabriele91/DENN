#pragma once
#include "Denn/Config.h"
#include "Denn/Layer.h"
#include "Denn/Utilities/Convolution.h"

namespace Denn
{
	class MaxPooling : public PoolingLayer
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
		MaxPooling 
		(
			  int in_width, int in_height, int in_channels
			, int window_width, int window_height, int stride = 1
		);
		MaxPooling 
		(
			  const Shape& in
			, const std::vector<int>& metadata
		);
		//////////////////////////////////////////////////
		virtual Layer::SPtr copy() const override;
		//////////////////////////////////////////////////
		virtual const Inputs inputs() const override;
		//////////////////////////////////////////////////
		virtual const Matrix& feedforward(const Matrix& input) override;
		virtual const Matrix& backpropagate(const Matrix& prev_layer_data, const Matrix& next_layer_data) override;
		//////////////////////////////////////////////////
		virtual void update(const Optimizer& optimize) override;
		//////////////////////////////////////////////////
	protected:    
		// dimensions of convolution
		internal::ConvDims m_dim;		         
 		// index of max values
		CODE_BACKPROPAGATION(
			std::vector<std::vector<int> > m_max_idxs;
		)
	};

	REGISTERED_LAYER(
		MaxPooling,
		LAYER_NAMES("max_pooling", "maxp"),
		LayerShapeType(SHAPE_2D_3D),    //shape 2D/3D
		LayerDescription::MinMax{ 2,3 } //min args filter size, max args filter size + stride
	)
}
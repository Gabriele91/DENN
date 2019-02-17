#pragma once
#include "Denn/Config.h"
#include "Denn/Layer.h"
#include "Denn/Utilities/Convolution.h"

namespace Denn
{

	class Convolutional : public DerivableLayer
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
		/// \param out_channels  Number of output channels.
		///
		Convolutional
		(
			  int in_width, int in_height, int in_channels
			, int window_width, int window_height, int out_channels = 1
			, int stride = 1, int pad_w = 0, int pad_h = 0
		);
		Convolutional
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
		virtual size_t size() const operator_override;
		virtual AlignedMapMatrix      operator[](size_t i) operator_override;
		virtual ConstAlignedMapMatrix operator[](size_t i) const operator_override;
		//////////////////////////////////////////////////
	protected:    
		//shape conv
		internal::ConvDims m_dim;		// dimensions of convolution

		//weight
		Matrix  m_kernels;		// Kernels parameters. Total length is
								// (in_channels x out_channels x filter_rows x filter_cols)
								// See Utils/Convolution.h for its layout
		ColVector m_bias;	    // Bias term for the output channels, out_channels x 1. (One bias term per channel)
		
		//backpropagation
		CODE_BACKPROPAGATION(
			std::vector<Matrix> m_images;// Save input inmage in ff fase
			Matrix 	m_grad_kernels; 	 // Derivative of filters, same dimension as m_filter_data
			ColVector m_grad_bias;		 // Derivative of bias, same dimension as m_bias
		)
	};

	REGISTERED_LAYER(
		Convolutional,
		LAYER_NAMES("convolutional", "conv"),
		LayerShapeType(SHAPE_2D_3D),    //shape 2D/3D
		LayerDescription::MinMax{ 2,6 } //min args filter size, max args filter size + channels
	)
}
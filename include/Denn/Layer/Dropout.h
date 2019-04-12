#pragma once
#include "Denn/Config.h"
#include "Denn/Layer.h"
#include "Denn/Utilities/Convolution.h"

namespace Denn
{
	class Dropout : public PoolingLayer //like a pooling layer
	{
	public:
		///////////////////////////////////////
		/// Constructor
		///
		/// \param dropout       Probability of drop a connection
		/// \param in_width      Width of the input image in each channel.
		/// \param in_height     Height of the input image in each channel.
		/// \param in_channels   Number of input channels.
		///
		Dropout 
		(
              float dropout
            , int in_width
            , int in_height = 1
            , int in_channels = 1
		);
		Dropout 
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
        // probability
        double m_probability;		         
 		// mask
        Matrix m_mask;
	};

	REGISTERED_LAYER(
		Dropout,
		LAYER_NAMES("dropout", "dout"),
		LayerShapeType(SHAPE_1D_2D_3D),  //shape 1D/2D/3D
		LayerDescription::MinMax{ 1 }    //dropout probability
	)
}
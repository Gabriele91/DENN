#include "Denn/Layer/Dropout.h"
#include "Denn/Core/Random.h"
#define _metadata(_x, _default) (_x < metadata.size() ? metadata[_x] : _default)
#define _shapepooling(in, kernel, stride) (1 + std::ceil((in - kernel) / stride))

namespace Denn
{
    Dropout::Dropout 
    (
      float dropout,
      int in_width,
      int in_height,
      int in_channels
    )
	: PoolingLayer
    (
        "dropout",
	    Shape(in_width, in_height, in_channels),
	    Shape(in_width, in_height, in_channels)
    )
    {
    }
    Dropout::Dropout 
    (
      const Shape& in
    , const std::vector<int>& metadata
    )
	: Dropout
	(
	//inputs
	metadata[0] / 100.0f,
	//shapes
	in.width(), in.height(), in.channels()
    )
    {
    }
    //////////////////////////////////////////////////
	Layer::SPtr Dropout::copy() const
	{
		return std::static_pointer_cast<Layer>(std::make_shared<Dropout>(*this));
	}
	const Inputs Dropout::inputs() const
	{
        if(in_size().is1D())
		    return { in_size().width() };
        else if(in_size().is2D())
		    return { in_size().width(),  in_size().height() };
        else //if(in_size().is3D())
		    return { in_size().width(),  in_size().height(), in_size().channels() };
	}	
    //////////////////////////////////////////////////
    const Matrix& Dropout::predict(const Matrix& bottom)
    {
        m_top = bottom;
        return m_top;
    }
    const Matrix& Dropout::feedforward(const Matrix& bottom)
    {
        //init nois vector
        m_mask.resize(bottom.rows(),bottom.cols());
        //Random engine
        thread_local Random random; //todo, pass as argument
        //random
        m_mask = m_mask.unaryExpr([&,this](const Scalar& x) -> Scalar  { 
            return random.uniform() < m_probability ? Scalar(0) : Scalar(1); 
        });
        //apply
        m_top = (bottom.array() * m_mask.array()).matrix();
        //return 
        return m_top;
    }
    
    const Matrix& Dropout::backpropagate(const Matrix& bottom, const Matrix& grad) 
    {
        CODE_BACKPROPAGATION(
            m_grad_bottom = (grad.array() * m_mask.array()).matrix();
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    
	void Dropout::update(const Optimizer& optimize)
	{
        /* none */
	}
    //////////////////////////////////////////////////
}
#include "Denn/Layer/AvgPooling.h"
#define _metadata(_x, _default) (_x < metadata.size() ? metadata[_x] : _default)
#define _shapepooling(in, kernel, stride) (1 + std::ceil((in - kernel) / stride))

namespace Denn
{
    AvgPooling::AvgPooling 
    (
      int in_width, int in_height, int in_channels
    , int window_width, int window_height, int stride
    )
	: PoolingLayer("max_pooling",
		Shape(in_width, in_height, in_channels),
		Shape(_shapepooling(in_width, window_width, stride), 
			  _shapepooling(in_height, window_height, stride),
			  in_channels))
    , m_dim(in_width, in_height, in_channels, window_width, window_height, in_channels, stride, 0, 0)
    {

    }
    AvgPooling::AvgPooling 
    (
      const Shape& in
    , const std::vector<int>& metadata
    )
	: AvgPooling
	(
	//shapes
	  in.width(), in.height(), in.channels()
	//inputs
	, metadata[0], metadata[1], _metadata(2, 1)
    )
    {
        
    }
    //////////////////////////////////////////////////
	Layer::SPtr AvgPooling::copy() const
	{
		return std::static_pointer_cast<Layer>(std::make_shared<AvgPooling>(*this));
	}
	const Inputs AvgPooling::inputs() const
	{
		return 
		{ 
			m_dim.width_kernel, m_dim.height_kernel, m_dim.stride
		};
	}	
    //////////////////////////////////////////////////    
	const Matrix& AvgPooling::predict(const Matrix& bottom)
	{
		return feedforward(bottom);
	}
    const Matrix& AvgPooling::feedforward(const Matrix& bottom)
    {
        int n_sample = bottom.cols();
        int hw_in = m_dim.in_image_size();
        int hw_pool = m_dim.pool_size();
        int hw_out = m_dim.out_image_size();
        //alloc and init
        m_top.resize(int(out_size()), n_sample);
        m_top.setZero(); 
        //avg pooling
        for (int i = 0; i < n_sample; i ++) 
        {
            auto image = bottom.col(i);
            for (int c = 0; c < m_dim.channel_in; c ++) 
            {
                for (int i_out = 0; i_out < hw_out; i_out ++) 
                {
                    int step_h = i_out / m_dim.width_out;
                    int step_w = i_out % m_dim.width_out;
                    // left-top idx of window in raw image
                    int start_idx = step_h * m_dim.width_in * m_dim.stride + step_w * m_dim.stride;
                    for (int i_pool = 0; i_pool < hw_pool; i_pool++) 
                    {
                        if (start_idx % m_dim.width_in + i_pool % m_dim.width_kernel >= m_dim.width_in ||
                            start_idx / m_dim.width_in + i_pool / m_dim.width_kernel >= m_dim.height_in) 
                        {
                            continue;  // out of range
                        }
                        //index
                        int pick_idx = start_idx + (i_pool / m_dim.width_kernel) *
                                       m_dim.width_in + i_pool % m_dim.width_kernel + c * hw_in;
                        //test
                        if (image(pick_idx) >= m_top(c * hw_out + i_out, i)) 
                        {  // max pooling
                            m_top(c * hw_out + i_out, i) += image(pick_idx);
                        }
                    }
                    m_top(c * hw_out + i_out, i) /= hw_pool;
                }
            }
        }
        return m_top;
    }
    
    const Matrix& AvgPooling::backpropagate(const Matrix& bottom, const Matrix& grad) 
    {
        CODE_BACKPROPAGATION(
            int n_sample = bottom.cols();
            int hw_in = m_dim.in_image_size();
            int hw_pool = m_dim.pool_size();
            int hw_out = m_dim.out_image_size();
            m_grad_bottom.resize(bottom.rows(), n_sample);
            m_grad_bottom.setZero();
            //dy/dx = 1/n
            //dout = 1/n * din
            //avg pooling
            for (int i = 0; i < n_sample; i ++) 
            {
                for (int c = 0; c < m_dim.channel_in; c ++) 
                {
                    for (int i_out = 0; i_out < hw_out; i_out ++) 
                    {
                        int step_h = i_out / m_dim.width_out;
                        int step_w = i_out % m_dim.width_out;
                        // left-top idx of window in raw image
                        int start_idx = step_h * m_dim.width_in * m_dim.stride + step_w * m_dim.stride;
                        for (int i_pool = 0; i_pool < hw_pool; i_pool++) 
                        {
                            if (start_idx % m_dim.width_in + i_pool % m_dim.width_kernel >= m_dim.width_in ||
                                start_idx / m_dim.width_in + i_pool / m_dim.width_kernel >= m_dim.height_in) 
                            {
                                continue;  // out of range
                            }
                            //index
                            int pick_idx = start_idx + (i_pool / m_dim.width_kernel) *
                                        m_dim.width_in + i_pool % m_dim.width_kernel + c * hw_in;
                            //test
                            m_grad_bottom(pick_idx, i) += grad(c * hw_out + i_out, i) / hw_pool;
                        }
                    }
                }
            }
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    
	void AvgPooling::update(const Optimizer& optimize)
	{
        /* none */
	}
    //////////////////////////////////////////////////
}
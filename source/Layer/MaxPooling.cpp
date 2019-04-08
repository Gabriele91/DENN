#include "Denn/Layer/MaxPooling.h"
#define _metadata(_x, _default) (_x < metadata.size() ? metadata[_x] : _default)
#define _shapepooling(in, kernel, stride) (1 + std::ceil((in - kernel) / stride))

namespace Denn
{
    MaxPooling::MaxPooling 
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
    MaxPooling::MaxPooling 
    (
      const Shape& in
    , const std::vector<int>& metadata
    )
	: MaxPooling
	(
	//shapes
	  in.width(), in.height(), in.channels()
	//inputs
	, metadata[0], metadata[1], _metadata(2, 1)
    )
    {
        
    }
    //////////////////////////////////////////////////
	Layer::SPtr MaxPooling::copy() const
	{
		return std::static_pointer_cast<Layer>(std::make_shared<MaxPooling>(*this));
	}
	const Inputs MaxPooling::inputs() const
	{
		return 
		{ 
			m_dim.width_kernel, m_dim.height_kernel, m_dim.stride
		};
	}	
    ////////////////////////////////////////////////// 
	const Matrix& MaxPooling::predict(const Matrix& bottom)
	{
        int n_sample = bottom.cols();
        int hw_in = m_dim.in_image_size();
        int hw_pool = m_dim.pool_size();
        int hw_out = m_dim.out_image_size();
        //alloc and init
        m_top.resize(int(out_size()), n_sample);
        m_top.setZero(); 
        m_top.array() += std::numeric_limits<float>::lowest();
        //backpropagation
        CODE_BACKPROPAGATION(
            m_max_idxs.resize(n_sample, std::vector<int>(int(out_size()), 0));
        )
        //max pooling
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
                    for (int i_pool = 0; i_pool < hw_pool; i_pool ++) 
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
                            m_top(c * hw_out + i_out, i) = image(pick_idx);
                        }
                    }
                }
            }
        }
        return m_top;
	}
    const Matrix& MaxPooling::feedforward(const Matrix& bottom)
    {
        int n_sample = bottom.cols();
        int hw_in = m_dim.in_image_size();
        int hw_pool = m_dim.pool_size();
        int hw_out = m_dim.out_image_size();
        //alloc and init
        m_top.resize(int(out_size()), n_sample);
        m_top.setZero(); 
        m_top.array() += std::numeric_limits<float>::lowest();
        //backpropagation
        CODE_BACKPROPAGATION(
            m_max_idxs.resize(n_sample, std::vector<int>(int(out_size()), 0));
        )
        //max pooling
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
                    for (int i_pool = 0; i_pool < hw_pool; i_pool ++) 
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
                            m_top(c * hw_out + i_out, i) = image(pick_idx);
                            CODE_BACKPROPAGATION(
                                m_max_idxs[i][c * hw_out + i_out] = pick_idx;
                            )
                        }
                    }
                }
            }
        }
        return m_top;
    }
    
    const Matrix& MaxPooling::backpropagate(const Matrix& bottom, const Matrix& grad) 
    {
        CODE_BACKPROPAGATION(
            m_grad_bottom.resize(bottom.rows(), bottom.cols());
            m_grad_bottom.setZero();
            for (int i = 0; i < m_max_idxs.size(); i ++) 
            {  // i-th sample
                for (int j = 0; j < m_max_idxs[i].size(); j ++)
                {
                    m_grad_bottom(m_max_idxs[i][j], i) += grad(j, i);
                }
            }
        )
        RETURN_BACKPROPAGATION(m_grad_bottom);
    }
    
	void MaxPooling::update(const Optimizer& optimize)
	{
        /* none */
	}
    //////////////////////////////////////////////////
}
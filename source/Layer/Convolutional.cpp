#include "Denn/Layer/Convolutional.h"
#define _metadata(_x, _default) (_x < metadata.size() ? metadata[_x] : _default)
#define _convshape(in, kernel, stride, pad) (1 + (in - kernel + 2 * pad) / stride)
namespace Denn
{
	///////////////////////////////////////	
	Convolutional::Convolutional
	(
	  int in_width, int in_height, int in_channels
	, int window_width, int window_height, int out_channels
	, int stride, int pad_w, int pad_h
	)
	: DerivableLayer("convolutional",
		Shape(in_width, in_height, in_channels),
		Shape(_convshape(in_width, window_width, stride, pad_w), 
			  _convshape(in_height, window_height, stride, pad_h),
			  out_channels))
	, m_dim(in_width, in_height, in_channels, window_width, window_height, out_channels, stride, pad_w, pad_h)
	{
		// Set data dimension
		m_kernels.resize(m_dim.kernel_size(), m_dim.channel_out);
		m_bias.resize(m_dim.channel_out);
		// Backpropagation
		CODE_BACKPROPAGATION(
			m_grad_bias.resize(m_dim.channel_out);
			m_grad_kernels.resize(m_dim.kernel_size(), m_dim.channel_out);
		)
	}
	Convolutional::Convolutional
	(
		  const Shape& in
		, const std::vector<int>& metadata
	)
	: Convolutional
	(
	//shapes
	  in.width(), in.height(), in.channels()
	//inputs
	, metadata[0], metadata[1], _metadata(2, 1)
	, _metadata(3, 1), _metadata(4, 0), _metadata(5, 0)
	)
	{
	}
	//////////////////////////////////////////////////
	const Inputs Convolutional::inputs() const
	{
		return 
		{ 
			m_dim.width_kernel, m_dim.height_kernel, m_dim.channel_out,
			m_dim.stride, m_dim.pad_w, m_dim.pad_h
		};
	}
	//////////////////////////////////////////////////
	Layer::SPtr Convolutional::copy() const
	{
		return std::static_pointer_cast<Layer>(std::make_shared<Convolutional>(*this));
	}
	//////////////////////////////////////////////////
	const Matrix& Convolutional::predict(const Matrix& bottom)
	{
			// Each column is an observation
			int n_sample = bottom.cols();
			m_top.resize(int(out_size()), n_sample);
			//Buffer
			thread_local Matrix image;
			//for each
			for (int i = 0; i < n_sample; i++) 
			{
				// im2col
				internal::img_to_col(m_dim, bottom.col(i), image);
				// conv by product
				AlignedMapMatrix result(m_top.col(i).data(), image.rows(), m_kernels.cols());
				result.noalias() = image * m_kernels;
				result.rowwise() += m_bias.transpose();
			}
			return m_top;
	}
	const Matrix& Convolutional::feedforward(const Matrix& bottom)
	{
		// Each column is an observation
		int n_sample = bottom.cols();
		m_top.resize(int(out_size()), n_sample);
		//backpropagation buffer
		CODE_BACKPROPAGATION(
			m_images.resize(n_sample);
		)
		//Buffer
        thread_local Matrix image;
		//for each
		for (int i = 0; i < n_sample; i++) 
		{
			// im2col
			internal::img_to_col(m_dim, bottom.col(i), image);
			//save for backpropagation pass
			CODE_BACKPROPAGATION(
				m_images[i] = image;
			)
			// conv by product
			AlignedMapMatrix result(m_top.col(i).data(), image.rows(), m_kernels.cols());
			result.noalias() = image * m_kernels;
			result.rowwise() += m_bias.transpose();
		}
		//return
		return m_top;
	}
	const Matrix&  Convolutional::backpropagate(const Matrix& bottom, const Matrix& grad)
	{
		CODE_BACKPROPAGATION(
			int n_sample = bottom.cols();
			m_grad_kernels.setZero();
			m_grad_bias.setZero();
			m_grad_bottom.resize(int(in_size()), n_sample);
			m_grad_bottom.setZero();

			for (int i = 0; i < n_sample; i++) 
			{
				// im2col of grad_top
				Matrix grad_top_i = grad.col(i);
				Matrix grad_top_i_col = Eigen::Map<Matrix>(grad_top_i.data(), m_dim.out_image_size(), m_dim.channel_out);
				// d(L)/d(w) = \sum{ d(L)/d(z_i) * d(z_i)/d(w) }
				m_grad_kernels += m_images[i].transpose() * grad_top_i_col;
				// d(L)/d(b) = \sum{ d(L)/d(z_i) * d(z_i)/d(b) }
				m_grad_bias += grad_top_i_col.colwise().sum().transpose();
				// d(L)/d(x) = \sum{ d(L)/d(z_i) * d(z_i)/d(x) } = d(L)/d(z)_col * w'
				Matrix grad_bottom_i_col = grad_top_i_col * m_kernels.transpose();
				// col2im of grad_bottom
				internal::col_to_img(m_dim, grad_bottom_i_col, m_grad_bottom.col(i));
			}
		)
        RETURN_BACKPROPAGATION(m_grad_bottom);
	}
	void Convolutional::update(const Optimizer& optimize)
	{
		CODE_BACKPROPAGATION(
			AlignedMapColVector  w(m_kernels.data(), m_kernels.size());
			AlignedMapColVector  b(m_bias.data(), m_bias.size());
			ConstAlignedMapColVector dw(m_grad_kernels.data(), m_grad_kernels.size());
			ConstAlignedMapColVector db(m_grad_bias.data(), m_grad_bias.size());

			optimize.update(w, dw);
			optimize.update(b, db);
		)
		BACKPROPAGATION_ASSERT
	}
	//////////////////////////////////////////////////
	size_t Convolutional::size() const
	{
		return 2;
	}
	AlignedMapMatrix Convolutional::operator[](size_t i)
	{
		denn_assert(i < 2);
		switch (i)
		{
			default:
			case 0: return  AlignedMapMatrix(m_kernels.data(), m_kernels.rows(), m_kernels.cols()); 
			case 1: return  AlignedMapMatrix(m_bias.data(), m_bias.rows(), m_bias.cols());
		}
	}
	ConstAlignedMapMatrix Convolutional::operator[](size_t i) const
	{
		denn_assert(i < 2);
		switch (i)
		{
			default:
			case 0: return  ConstAlignedMapMatrix(m_kernels.data(), m_kernels.rows(), m_kernels.cols());
			case 1: return  ConstAlignedMapMatrix(m_bias.data(), m_bias.rows(), m_bias.cols());
		}
	}
	//////////////////////////////////////////////////
}

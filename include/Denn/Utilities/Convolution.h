#pragma once
#include "Denn/Config.h"

namespace Denn 
{

namespace internal 
{
	struct ConvDims
	{
		//from user
		int width_in;
		int height_in;
		int channel_in;

		int width_kernel;
		int height_kernel;
		int channel_out;

		int stride;
		int pad_w;
		int pad_h;
		
		//self compute
		int width_out;
		int height_out;
		int dim_out;

		//init
		ConvDims() = default;
		ConvDims
		(
			int in_width, int in_height, int in_channels,
			int window_width, int window_height, int out_channels = 1,
			int stride = 1, int pad_w = 0, int pad_h = 0
		)
		: width_in(in_width)
		, height_in(in_height)
		, channel_in(in_channels)
		, width_kernel(window_width)
		, height_kernel(window_height)
		, channel_out(out_channels)
		, stride(stride)
		, pad_w(pad_w)
		, pad_h(pad_h)
		{
			height_out = (1 + (height_in - height_kernel + 2 * pad_h) / stride);
			width_out = (1 + (width_in - width_kernel + 2 * pad_w) / stride);
			dim_out = height_out * width_out * channel_out;
		}

		int in_image_size() const
		{
			return height_in * width_in;
		}

		int out_image_size() const
		{
			return height_out * width_out;
		}

		int on_channel_kernel_size() const
		{
			return height_kernel * width_kernel;
		}

		int kernel_size() const
		{
			return channel_in  * height_kernel * width_kernel;
		}

		int out_size() const
		{
			return dim_out;
		}
	};

	inline void img_to_col(const ConvDims& dim,const ColVector& image, Matrix& data_col)
	{
		int hw_in = dim.in_image_size();
		int hw_kernel = dim.on_channel_kernel_size();
		int hw_out = dim.out_image_size();
		// im2col
		data_col.resize(hw_out, hw_kernel * dim.channel_in);
		for (int c = 0; c < dim.channel_in; c++) 
		{
			ColVector map = image.block(hw_in * c, 0, hw_in, 1);  // c-th channel map
			for (int i = 0; i < hw_out; i++) 
			{
				int step_h = i / dim.width_out;
				int step_w = i % dim.width_out;
				int start_idx = step_h * dim.width_in * dim.stride + step_w * dim.stride;  // left-top idx of window
				for (int j = 0; j < hw_kernel; j++) 
				{
					int cur_col = start_idx % dim.width_in + j % dim.width_kernel - dim.pad_w;  // col after padding
					int cur_row = start_idx / dim.width_in + j / dim.width_kernel - dim.pad_h;
					if (cur_col < 0 || cur_col >= dim.width_in || cur_row < 0 || cur_row >= dim.height_in)
					{
						data_col(i, c * hw_kernel + j) = 0;
					}
					else 
					{
						//int pick_idx = start_idx + (j / width_kernel) * width_in + j % width_kernel;
						int pick_idx = cur_row * dim.width_in + cur_col;
						data_col(i, c * hw_kernel + j) = map(pick_idx);  // pick which pixel
					}
				}
			}
		}
	}

	inline void col_to_img(const ConvDims& dim, const Matrix& data_col, ColVector& image)
	{
		int hw_in = dim.in_image_size();
		int hw_kernel = dim.on_channel_kernel_size();
		int hw_out = dim.out_image_size();
		// col2im
		image.resize(hw_in * dim.channel_in);
		image.setZero();
		for (int c = 0; c < dim.channel_in; c++)
		{
			for (int i = 0; i < hw_out; i++)
			{
				int step_h = i / dim.width_out;
				int step_w = i % dim.width_out;
				int start_idx = step_h * dim.width_in * dim.stride + step_w * dim.stride;  // left-top idx of window
				for (int j = 0; j < hw_kernel; j++) 
				{
					int cur_col = start_idx % dim.width_in + j % dim.width_kernel - dim.pad_w;  // col after padding
					int cur_row = start_idx / dim.width_in + j / dim.width_kernel - dim.pad_h;
					if (cur_col < 0 || cur_col >= dim.width_in || cur_row < 0 || cur_row >= dim.height_in)
					{
						continue;
					}
					else 
					{
						//int pick_idx = start_idx + (j / width_kernel) * width_in + j % width_kernel;
						int pick_idx = cur_row * dim.width_in + cur_col;
						image(c * hw_in + pick_idx) += data_col(i, c * hw_kernel + j);  // pick which pixel
					}
				}
			}
		}
	}
} // namespace internal

} // namespace Denn

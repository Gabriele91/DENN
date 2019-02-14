#pragma once

namespace Denn
{
class Shape
{
public:

	Shape(int width = 0, int height = 1, int channels = 1)
	: m_width(width), m_height(height), m_channels(channels)
	{
	}

	explicit operator int() const { return size3D(); }
	int width() const { return m_width; }
	int height() const { return m_height; }
	int channels() const { return m_channels; }
	int size3D() const  { return m_width * m_height * m_channels; }
	int size2D() const { return  m_width * m_height; }
	bool is3D() { return m_channels > 1; }
	bool is2D() { return m_channels == 1; }
	bool is1D() { return m_height == 1 && m_channels == 1; }

protected:

	int m_width;
	int m_height;
	int m_channels;
};
}
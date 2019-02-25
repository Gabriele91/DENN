#include "Denn/TestSetStream.h"

namespace Denn
{
	//start
	void TestSetStream::start_read_batch(size_t batch_size, size_t rows_offset)
	{
		//start
		m_cache_cols_read = 0;
		m_batch_size = batch_size;
		m_batch_offset = rows_offset;
		m_dataset->start_read_batch();
		//init batch
		m_batch.m_features_shape = Shape(m_dataset->get_main_header_info().m_n_features);
		m_batch.m_labels_shape = Shape(m_dataset->get_main_header_info().m_n_classes); 
		//int features
		m_batch.features().conservativeResize
		(
			  m_dataset->get_main_header_info().m_n_features
			, m_batch_size
		);
		//init labels
		m_batch.labels().conservativeResize
		(
			  m_dataset->get_main_header_info().m_n_classes
			, m_batch_size
		);
		//read
		read_batch(m_batch_size);
	}

	//read
	DataSetScalar& TestSetStream::read_batch()
	{
		return read_batch(m_batch_offset);
	}

	//get last
	const DataSetScalar& TestSetStream::last_batch() const
	{
		return m_batch;
	}
	//read
	DataSetScalar& TestSetStream::read_batch(size_t n_cols_update)
	{
		//test
		denn_assert(n_cols_update <= m_batch_size);
		//olready read
		const size_t c_features = m_dataset->get_main_header_info().m_n_features;
		const size_t c_labels = m_dataset->get_main_header_info().m_n_classes;
		size_t offset = 0;
		size_t n_read = 0;
		//shift (not equal)
		if (n_cols_update && n_cols_update < m_batch_size)
		{
			//put the last N values on the top (for all features)
			m_batch.features() = Denn::shift_left(m_batch.features(), n_cols_update);
			//put the last N values on the top
			m_batch.labels() = Denn::shift_left(m_batch.labels(), n_cols_update);
			//start to n_rows
			offset = m_batch.features().cols() - n_cols_update;
		}
		//copy next
		while (n_read < n_cols_update)
		{
			//get remaning
			size_t read_remaning = n_cols_update - n_read;
			//restart
			if (m_cache_cols_read >= m_cache_batch.features().cols())
				m_cache_cols_read = 0;
			//init case
			if(!m_cache_cols_read)
				m_dataset->read_batch(m_cache_batch);
			//compute n rows to read
			size_t n_samples = m_cache_batch.features().cols() - m_cache_cols_read;
			size_t to_read = std::min<size_t>(n_samples, read_remaning);
			//copy all features
			m_batch.features().block(0, offset, c_features, to_read).noalias() = m_cache_batch.features().block(0, m_cache_cols_read, c_features, to_read);
			//copy all labels
			m_batch.labels().block(0, offset, c_labels, to_read).noalias() = m_cache_batch.labels().block(0, m_cache_cols_read, c_labels, to_read);
			//move
			n_read += to_read;
			offset += to_read;
			m_cache_cols_read += to_read;
		}
		//get
		return m_batch;
	}
}
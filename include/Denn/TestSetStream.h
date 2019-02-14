#pragma once
#include "Config.h"
#include "Dataset.h"
#include "DatasetLoader.h"

namespace Denn
{
	//dataset stream
	class TestSetStream
	{
	public:

		TestSetStream( DataSetLoader* dataset )
		: m_dataset(dataset)
		{
		}

		//start
		void start_read_batch(size_t batch_size, size_t rows_offset);

		//read
		DataSetScalar& read_batch();

		//get last
		const DataSetScalar& last_batch() const;

		//info
		size_t batch_size() const { return m_batch_size; }
		size_t batch_offset() const { return m_batch_offset; }

	protected:

		DataSetLoader*  m_dataset;				//< dataset
		size_t          m_batch_size  { 0 };	//< virtual batch size
		size_t          m_batch_offset{ 0 };	//< next rows for next batch

		size_t			m_cache_cols_read;		//< ptr last row in cache
		DataSetScalar	m_cache_batch;			//< cache 
		DataSetScalar	m_batch;

		//read
		DataSetScalar& read_batch(size_t n_rows);

	};
}
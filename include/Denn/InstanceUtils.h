#pragma once
#include "Config.h"
#include "Parameters.h"
#include "SerializeOutput.h"

namespace Denn
{
	//build output stream
	bool build_outputstream(std::ostream& output, std::ofstream& file, const Denn::Parameters& parameters);
	//build output stream
	bool build_serialize(SerializeOutput::SPtr& serialize, std::ofstream& file, const Denn::Parameters& parameters);
	//build thread pool
	bool build_thread_pool(std::unique_ptr<ThreadPool>& tpool, const Denn::Parameters& parameters);
}
#pragma once

namespace Denn
{
namespace Time
{
	//get ticks
	extern unsigned long long get_ticks_time();
	//get milliseconds time
	extern double get_ms_time();
	//get seconds time
	inline double get_time() { return get_ms_time() / 1000.0; }
}
}
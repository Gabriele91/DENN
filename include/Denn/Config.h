#pragma once
//default C include
#include <cstring>
#include <cmath>
#include <cassert>
#include <cctype>
//default CPP include
#include <limits>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <functional>
#include <memory>
#include <iostream>
//lib include
#include <Eigen/Eigen>
#include <zlib.h>
//macro
#define MESSAGE( _msg_ ) std::cout<< _msg_ <<std::endl;
//MSMACRO
#if defined( _MSC_VER )
	#define ASPACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
	#define operator_override
	#define denn_noop __noop 
#else 
	#define ASPACKED( __Declaration__ ) __Declaration__ __attribute__((__packed__))
	#define operator_override override
	#ifdef __clang__
		#define denn_noop ((void)0)
	#else
		#define denn_noop (__ASSERT_VOID_CAST (0))
	#endif 
#endif
//Debug
#if !defined(  NDEBUG )
	#define debug_message(_msg_) std::cout<< _msg_ <<std::endl;
	#define denn_assert(_exp_) assert(_exp_)
	#define denn_assert_code(_code_)  assert( _code_ );
#else
	#define debug_message(_msg_)
	#define denn_assert(_exp_) denn_noop
	#define denn_assert_code(_code_)  _code_ ;
#endif

#if !defined(DISABLE_BACKPROPAGATION)
	#define BACKPROPAGATION true
	#define BACKPROPAGATION_ASSERT denn_noop;
	#define CODE_BACKPROPAGATION(...) __VA_ARGS__
	#define RETURN_BACKPROPAGATION(value) return value
#else 
	#define BACKPROPAGATION false
	#define BACKPROPAGATION_ASSERT { MESSAGE("backpropagation does not supported"); exit(-1); };
	#define CODE_BACKPROPAGATION(...) 
	#define RETURN_BACKPROPAGATION(value) \
		{ BACKPROPAGATION_ASSERT static Matrix __tmp__; return __tmp__; }
#endif

//core
#include "Core/Scalar.h"
#include "Core/EigenAlias.h"
#include "Core/Variant.h"
#include "Core/Random.h"
#include "Core/TicksTime.h"
#include "Core/ThreadPool.h"
#include "Core/Filesystem.h"
#include "Core/IOFileWrapper.h"
#include "Core/Dump.h"
#include "Core/Json.h"
//utilities
#include "Utilities/Math.h"
#include "Utilities/Matrix.h"
#include "Utilities/String.h"
#include "Utilities/Vector.h"

#if !defined(SCALAR_EPS)
	#if defined(USE_SCALAR_EPS_BY_TYPE)
		#define SCALAR_EPS (Denn::Constants<Scalar>::epsilon())
	#else
		#define SCALAR_EPS (Scalar(1e-6))
	#endif
#endif

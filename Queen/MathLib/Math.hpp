#ifndef Math_h__
#define Math_h__

#include <cmath>
#include <memory>

//#define USE_SIMD

#ifdef USE_SIMD
#include <pmmintrin.h>  // SSE3 including
#endif

#define ALIGNED_16  __declspec(align(16))
#define ALIGNED_4   __declspec(align(4))


#endif // Math_h__

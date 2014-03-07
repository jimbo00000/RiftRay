// VectorMath.h
// This pair of file contains float3 operations which
// may also be included in the CUDA SDK.
///@todo What is the cleanest way to include vectormath if CUDA is not available? Is it worth it?

#ifndef _VECTORMATH_H_
#define _VECTORMATH_H_

///@todo Where do we need these Windows headers?
#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

// Use CUDA float3 types
#ifdef USE_CUDA
#include <vector_types.h>
#else
#  include "vectortypes.h"
#endif

float  length   (float3 v);
float  length2  (float3 v);
float3 normalize(float3 v);
float  dot      (float3 a, float3 b);
float3 cross    (const float3& b, const float3& c);
float3 operator+(const float3& a, const float3& b);
float3 operator-(const float3& a, const float3& b);
float3 operator*(      float    , const float3& b);

int2   operator+(const int2& a, const int2& b);

#endif // _VECTORMATH_H_

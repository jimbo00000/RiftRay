// MatrixMath.h

#ifndef _MATRIXMATH_H_
#define _MATRIXMATH_H_

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#ifdef USE_CUDA
#  include <vector_types.h>
#else
#  include "vectortypes.h"
#  include "VectorMath.h"
#endif

float3 transform(const float3 pt, const float* mtx);

void MakeIdentityMatrix   (float* dst);
void MakeTranslationMatrix(float* mtx, float3 vec);
void MakeRotationMatrix   (float* mtx, float theta, float3 axis);

void preMultiply (float* m1, const float* m2); // modifies first parameter
void postMultiply(float* m1, const float* m2); // modifies first parameter

void glhTranslate(float* mtx,
                  float x, 
                  float y,
                  float z);

void glhRotate(float* mtx,
               float theta,
               float x, 
               float y,
               float z);

void glhScale(float* mtx,
              float x, 
              float y,
              float z);

void glhFrustumf2(float *matrix,
                  float left, float right,
                  float bottom, float top,
                  float znear, float zfar);

void glhPerspectivef2(float *matrix,
                      float fovyInDegrees,
                      float aspectRatio,
                      float znear, float zfar);

void glhLookAtf2(float *matrix,
                 float3 eye,
                 float3 center,
                 float3 up);

void glhOrtho(float *matrix,
              float left,
              float right,
              float bottom,
              float top,
              float near,
              float far);

#endif //_MATRIXMATH_H_

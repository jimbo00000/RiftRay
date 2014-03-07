// MatrixMath.cpp

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_CUDA
#  include <cutil_math.h>
#else
#  include "VectorMath.h"
#endif

#include "utils/MatrixMath.h"

#ifndef M_PI
#  define M_PI   3.14159265358979323846264338327
#endif

// Transforms the point by the specified 4x4 transformation matrix.
// Note that the matrix is specified in COLUMN order.
float3 transform(const float3 pt, const float* mtx)
{
    float4 vec = {
        mtx[0]*pt.x + mtx[4]*pt.y + mtx[ 8]*pt.z + mtx[12],
        mtx[1]*pt.x + mtx[5]*pt.y + mtx[ 9]*pt.z + mtx[13],
        mtx[2]*pt.x + mtx[6]*pt.y + mtx[10]*pt.z + mtx[14],
        mtx[3]*pt.x + mtx[7]*pt.y + mtx[11]*pt.z + mtx[15],
    };
    float3 vec3 = {
        vec.x / vec.w,
        vec.y / vec.w,
        vec.z / vec.w,
    };
    return vec3;
}

void MakeIdentityMatrix(float* dst)
{
    float id[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    memcpy(dst, id, 16*sizeof(float));
}

void MakeTranslationMatrix(float* mtx, float3 vec)
{
    if (!mtx)
        return;
    memset(mtx, 0, 16*sizeof(float));

    mtx[0] = 1.0f;
    mtx[5] = 1.0f;
    mtx[10] = 1.0f;
    mtx[12] = vec.x;
    mtx[13] = vec.y;
    mtx[14] = vec.z;
    mtx[15] = 1.0f;
}

// Creates a rotation matrix in the array pointed to by mtx
void MakeRotationMatrix(float* mtx, float theta, float3 axis)
{
    if (!mtx) return;

    float len = length(axis);
    if (len < 0.000001f)
        return;
    normalize(axis);

    float c = cos(theta);
    float s = sin(theta);
    float t = 1-c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    mtx[0] = t*x*x + c;
    mtx[1] = t*x*y - s*z;
    mtx[2] = t*x*z + s*y;
    mtx[3] = 0.0f;

    mtx[4] = t*x*y + s*z;
    mtx[5] = t*y*y + c;
    mtx[6] = t*y*z - s*x;
    mtx[7] = 0.0f;

    mtx[8] = t*x*z - s*y;
    mtx[9] = t*y*z + s*x;
    mtx[10] = t*z*z + c;
    mtx[11] = 0.0f;

    mtx[12] = 0.0f;
    mtx[13] = 0.0f;
    mtx[14] = 0.0f;
    mtx[15] = 1.0f;
}


// Store result in first parameter
void preMultiply(float* a, const float* b)
{
    if(!a || !b) return;

    float result[16];
    memset(result, 0, 16*sizeof(float));

    for (unsigned int i=0; i<16; i+=4)
    {
        for (unsigned int j=0; j<4; ++j)
        {
            result[i+j] =  a[i+0] * b[j+0]
                         + a[i+1] * b[j+4]
                         + a[i+2] * b[j+8]
                         + a[i+3] * b[j+12];
        }
    }

    memcpy(a, result, 16*sizeof(float));
}

// Store result in first parameter
void postMultiply(float* a, const float* b)
{
    if (!a || !b)
        return;

    float result[16];
    memset(result, 0, 16*sizeof(float));

    for (unsigned int i=0; i<16; i+=4)
    {
        for (unsigned int j=0; j<4; ++j)
        {
            result[i+j] =  b[i+0] * a[j+0]
                         + b[i+1] * a[j+4]
                         + b[i+2] * a[j+8]
                         + b[i+3] * a[j+12];
        }
    }

    memcpy(a, result, 16*sizeof(float));
}

void glhTranslate(float* mtx,
                  float x, 
                  float y,
                  float z)
{
    float txmtx[16] = {
        1,0,0,0,
        0,1,0,0, 
        0,0,1,0,
        x,y,z,1
    };
    postMultiply(mtx, txmtx);
}

void glhRotate(float* mtx,
               float theta,
               float x, 
               float y,
               float z)
{
    float rotmtx[16];
    float3 axis = {x,y,z};
    MakeRotationMatrix(rotmtx, -theta * (float)M_PI / 180.0f, axis);
    postMultiply(mtx, rotmtx);
}

void glhScale(float* mtx,
              float x, 
              float y,
              float z)
{
    float scmtx[16] = {
        x,0,0,0,
        0,y,0,0, 
        0,0,z,0,
        0,0,0,1
    };
    postMultiply(mtx, scmtx);
}

/// Support for glhPerspectivef2, which is a standin for gluPerspective.
// http://www.opengl.org/wiki/GluPerspective_code
void glhFrustumf2(float *matrix,
                  float left, float right,
                  float bottom, float top,
                  float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp  = 2.0f * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;

    matrix[0] = temp / temp2;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;
    matrix[4] = 0.0;
    matrix[5] = temp / temp3;
    matrix[6] = 0.0;
    matrix[7] = 0.0;
    matrix[8] = (right + left) / temp2;
    matrix[9] = (top + bottom) / temp3;
    matrix[10] = (-zfar - znear) / temp4;
    matrix[11] = -1.0;
    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = (-temp * zfar) / temp4;
    matrix[15] = 0.0;
}

// http://www.opengl.org/wiki/GluPerspective_code
void glhPerspectivef2(float *matrix,
                      float fovyInDegrees,
                      float aspectRatio,
                      float znear, float zfar)
{
    float ymax, xmax;
    ymax = znear * tanf(fovyInDegrees * (float)M_PI / 360.0f);
    //ymin = -ymax;
    //xmin = -ymax * aspectRatio;
    xmax = ymax * aspectRatio;
    glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void glhLookAtf2(float *matrix,
                 float3 eye,
                 float3 center,
                 float3 up)
{
    memset(matrix, 0, 16*sizeof(float));
    float3 forward = normalize(center - eye);
    float3 right   = normalize(cross(forward, up));
    float3 up2     = cross(right, forward);

    matrix[0] = right.x;
    matrix[4] = right.y;
    matrix[8] = right.z;

    matrix[1] = up2.x;
    matrix[5] = up2.y;
    matrix[9] = up2.z;

    matrix[2]  = -forward.x;
    matrix[6]  = -forward.y;
    matrix[10] = -forward.z;

    matrix[15] = 1.0f;

    glhTranslate(matrix, -eye.x, -eye.y, -eye.z);
}

// From openGL reference
// http://www.talisman.org/opengl-1.1/Reference/glOrtho.html
void glhOrtho(float *matrix,
              float leftVal,
              float rightVal,
              float bottomVal,
              float topVal,
              float nearVal,
              float farVal)
{
    float A  =  2 / (rightVal -   leftVal);
    float B  =  2 / (  topVal - bottomVal);
    float C  = -2 / (  farVal -   nearVal);
    float tx = -(rightVal +   leftVal) / (rightVal -   leftVal);
    float ty = -(topVal   + bottomVal) / (topVal   - bottomVal);
    float tz = -(farVal   +   nearVal) / (farVal   -   nearVal);

    float mtx[16] = {
         A,  0,  0, 0,
         0,  B,  0, 0,
         0,  0,  C, 0,
        tx, ty, tz, 1
    };

    memcpy(matrix, mtx, 16*sizeof(float));
}

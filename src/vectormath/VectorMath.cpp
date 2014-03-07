// VectorMath.cpp

#include <math.h>
#include <string.h>
#include "VectorMath.h"
#include "vector_make_helpers.h"

float length(float3 v)
{
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

float length2(float3 v)
{
    return (v.x*v.x + v.y*v.y + v.z*v.z);
}

float3 normalize(float3 v)
{
    float len = length(v);
    float3 vec = {v.x/len, v.y/len, v.z/len};
    return vec;
}

float dot(float3 a, float3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float3 cross(const float3& b, const float3& c)
{
    float3 a = {b.y*c.z - b.z*c.y,
                b.z*c.x - b.x*c.z,
                b.x*c.y - b.y*c.x};
    normalize(a);
    return a;
}

float3 operator+ (const float3& a, const float3& b)
{
    float3 sum = {a.x + b.x,
                  a.y + b.y,
                  a.z + b.z};
    return sum;
}

float3 operator- (const float3& a, const float3& b)
{
    float3 sum = {a.x - b.x,
                  a.y - b.y,
                  a.z - b.z};
    return sum;
}

float3 operator* (float c, const float3& b)
{
    float3 prod = {c * b.x,
                   c * b.y,
                   c * b.z};
    return prod;
}

int2 operator+ (const int2& a, const int2& b)
{
    int2 sum = make_int2(a.x + b.x, a.y + b.y);
    return sum;
}

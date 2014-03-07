// vectortypes.h

#ifndef _VECTORTYPES_H_
#define _VECTORTYPES_H_

/// A pair of ints
struct int2 {
    int x, y;
    int2(int _x, int _y): x(_x), y(_y) {}
};

struct uint3 {
    unsigned int x, y, z;
};

//#pragma pack(1)
/// A point in 2 space.
struct float2 {
    float x,y;
    //float2(float _x, float _y): x(_x), y(_y) {}
};

/// A point in 3 space.
/// @todo3 Do we want the constructor here? Refactor if so.
struct float3 {
    float x,y,z;
    //float3(float _x, float _y, float _z): x(_x), y(_y), z(_z) {}
};

/// A point in 3 space with homogeneous coordinate.
/// Used for transforming float3 by 4x4 matrix.
struct float4 {
    float x,y,z,w;
};

/// An axis-aligned rectangle in screen space.
struct rect {
    int x,y,w,h;
    rect(int _x, int _y, int _w, int _h): x(_x), y(_y), w(_w), h(_h) {}
};


/// A float3 with corresponding index into a point list.
struct float3Idx {
    float3 pt;
    int idx;
};

#endif //_VECTORTYPES_H_

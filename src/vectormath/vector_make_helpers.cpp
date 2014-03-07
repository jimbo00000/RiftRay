// vector_make_helpers.cpp

#include "vectortypes.h"

int2 make_int2(int a, int b)
{
    int2 i2(a,b);
    i2.x = a;
    i2.y = b;
    return i2;
}

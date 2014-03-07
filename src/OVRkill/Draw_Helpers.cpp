// Draw_Helpers.cpp

#include "Draw_Helpers.h"
#include "VectorMath.h"

#include <GL/glew.h>

void DrawOriginLines()
{
    const float3 minPt = {0,0,0};
    const float3 maxPt = {1,1,1};
    const float3 verts[] = {
        {0,0,0},
        {1,0,0},
        {0,1,0},
        {0,0,1},
    };
    const unsigned int lines[] = {
        0,1,
        0,2,
        0,3,
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_LINES,
                   3*2,
                   GL_UNSIGNED_INT,
                   &lines[0]);
}


/// Draw a frustum oriented facing along negative z.
void DrawViewFrustum(float aspect)
{
    const float3 forward = {0,0,-1};
    const float3 up = {0,1,0};
    const float3 right = {1,0,0};
    const float forwardLength = 1.0f;

    const float3 origin = {0,0,0};
    const float xoff = 0.8f;
    const float yoff = xoff / aspect;

    const float3 verts[] = {
        origin,
        origin + forward + xoff * right + yoff * up,
        origin + forward + xoff * right - yoff * up,
        origin + forward - xoff * right - yoff * up,
        origin + forward - xoff * right + yoff * up,
    };
    const float3 cols[] = {
        {1,1,1},
        {1,1,0},
        {1,0,0},
        {0,0,1},
        {0,1,0},
    };
    const unsigned int lines[] = {
        0,1,
        0,2,
        0,3,
        0,4,
        1,2,
        2,3,
        3,4,
        4,1,
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, cols);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_LINES,
                   sizeof(lines)/sizeof(lines[0]),
                   GL_UNSIGNED_INT,
                   &lines[0]);
}


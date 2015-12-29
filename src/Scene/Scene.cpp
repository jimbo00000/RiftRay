// Scene.cpp

#include "Scene.h"

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdlib.h>
#include <string.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include <GL/glew.h>

#include "Logger.h"

Scene::Scene()
: m_basic()
, m_plane()
, m_phaseVal(0.0f)
, m_amplitude(0.01f)
{
}

Scene::~Scene()
{
}

void Scene::initGL()
{
    m_basic.initProgram("basic");
    m_basic.bindVAO();
    _InitCubeAttributes();
    glBindVertexArray(0);

    m_plane.initProgram("basicplane");
    m_plane.bindVAO();
    _InitPlaneAttributes();
    glBindVertexArray(0);
}

void Scene::exitGL()
{
    m_basic.destroy();
    m_plane.destroy();
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void Scene::_InitCubeAttributes()
{
    const glm::vec3 minPt(0,0,0);
    const glm::vec3 maxPt(1,1,1);
    const glm::vec3 verts[] = {
        minPt,
        glm::vec3(maxPt.x, minPt.y, minPt.z),
        glm::vec3(maxPt.x, maxPt.y, minPt.z),
        glm::vec3(minPt.x, maxPt.y, minPt.z),
        glm::vec3(minPt.x, minPt.y, maxPt.z),
        glm::vec3(maxPt.x, minPt.y, maxPt.z),
        maxPt,
        glm::vec3(minPt.x, maxPt.y, maxPt.z)
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_basic.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, 8*3*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_basic.AddVbo("vColor", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, 8*3*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vColor"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_basic.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_basic.GetAttrLoc("vColor"));

    const unsigned int quads[] = {
        0,3,2, 1,0,2, // ccw
        4,5,6, 7,4,6,
        1,2,6, 5,1,6,
        2,3,7, 6,2,7,
        3,0,4, 7,3,4,
        0,1,5, 4,0,5,
    };
    GLuint quadVbo = 0;
    glGenBuffers(1, &quadVbo);
    m_basic.AddVbo("elements", quadVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*3*sizeof(GLuint), quads, GL_STATIC_DRAW);
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void Scene::_InitPlaneAttributes()
{
    const glm::vec3 minPt(-10.0f, 0.0f, -10.0f);
    const glm::vec3 maxPt(10.0f, 0.0f, 10.0f);
    const float verts[] = {
        minPt.x, minPt.y, minPt.z,
        minPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, minPt.z,
    };
    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_plane.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*3*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_plane.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const float texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_plane.AddVbo("vTexCoord", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_plane.GetAttrLoc("vTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_plane.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_plane.GetAttrLoc("vTexCoord"));

    const unsigned int tris[] = {
        0,3,2, 1,0,2, // ccw
    };
    GLuint triVbo = 0;
    glGenBuffers(1, &triVbo);
    m_plane.AddVbo("elements", triVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*sizeof(GLuint), tris, GL_STATIC_DRAW);
}

// Draw an RGB color cube
void Scene::DrawColorCube() const
{
    m_basic.bindVAO();
    glDrawElements(GL_TRIANGLES,
                   6*3*2, // 6 triangle pairs
                   GL_UNSIGNED_INT,
                   0);
    glBindVertexArray(0);
}

/// Draw a circle of color cubes(why not)
void Scene::_DrawBouncingCubes(
    const glm::mat4& modelview,
    glm::vec3 center,
    float radius,
    float scale) const
{
    const glm::mat4 ringCenter = glm::translate(modelview, center);

    const int numCubes = 12;
    for (int i=0; i<numCubes; ++i)
    {
        const float frequency = 3.0f;
        const float posPhase = 2.0f * (float)M_PI * (float)i / (float)numCubes;
        const float oscVal = m_amplitude * sin(frequency * (m_phaseVal + posPhase));

        glm::mat4 sinmtx = glm::rotate(ringCenter, posPhase, glm::vec3(0.0f, 1.0f, 0.0f));
        sinmtx = glm::translate(
            sinmtx,
            glm::vec3(0.0f, oscVal, radius));
        sinmtx = glm::scale(sinmtx, glm::vec3(scale));

        glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(sinmtx));
        DrawColorCube();
    }
}


void Scene::_DrawScenePlanes(const glm::mat4& modelview) const
{
    m_plane.bindVAO();
    {
        // floor
        glDrawElements(GL_TRIANGLES,
                       3*2, // 2 triangle pairs
                       GL_UNSIGNED_INT,
                       0);

        const float ceilHeight = 3.0f;
        glm::mat4 ceilmtx = glm::translate(
            modelview,
            glm::vec3(0.0f, ceilHeight, 0.0f));

        glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(ceilmtx));

        // ceiling
        glDrawElements(GL_TRIANGLES,
                       3*2, // 2 triangle pairs
                       GL_UNSIGNED_INT,
                       0);
    }
    glBindVertexArray(0);
}


/// Draw the scene(matrices have already been set up).
void Scene::DrawScene(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const glm::mat4& object) const
{
    glUseProgram(m_plane.prog());
    {
        glUniformMatrix4fv(m_plane.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_plane.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

        _DrawScenePlanes(modelview);
    }
    glUseProgram(0);

    glUseProgram(m_basic.prog());
    {
        glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_basic.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

        _DrawBouncingCubes(modelview, glm::vec3(0.0f, 1.0f, 0.5f), 0.25f, 0.064f);
        _DrawBouncingCubes(modelview, glm::vec3(0.0f, 0.0f, 0.5f), 1.5f, 0.5f);

        (void)object;
#if 0
        glm::mat4 objectMatrix = modelview;
        objectMatrix = glm::translate(objectMatrix, glm::vec3(0.0f, 1.0f, 0.0f)); // Raise rotation center above floor
        // Rotate about cube center
        objectMatrix = glm::translate(objectMatrix, glm::vec3(0.5f));
        objectMatrix *= object;
        objectMatrix = glm::translate(objectMatrix, glm::vec3(-0.5f));
        glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(objectMatrix));
        DrawColorCube();
#endif
    }
    glUseProgram(0);
}


void Scene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);
    const glm::mat4 object = glm::mat4(1.0f);

    DrawScene(modelview, projection, object);
}

void Scene::timestep(double /*absTime*/, double dt)
{
    m_phaseVal += static_cast<float>(dt);
}

// Check for hits against floor plane
bool Scene::RayIntersects(
    const float* pRayOrigin,
    const float* pRayDirection,
    float* pTParameter, // [inout]
    float* pHitLocation, // [inout]
    float* pHitNormal // [inout]
    ) const
{
    const glm::vec3 origin3 = glm::make_vec3(pRayOrigin);
    const glm::vec3 dir3 = glm::make_vec3(pRayDirection);

    const glm::vec3 minPt(-10.f, 0.f, -10.f);
    const glm::vec3 maxPt( 10.f, 0.f,  10.f);

    std::vector<glm::vec3> pts;
    pts.push_back(glm::vec3(minPt.x, minPt.y, minPt.z));
    pts.push_back(glm::vec3(minPt.x, minPt.y, maxPt.z));
    pts.push_back(glm::vec3(maxPt.x, minPt.y, maxPt.z));
    pts.push_back(glm::vec3(maxPt.x, minPt.y, minPt.z));

    glm::vec3 retval1(0.0f);
    glm::vec3 retval2(0.0f);
    const bool hit1 = glm::intersectLineTriangle(origin3, dir3, pts[0], pts[1], pts[2], retval1);
    const bool hit2 = glm::intersectLineTriangle(origin3, dir3, pts[0], pts[2], pts[3], retval2);
    if ( !(hit1||hit2) )
        return false;

    glm::vec3 cartesianpos(0.f);
    if (hit1)
    {
        // At this point, retval1 or retval2 contains hit data returned from glm::intersectLineTriangle.
        // This does not appear to be raw - y and z appear to be barycentric coordinates.
        // X coordinate of retval1 appears to be the t parameter of the intersection point along dir3.
        // Fill out the x coord with the barycentric identity then convert using simple weighted sum.
        if (retval1.x < 0.f) // Hit behind origin
            return false;
        *pTParameter = retval1.x;
        const float bary_x = 1.f - retval1.y - retval1.z;
        cartesianpos = 
               bary_x * pts[0] +
            retval1.y * pts[1] +
            retval1.z * pts[2];
    }
    else if (hit2)
    {
        if (retval2.x < 0.f) // Hit behind origin
            return false;
        *pTParameter = retval2.x;
        const float bary_x = 1.f - retval2.y - retval2.z;
        cartesianpos = 
               bary_x * pts[0] +
            retval2.y * pts[2] +
            retval2.z * pts[3];
    }

    const glm::vec3 hitPos = origin3 + *pTParameter * dir3;
    pHitLocation[0] = hitPos.x;
    pHitLocation[1] = hitPos.y;
    pHitLocation[2] = hitPos.z;

    pHitNormal[0] = 0.f;
    pHitNormal[1] = 1.f;
    pHitNormal[2] = 0.f;

    return true;
}

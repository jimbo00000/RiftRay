// OVRScene.cpp

#include "OVRScene.h"

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

#include <OVR_CAPI.h>
#include "OVR.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include "Logger.h"

OVRScene::OVRScene()
: m_basic()
, m_pHmd(NULL)
, m_pPos(NULL)
, m_pYaw(NULL)
, m_frustumVerts()
, m_distanceToFrustum(999.0f)
, m_tanFromCameraCenterline(0.0f, 0.0f)
, m_tanHalfFov(1.0f, 1.0f)
{
}

OVRScene::~OVRScene()
{
}

void OVRScene::initGL()
{
    m_basic.initProgram("ucolor");
    m_basic.bindVAO();
    _InitFrustumAttributes();
    glBindVertexArray(0);
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void OVRScene::_InitFrustumAttributes()
{
    float hfovRad = 1.0f;
    float vfovRad = 1.0f;
    float znear = 0.5f;
    float zfar = 2.0f;
    if (m_pHmd != NULL)
    {
        hfovRad = m_pHmd->CameraFrustumHFovInRadians;
        vfovRad = m_pHmd->CameraFrustumVFovInRadians;
        znear = m_pHmd->CameraFrustumNearZInMeters;
        zfar = m_pHmd->CameraFrustumFarZInMeters;
    }
    // Construct tracking frustum geometry
    m_tanHalfFov.x = tan(hfovRad * 0.5f);
    m_tanHalfFov.y = tan(vfovRad * 0.5f);
    const float xn = znear * m_tanHalfFov.x;
    const float yn = znear * m_tanHalfFov.y;
    const float xf = zfar * m_tanHalfFov.x;
    const float yf = zfar * m_tanHalfFov.y;

    const glm::vec3 verts[] = {
        glm::vec3(0, 0, 0),
        glm::vec3(0, 0, 0.25f),

        glm::vec3(-xn, -yn, znear),
        glm::vec3(-xn,  yn, znear),
        glm::vec3( xn,  yn, znear),
        glm::vec3( xn, -yn, znear),

        glm::vec3(-xf, -yf, zfar),
        glm::vec3(-xf,  yf, zfar),
        glm::vec3( xf,  yf, zfar),
        glm::vec3( xf, -yf, zfar),
    };

    for (int i=0; i<sizeof(verts)/sizeof(glm::vec3); ++i)
    {
        m_frustumVerts.push_back(verts[i]);
    }

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_basic.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, m_frustumVerts.size()*sizeof(glm::vec3), &m_frustumVerts[0], GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_basic.GetAttrLoc("vPosition"));

    const unsigned int lines[] = {
        0,1,
        0,6,0,7,0,8,0,9,
        2,3,3,4,4,5,5,2,
        6,7,7,8,8,9,9,6,
    };
    GLuint quadVbo = 0;
    glGenBuffers(1, &quadVbo);
    m_basic.AddVbo("elements", quadVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);
}

void OVRScene::_DrawFrustum() const
{
    m_basic.bindVAO();
    glDrawElements(GL_LINES,
                   13*2,
                   GL_UNSIGNED_INT,
                   0);
    glBindVertexArray(0);
}

void OVRScene::DrawScene(
    const glm::mat4& modelview,
    const glm::mat4& projection) const
{
    glUseProgram(m_basic.prog());
    {
        glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_basic.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

        // If we are within the warning track, draw the frustum.
        // Color it closer to red as we get closer to it.
        const float warningTrackTan = 0.25f;
        const float delta = std::max(
            m_tanFromCameraCenterline.x - m_tanHalfFov.x + warningTrackTan,
            m_tanFromCameraCenterline.y - m_tanHalfFov.y + warningTrackTan
            );
        if (delta > 0.0f)
        {
            const glm::vec4 col(5.0f * delta, 0.0f, 0.0f, 1.0f);
            glUniform4fv(m_basic.GetUniLoc("u_Color"), 1, glm::value_ptr(col));

            _DrawFrustum();
        }
    }
    glUseProgram(0);
}

void OVRScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);

    // Assemble modelview matrix to lock camera in with real world geometry:
    // We still have to use the assembled HMD stereo modelview matrices from RiftAppSkeleton,
    // but we undo the effects of chassis yaw and position so the frustum follows the viewer.
    if (m_pHmd != NULL)
    {
        const ovrTrackingState ts = ovrHmd_GetTrackingState(m_pHmd, ovr_GetTimeInSeconds());
        const ovrPosef& cp = ts.CameraPose;

        OVR::Matrix4f camMtx = OVR::Matrix4f();

        // Construct the matrix as the reverse of the one in RiftAppSkeleton::display_client
        if (m_pPos != NULL)
            camMtx *= OVR::Matrix4f::Translation(OVR::Vector3f(*m_pPos));
        if (m_pYaw != NULL)
            camMtx *= OVR::Matrix4f::RotationY(-*m_pYaw);

        camMtx *= OVR::Matrix4f::Translation(cp.Position)
            * OVR::Matrix4f(OVR::Quatf(cp.Orientation));

        glm::mat4 ogmat = glm::make_mat4(&camMtx.Transposed().M[0][0]);

        glLineWidth(3.0f);
        DrawScene(modelview * ogmat, projection);
        glLineWidth(1.0f);
    }
}

void OVRScene::timestep(float dt)
{
    (void)dt;

    const ovrTrackingState ts = ovrHmd_GetTrackingState(m_pHmd, ovr_GetTimeInSeconds());
    const ovrVector3f& hp = ts.HeadPose.ThePose.Position;
    glm::vec4 headPt(hp.x, hp.y, hp.z, 1.0f);

    // Get camera pose as a matrix
    const ovrPosef& cp = ts.CameraPose;
    OVR::Matrix4f camMtx = OVR::Matrix4f();
    camMtx *= OVR::Matrix4f::Translation(cp.Position)
        * OVR::Matrix4f(OVR::Quatf(cp.Orientation));

    const glm::mat4 gcamMtx = glm::make_mat4(&camMtx.Inverted().Transposed().M[0][0]);
    headPt = gcamMtx * headPt;
    m_tanFromCameraCenterline.x = fabs(headPt.x / headPt.z);
    m_tanFromCameraCenterline.y = fabs(headPt.y / headPt.z);

#if 0
    std::vector<glm::vec3> txFrustumPts = m_frustumVerts;
    for (std::vector<glm::vec3>::const_iterator it = txFrustumPts.begin();
        it != txFrustumPts.end();
        ++it)
    {
        glm::vec3 pt = *it;
        glm::vec4 pt4(pt, 1.0f);
        pt4 = gcamMtx * pt4;
        pt.x = pt4.x;
        pt.y = pt4.y;
        pt.z = pt4.z;
    }

    // Calculate minimum distance to frustum
    std::vector<glm::ivec3> planeIndices;
    planeIndices.push_back(glm::ivec3(2, 3, 4));
    planeIndices.push_back(glm::ivec3(8, 7, 6));
    planeIndices.push_back(glm::ivec3(2, 3, 7));
    planeIndices.push_back(glm::ivec3(3, 4, 8));
    planeIndices.push_back(glm::ivec3(4, 5, 9));
    planeIndices.push_back(glm::ivec3(5, 2, 6));

    float minDist = 999.0f;
    for (std::vector<glm::ivec3>::const_iterator it = planeIndices.begin();
        it != planeIndices.end();
        ++it)
    {
        const glm::ivec3& idxs = *it;
        // Assume this point has already been transformed
        // If our indices are out of bounds, we're hosed
        const glm::vec3& p1 = txFrustumPts[idxs.x];
        const glm::vec3& p2 = txFrustumPts[idxs.y];
        const glm::vec3& p3 = txFrustumPts[idxs.z];
        const glm::vec3 v1 = p1 - p2;
        const glm::vec3 v2 = p3 - p2;
        const glm::vec3 norm = glm::normalize(glm::cross(v1, v2));
        const glm::vec3 ptDist = headPt - p2;
        const float dist = fabs(glm::dot(norm, ptDist)); // shouldn't need fabs if ordering is correct
        minDist = std::min(minDist, dist);
    }
    m_distanceToFrustum = minDist;
#endif
}

// HudQuad.cpp

#include "HudQuad.h"
#include "Logger.h"
#include "MatrixFunctions.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

HudQuad::HudQuad()
: m_QuadPoseCenter()
, m_showQuadInWorld(true)
, m_quadSize(.5f)
, m_holding(false)
, m_hitPtPositionAtGrab(0.f)
, m_hitPtTParam(-1.f)
{
}

HudQuad::~HudQuad()
{
}

void HudQuad::initGL(ovrSession& session, ovrSizei sz)
{
    m_session = session; ///@todo Make this a parameter to draw func

    m_QuadPoseCenter.Orientation = //{ 0.f, 0.f, 0.f, 1.f };
        { 0.129206583f, 0.0310291424f, 0.000810863741f, -0.991131783f };
    m_QuadPoseCenter.Position = { 0.f, -.375f, -.75f };

    const ovrSizei& bufferSize = { 600, 600 };

    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.Width = bufferSize.w;
    desc.Height = bufferSize.h;
    desc.MipLevels = 1;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    // Allocate the frameBuffer that will hold the scene, and then be
    // re-rendered to the screen with distortion
    if (ovr_CreateTextureSwapChainGL(session, &desc, &m_swapChain) == ovrSuccess)
    {
        int length = 0;
        ovr_GetTextureSwapChainLength(session, m_swapChain, &length);

        for (int i = 0; i < length; ++i)
        {
            GLuint chainTexId;
            ovr_GetTextureSwapChainBufferGL(session, m_swapChain, i, &chainTexId);
            glBindTexture(GL_TEXTURE_2D, chainTexId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
    else
    {
        LOG_ERROR("HudQuad::initGL Unable to create swap textures");
        return;
    }

    // Manually assemble swap FBO
    FBO& swapfbo = m_fbo;
    swapfbo.w = bufferSize.w;
    swapfbo.h = bufferSize.h;
    glGenFramebuffers(1, &swapfbo.id);
    glBindFramebuffer(GL_FRAMEBUFFER, swapfbo.id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, swapfbo.tex, 0);

    swapfbo.depth = 0;
    glGenRenderbuffers(1, &swapfbo.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, swapfbo.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, bufferSize.w, bufferSize.h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, swapfbo.depth);

    // Check status
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("Framebuffer status incomplete: %d %x", status, status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HudQuad::exitGL(ovrSession& session)
{
    ovr_DestroyTextureSwapChain(session, m_swapChain);
    FBO& f = m_fbo;
    glDeleteFramebuffers(1, &f.id), f.id = 0;
    glDeleteRenderbuffers(1, &f.depth), f.depth = 0;
}

///@brief Called from the UI to indicate whether the user is holding and dragging
/// the quad around in world space.
void HudQuad::SetHoldingFlag(ovrPosef pose, bool f)
{
    if (f == false)
    {
        m_holding = false;
        return;
    }
    glm::vec3 ro, rd;
    GetHMDEyeRayPosAndDir(pose, ro, rd);
    const glm::mat4 quadposeMatrix = makeMatrixFromPose(GetPose());

    glm::vec2 planePt;
    float tParam;
    const bool hit = GetPaneRayIntersectionCoordinates(quadposeMatrix, ro, rd, planePt, tParam);
    if (hit == true)
    {
        if ((f == true) && (m_holding == false))
        {
            // Just grabbed; store quad's pose at start
            m_holding = true;
            m_hitPtTParam = tParam;
            const ovrVector3f& tx = m_QuadPoseCenter.Position;
            m_planePositionAtGrab = glm::vec3(tx.x, tx.y, tx.z);
            m_hitPtPositionAtGrab = ro + m_hitPtTParam*rd;
        }
    }
}

void HudQuad::_PrepareToDrawToQuad() const
{
    const FBO& f = m_fbo;
    const ovrTextureSwapChain& chain = m_swapChain;
    const ovrSession& session = m_session;

    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(session, chain, &curIndex);
    GLuint curTexId;
    ovr_GetTextureSwapChainBufferGL(session, chain, curIndex, &curTexId);

    glBindFramebuffer(GL_FRAMEBUFFER, f.id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
    glViewport(0, 0, f.w, f.h);
}

void HudQuad::DrawToQuad()
{
    _PrepareToDrawToQuad();
    {
        const float g = .3f;
        glClearColor(g, g, g, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HudQuad::_FinalizeDrawToQuad()
{
    ovr_CommitTextureSwapChain(m_session, m_swapChain);
}

///@param [out] planePtOut Intersection point on plane in local normalized coordinates
///@param [out] tParamOut T parameter value along intersection ray
///@return true if ray hits pane quad, false otherwise
bool HudQuad::GetPaneRayIntersectionCoordinates(
    const glm::mat4& quadPoseMatrix, ///< [in] Quad's pose in world space
    glm::vec3 origin3, ///< [in] Ray origin
    glm::vec3 dir3, ///< [in] Ray direction(normalized)
    glm::vec2& planePtOut, ///< [out] Intersection point in XY plane coordinates
    float& tParamOut) ///< [out] t parameter of ray intersection (ro + t*dt)
{
    if (m_showQuadInWorld == false)
        return false;

    // Standard Oculus quad layer coordinates
    glm::vec3 pts[] = {
        glm::vec3(-.5f*m_quadSize.x, -.5f*m_quadSize.y, 0.f),
        glm::vec3( .5f*m_quadSize.x, -.5f*m_quadSize.y, 0.f),
        glm::vec3( .5f*m_quadSize.x,  .5f*m_quadSize.y, 0.f),
        glm::vec3(-.5f*m_quadSize.x,  .5f*m_quadSize.y, 0.f),
    };
    for (int i = 0; i < 4; ++i)
    {
        glm::vec4 p4 = glm::vec4(pts[i], 1.f);
        p4 = quadPoseMatrix * p4;
        pts[i] = glm::vec3(p4);
    }

    glm::vec3 retval1(0.f);
    glm::vec3 retval2(0.f);
    const bool hit1 = glm::intersectLineTriangle(origin3, dir3, pts[0], pts[1], pts[2], retval1);
    const bool hit2 = glm::intersectLineTriangle(origin3, dir3, pts[0], pts[2], pts[3], retval2);
    if (!(hit1 || hit2))
        return false;

    glm::vec3 hitval(0.f);
    glm::vec3 cartesianpos(0.f);
    if (hit1)
    {
        hitval = retval1;
        // At this point, retval1 or retval2 contains hit data returned from glm::intersectLineTriangle.
        // This does not appear to be raw - y and z appear to be barycentric coordinates.
        // Fill out the x coord with the barycentric identity then convert using simple weighted sum.
        cartesianpos =
            (1.f - hitval.y - hitval.z) * pts[0] +
            hitval.y * pts[1] +
            hitval.z * pts[2];
    }
    else if (hit2)
    {
        hitval = retval2;
        cartesianpos =
            (1.f - hitval.y - hitval.z) * pts[0] +
            hitval.y * pts[2] +
            hitval.z * pts[3];
    }

    // Store the t param along controller ray of the hit in the Transformation
    // Did you know that x stores the t param val? I couldn't find this in docs anywhere.
    const float tParam = hitval.x;
    tParamOut = tParam;
    if (tParam < 0.f)
        return false; // Behind the origin

    const glm::vec3 v1 = pts[1] - pts[0]; // x axis
    const glm::vec3 v2 = pts[3] - pts[0]; // y axis
    const float len = glm::length(v1); // v2 length should be equal
    const glm::vec3 vh = (cartesianpos - pts[0]) / len;
    planePtOut = glm::vec2(
        glm::dot(v1 / len, vh),
        1.f - glm::dot(v2 / len, vh) // y coord flipped by convention
        );

    return true;
}

///@brief Update the latest position of the HMD - used for grabbing the quad
/// with a key then glancing while holding it to move the quad in space.
///@note Writes to m_layerQuad.QuadPoseCenter
void HudQuad::SetHmdEyeRay(ovrPosef pose)
{
    glm::vec3 ro, rd;
    GetHMDEyeRayPosAndDir(pose, ro, rd);

    if (m_holding == true)
    {
        const glm::vec3 rayPt = ro + m_hitPtTParam * rd;
        const glm::vec3 movement = rayPt - m_hitPtPositionAtGrab;
        const glm::vec3 quadLocation = m_planePositionAtGrab + movement;
        m_QuadPoseCenter.Position.x = quadLocation.x;
        m_QuadPoseCenter.Position.y = quadLocation.y;
        m_QuadPoseCenter.Position.z = quadLocation.z;
        m_QuadPoseCenter.Orientation = pose.Orientation;
    }
}

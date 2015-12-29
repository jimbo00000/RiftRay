// HudQuad.cpp

#include "HudQuad.h"
#include "Logger.h"
#include "MatrixFunctions.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

HudQuad::HudQuad()
: m_layerQuad()
, m_pQuadTex(NULL)
, m_showQuadInWorld(true)
, m_quadLocation(0.f)
, m_holding(false)
, m_hitPtPositionAtGrab(0.f)
, m_hitPtTParam(-1.f)
{
}

HudQuad::~HudQuad()
{
}

void HudQuad::initGL(ovrHmd hmd, ovrSizei sz)
{
    if (!OVR_SUCCESS(ovr_CreateSwapTextureSetGL(hmd, GL_RGBA, sz.w, sz.h, &m_pQuadTex)))
    {
        LOG_ERROR("Unable to create quad layer swap tex");
        return;
    }

    const ovrSwapTextureSet& swapSet = *m_pQuadTex;
    const ovrGLTexture& ovrTex = (ovrGLTexture&)swapSet.Textures[0];
    glBindTexture(GL_TEXTURE_2D, ovrTex.OGL.TexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    ovrLayerQuad& layer = m_layerQuad;
    layer.Header.Type = ovrLayerType_Quad;
    layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
    layer.ColorTexture = m_pQuadTex;
    layer.Viewport.Pos = { 0, 0 };
    layer.Viewport.Size = sz;

    layer.QuadPoseCenter.Orientation = { 0.f, 0.f, 0.f, 1.f };
    layer.QuadPoseCenter.Position = {-.5f, .3f, -1.f};

    m_quadLocation.x = layer.QuadPoseCenter.Position.x;
    m_quadLocation.y = layer.QuadPoseCenter.Position.y;
    m_quadLocation.z = layer.QuadPoseCenter.Position.z;

    layer.QuadSize = { 1.f, 1.f };

    // Manually assemble quad m_fbo
    m_fbo.w = sz.w;
    m_fbo.h = sz.h;
    glGenFramebuffers(1, &m_fbo.id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.id);
    const int idx = 0;
    const ovrGLTextureData* pGLData = reinterpret_cast<ovrGLTextureData*>(&swapSet.Textures[0]);
    m_fbo.tex = pGLData->TexId;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo.tex, 0);

    m_fbo.depth = 0;
    glGenRenderbuffers(1, &m_fbo.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, sz.w, sz.h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fbo.depth);

    // Check status
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERROR("Framebuffer status incomplete: %d %x", status, status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HudQuad::exitGL(ovrHmd hmd)
{
    ovr_DestroySwapTextureSet(hmd, m_pQuadTex);
    m_pQuadTex = nullptr;
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
            m_planePositionAtGrab = m_quadLocation;
            m_hitPtPositionAtGrab = ro + m_hitPtTParam*rd;
        }
    }
}

void HudQuad::_PrepareToDrawToQuad() const
{
    const ovrSwapTextureSet& swapSet = *m_pQuadTex;
    const FBO& f = m_fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, f.id);
    const ovrGLTexture& tex = (ovrGLTexture&)(swapSet.Textures[swapSet.CurrentIndex]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.OGL.TexId, 0);

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
        glm::vec3(-.5f, -.5f, 0.f),
        glm::vec3( .5f, -.5f, 0.f),
        glm::vec3( .5f,  .5f, 0.f),
        glm::vec3(-.5f,  .5f, 0.f),
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
        m_quadLocation = m_planePositionAtGrab + movement;
        m_layerQuad.QuadPoseCenter.Position.x = m_quadLocation.x;
        m_layerQuad.QuadPoseCenter.Position.y = m_quadLocation.y;
        m_layerQuad.QuadPoseCenter.Position.z = m_quadLocation.z;
        m_layerQuad.QuadPoseCenter.Orientation = pose.Orientation;
    }
}

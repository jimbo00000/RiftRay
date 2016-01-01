// main_glfw_ovrsdk08.cpp
// With humongous thanks to cThrough 2014 (Daniel Dekkers)
// Get a window created with GL context and OVR backend initialized,
// then hand off display to the Scene class.

#include <GL/glew.h>
#if defined(_WIN32)
#  include <Windows.h>
#endif
#include <GLFW/glfw3.h>
#include <OVR.h>
#include <Kernel/OVR_Types.h> // Pull in OVR_OS_* defines 
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "FBO.h"
#include "Timer.h"
#include "Logger.h"

#include "Scene.h"
#include "ShaderGalleryScene.h"

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include "HudQuad.h"
#include "AntQuad.h"
#include "MatrixFunctions.h"

Timer g_timer;
double g_lastFrameTime = 0.0;
GLFWwindow* g_pMirrorWindow = NULL;
glm::ivec2 g_mirrorWindowSz(1200, 900);

ovrHmd m_Hmd;
ovrEyeRenderDesc m_eyeRenderDescs[ovrEye_Count];
ovrVector3f m_eyeOffsets[ovrEye_Count];
glm::mat4 m_eyeProjections[ovrEye_Count];

ovrPosef m_eyePoses[ovrEye_Count];
ovrLayerEyeFov m_layerEyeFov;
int m_frameIndex = 0;
ovrTexture* m_pMirrorTex = NULL;
FBO m_mirrorFBO;
ovrSwapTextureSet* m_pTexSet[ovrEye_Count];
ovrPerfHudMode m_perfHudMode = ovrPerfHud_Off;
FBO m_swapFBO;
FBO m_undistortedFBO;

IScene* g_pScene = NULL;
ShaderGalleryScene g_gallery;

AntQuad g_tweakbarQuad;
#ifdef USE_ANTTWEAKBAR
TwBar* g_pMainTweakbar = NULL;
TwBar* g_pShaderTweakbar = NULL;
#endif

float m_fboScale = 1.f;
float m_cinemaScope = 0.f;

int m_keyStates[GLFW_KEY_LAST];
glm::vec3 m_keyboardMove(0.f);
glm::vec3 m_chassisPos(0.f);
float m_chassisYaw = 0.f;
glm::vec3 m_hmdRo;
glm::vec3 m_hmdRd;

void initAnt()
{
    ///@note Bad size errors will be thrown if this is not called before bar creation.
    TwWindowSize(g_mirrorWindowSz.x, g_mirrorWindowSz.y);

    TwDefine(" GLOBAL fontsize=3 ");
    TwDefine(" TweakBar size='300 520' ");

    // Create a tweak bar
    g_pMainTweakbar = TwNewBar("TweakBar");
    g_pShaderTweakbar = TwNewBar("ShaderTweakBar");
    g_gallery.m_pMainTweakbar = g_pMainTweakbar;
    g_gallery.m_pShaderTweakbar = g_pShaderTweakbar;

    TwDefine(" GLOBAL fontsize=3 ");
    TwDefine(" TweakBar size='300 580' ");
    TwDefine(" TweakBar position='10 10' ");
    TwDefine(" ShaderTweakBar size='300 420' ");
    TwDefine(" ShaderTweakBar position='290 170' ");

    TwAddVarRW(g_pMainTweakbar, "FBO Scale", TW_TYPE_FLOAT, &m_fboScale,
        " min=0.05 max=1.0 step=0.005 group='Performance' ");
    TwAddVarRW(g_pMainTweakbar, "Cinemascope", TW_TYPE_FLOAT, &m_cinemaScope,
        " min=0.05 max=1.0 step=0.005 group='Performance' ");
}

///@brief Can be called before GL context is initialized.
void initHMD()
{
    const ovrResult res = ovr_Initialize(NULL);
    if (ovrSuccess != res)
    {
        LOG_ERROR("ovr_Initialize failed with code %d", res);
    }

    ovrGraphicsLuid luid;
    if (ovrSuccess != ovr_Create(&m_Hmd, &luid))
    {
        LOG_ERROR("ovr_Create failed with code %d", res);
    }

    const ovrResult ret = ovr_ConfigureTracking(m_Hmd,
        ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position,
        ovrTrackingCap_Orientation);
    if (!OVR_SUCCESS(ret))
    {
        LOG_ERROR("Error calling ovr_ConfigureTracking");
    }
}

///@brief Called once a GL context has been set up.
void initVR()
{
    if (m_Hmd == NULL)
        return;

    // Set up eye view parameters
    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        ovrEyeRenderDesc& erd = m_eyeRenderDescs[eye];
        const ovrHmdDesc& hmd = ovr_GetHmdDesc(m_Hmd);
        erd = ovr_GetRenderDesc(m_Hmd, eye, hmd.MaxEyeFov[eye]);

        m_eyeOffsets[eye] = erd.HmdToEyeViewOffset;
        const ovrMatrix4f ovrPerspectiveProjection = ovrMatrix4f_Projection(
            erd.Fov, .1f, 10000.f, ovrProjection_RightHanded);
        m_eyeProjections[eye] = glm::transpose(glm::make_mat4(&ovrPerspectiveProjection.M[0][0]));
    }

    // Create eye render target textures and FBOs
    ovrLayerEyeFov& layer = m_layerEyeFov;
    layer.Header.Type = ovrLayerType_EyeFov;
    layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        const ovrHmdDesc& hmd = ovr_GetHmdDesc(m_Hmd);
        const ovrFovPort& fov = layer.Fov[eye] = hmd.MaxEyeFov[eye];
        const ovrSizei& size = layer.Viewport[eye].Size = ovr_GetFovTextureSize(m_Hmd, eye, fov, 1.f);
        layer.Viewport[eye].Pos = { 0, 0 };
        LOG_INFO("Eye %d tex : %dx%d @ (%d,%d)", eye, size.w, size.h,
            layer.Viewport[eye].Pos.x, layer.Viewport[eye].Pos.y);

        // Allocate the frameBuffer that will hold the scene, and then be
        // re-rendered to the screen with distortion
        if (!OVR_SUCCESS(ovr_CreateSwapTextureSetGL(m_Hmd, GL_SRGB8_ALPHA8, size.w, size.h, &m_pTexSet[eye])))
        {
            LOG_ERROR("Unable to create swap textures");
            return;
        }
        const ovrSwapTextureSet& swapSet = *m_pTexSet[eye];
        for (int i = 0; i < swapSet.TextureCount; ++i)
        {
            const ovrGLTexture& ovrTex = (ovrGLTexture&)swapSet.Textures[i];
            glBindTexture(GL_TEXTURE_2D, ovrTex.OGL.TexId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        // Manually assemble swap FBO
        m_swapFBO.w = size.w;
        m_swapFBO.h = size.h;
        glGenFramebuffers(1, &m_swapFBO.id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_swapFBO.id);
        const int idx = 0;
        const ovrGLTextureData* pGLData = reinterpret_cast<ovrGLTextureData*>(&swapSet.Textures[idx]);
        m_swapFBO.tex = pGLData->TexId;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_swapFBO.tex, 0);

        m_swapFBO.depth = 0;
        glGenRenderbuffers(1, &m_swapFBO.depth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_swapFBO.depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.w, size.h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_swapFBO.depth);

        // Check status
        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR("Framebuffer status incomplete: %d %x", status, status);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        layer.ColorTexture[eye] = m_pTexSet[eye];
    }

    // Initialize distorted mirror texture
    const ovrEyeType eye = ovrEyeType::ovrEye_Left;
    const ovrHmdDesc& hmd = ovr_GetHmdDesc(m_Hmd);
    const ovrFovPort& fov = layer.Fov[eye] = hmd.MaxEyeFov[eye];
    const ovrSizei& size = layer.Viewport[eye].Size = ovr_GetFovTextureSize(m_Hmd, eye, fov, 1.f);
    const ovrResult result = ovr_CreateMirrorTextureGL(m_Hmd, GL_RGBA, size.w, size.h, &m_pMirrorTex);
    if (!OVR_SUCCESS(result))
    {
        LOG_ERROR("Unable to create mirror texture");
        return;
    }

    // Manually assemble mirror FBO
    m_mirrorFBO.w = size.w;
    m_mirrorFBO.h = size.h;
    glGenFramebuffers(1, &m_mirrorFBO.id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_mirrorFBO.id);
    const ovrGLTextureData* pMirrorGLData = reinterpret_cast<ovrGLTextureData*>(m_pMirrorTex);
    m_mirrorFBO.tex = pMirrorGLData->TexId;
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mirrorFBO.tex, 0);



    // Create another FBO for blitting the undistorted scene to for desktop window display.
    m_undistortedFBO.w = size.w;
    m_undistortedFBO.h = size.h;
    glGenFramebuffers(1, &m_undistortedFBO.id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_undistortedFBO.id);
    glGenTextures(1, &m_undistortedFBO.tex);
    glBindTexture(GL_TEXTURE_2D, m_undistortedFBO.tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        m_undistortedFBO.w, m_undistortedFBO.h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_undistortedFBO.tex, 0);

    // Check status
    {
        const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR("Framebuffer status incomplete: %d %x", status, status);
        }
    }




    const ovrSizei sz = { 600, 600 };
    g_tweakbarQuad.initGL(m_Hmd, sz);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::vec3 getChassisWorldPos()
{
    return //glm::vec3(0.f, 1.f, 1.f) + 
        m_chassisPos;
}

glm::mat4 makeWorldToChassisMatrix()
{
    return makeChassisMatrix_glm(m_chassisYaw, 0.f, 0.f, m_chassisPos);
}

void storeHmdPose(const ovrPosef& eyePose)
{
    m_hmdRo.x = eyePose.Position.x + m_chassisPos.x;
    m_hmdRo.y = eyePose.Position.y + m_chassisPos.y;
    m_hmdRo.z = eyePose.Position.z + m_chassisPos.z;

    const float m_headSize = 1.f;
    const glm::mat4 w2eye = makeWorldToChassisMatrix() * makeMatrixFromPose(eyePose, m_headSize);
    const OVR::Matrix4f rotmtx = makeOVRMatrixFromGlmMatrix(w2eye);
    const OVR::Vector4f lookFwd(0.f, 0.f, -1.f, 0.f);
    const OVR::Vector4f rotvec = rotmtx.Transform(lookFwd);
    m_hmdRd.x = rotvec.x;
    m_hmdRd.y = rotvec.y;
    m_hmdRd.z = rotvec.z;
}

void BlitLeftEyeRenderToUndistortedMirrorTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_swapFBO.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_undistortedFBO.id);
    glViewport(0, 0, m_undistortedFBO.w, m_undistortedFBO.h);
    const float fboScale = 1.f; //m_fboScale
    glBlitFramebuffer(
        0, static_cast<int>(static_cast<float>(m_swapFBO.h)*fboScale),
        static_cast<int>(static_cast<float>(m_swapFBO.w)*fboScale), 0, ///@todo Fix for FBO scaling
        0, 0, m_undistortedFBO.w, m_undistortedFBO.h,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_swapFBO.id);
}

// Display the old-fashioned way, to a monoscopic viewport on a desktop monitor.
void displayMonitor()
{
    if (g_pScene == NULL)
        return;

    const glm::mat4 mview = glm::translate(glm::mat4(1.f), -getChassisWorldPos());
    const glm::ivec2 vp = g_mirrorWindowSz;
    const glm::mat4 persp = glm::perspective(
        90.f,
        static_cast<float>(vp.x) / static_cast<float>(vp.y),
        .004f,
        500.f);
    glViewport(0, 0, vp.x, vp.y);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    g_pScene->RenderForOneEye(glm::value_ptr(mview), glm::value_ptr(persp));

    m_hmdRo = m_chassisPos;
    m_hmdRd = glm::vec3(0.f, 0.f, -1.f);
}

// Display to an HMD with OVR SDK backend.
void displayHMD()
{
    const ovrHmd hmd = m_Hmd;
    if (hmd == NULL)
    {
        displayMonitor();
        return;
    }

    ovrTrackingState outHmdTrackingState = { 0 };
    ovr_GetEyePoses(hmd, m_frameIndex, false, m_eyeOffsets,
        m_eyePoses, &outHmdTrackingState);

    storeHmdPose(m_eyePoses[0]);

    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        const ovrSwapTextureSet& swapSet = *m_pTexSet[eye];
        glBindFramebuffer(GL_FRAMEBUFFER, m_swapFBO.id);
        const ovrGLTexture& tex = (ovrGLTexture&)(swapSet.Textures[swapSet.CurrentIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.OGL.TexId, 0);
        {
            // Handle render target resolution scaling
            m_layerEyeFov.Viewport[eye].Size = ovr_GetFovTextureSize(m_Hmd, eye, m_layerEyeFov.Fov[eye], m_fboScale);
            ovrRecti& vp = m_layerEyeFov.Viewport[eye];
            if (m_layerEyeFov.Header.Flags & ovrLayerFlag_TextureOriginAtBottomLeft)
            {
                ///@note It seems that the render viewport should be vertically centered within the swapSet texture.
                /// See also OculusWorldDemo.cpp:1443 - "The usual OpenGL viewports-don't-match-UVs oddness."
                const int texh = swapSet.Textures[swapSet.CurrentIndex].Header.TextureSize.h;
                vp.Pos.y = (texh - vp.Size.h) / 2;
            }
            glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);

            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Cinemascope - letterbox bars scissoring off pixels above and below vp center
            const float hc = .5f * m_cinemaScope;
            const int scisPx = static_cast<int>(hc * static_cast<float>(vp.Size.h));
            ovrRecti sp(vp);
            sp.Pos.y += scisPx;
            sp.Size.h -= 2 * scisPx;
            glScissor(sp.Pos.x, sp.Pos.y, sp.Size.w, sp.Size.h);
            glEnable(GL_SCISSOR_TEST);
            glEnable(GL_DEPTH_TEST);

            // Render the scene for the current eye
            const ovrPosef& eyePose = m_eyePoses[eye];
            const glm::mat4 mview =
                makeWorldToChassisMatrix() *
                makeMatrixFromPose(eyePose);
            const glm::mat4& proj = m_eyeProjections[eye];
            g_pScene->RenderForOneEye(glm::value_ptr(glm::inverse(mview)), glm::value_ptr(proj));

            m_layerEyeFov.RenderPose[eye] = eyePose;
        }
        glDisable(GL_SCISSOR_TEST);

        // Grab a copy of the left eye's undistorted render output for presentation
        // to the desktop window instead of the barrel distorted mirror texture.
        // This blit, while cheap, could cost some framerate to the HMD.
        // An over-the-shoulder view is another option, at a greater performance cost.
        {
            if (eye == ovrEyeType::ovrEye_Left)
            {
                BlitLeftEyeRenderToUndistortedMirrorTexture();
            }
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Submit layers to HMD for display
    std::vector<const ovrLayerHeader*> layers;
    layers.push_back(&m_layerEyeFov.Header);

    if (g_tweakbarQuad.m_showQuadInWorld)
    {
        g_tweakbarQuad.SetHmdEyeRay(m_eyePoses[ovrEyeType::ovrEye_Left]); // Writes to m_layerQuad.QuadPoseCenter
        g_tweakbarQuad.DrawToQuad();
        layers.push_back(&g_tweakbarQuad.m_layerQuad.Header);
    }

    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdToEyeViewOffset[0] = m_eyeOffsets[0];
    viewScaleDesc.HmdToEyeViewOffset[1] = m_eyeOffsets[1];
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.f;
    const ovrResult result = ovr_SubmitFrame(hmd, m_frameIndex, &viewScaleDesc, &layers[0], layers.size());
    if (result == ovrSuccess_NotVisible)
    {
        LOG_INFO("ovr_SubmitFrame returned ovrSuccess_NotVisible");
        ///@todo Enter a lower-power, polling "no focus" mode
    }
    else if (result == ovrError_DisplayLost)
    {
        LOG_INFO("ovr_SubmitFrame returned ovrError_DisplayLost");
        ///@todo Tear down textures and session and re-create
    }
    ++m_frameIndex;

    // Increment counters in each swap texture set
    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        ovrSwapTextureSet& swapSet = *m_pTexSet[eye];
        ++swapSet.CurrentIndex %= swapSet.TextureCount;
    }

    if (g_tweakbarQuad.m_showQuadInWorld)
    {
        ovrSwapTextureSet& swapSet = *g_tweakbarQuad.m_pQuadTex;
        ++swapSet.CurrentIndex %= swapSet.TextureCount;
    }

    // Blit mirror texture to monitor window
    {
        glViewport(0, 0, g_mirrorWindowSz.x, g_mirrorWindowSz.y);
        const FBO& srcFBO = m_undistortedFBO;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO.id);
        glBlitFramebuffer(
            0, srcFBO.h, srcFBO.w, 0,
            0, 0, g_mirrorWindowSz.x, g_mirrorWindowSz.y,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

#ifdef USE_ANTTWEAKBAR
    TwDraw();
#endif

}

void exitVR()
{
    //_DestroySwapTextures();
    ovr_Destroy(m_Hmd);
    ovr_Shutdown();
}

static void ErrorCallback(int p_Error, const char* p_Description)
{
    (void)p_Error;
    (void)p_Description;
    LOG_INFO("ERROR: %d, %s", p_Error, p_Description);
}

void keyboard(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
    (void)pWindow;
    (void)codes;

    if ((key > -1) && (key <= GLFW_KEY_LAST))
    {
        m_keyStates[key] = action;
    }

    if (action == GLFW_PRESS)
    {
    switch (key)
    {
        default:
            break;

        case GLFW_KEY_SPACE:
            ovr_RecenterPose(m_Hmd);
            break;

        case GLFW_KEY_R:
            m_chassisPos = glm::vec3(0.f);
            break;

        case GLFW_KEY_BACKSPACE:
            {
                int phm = static_cast<int>(m_perfHudMode);
                ++phm %= static_cast<int>(ovrPerfHud_Count);
                m_perfHudMode = static_cast<ovrPerfHudMode>(phm);
                ovr_SetInt(m_Hmd, OVR_PERF_HUD_MODE, m_perfHudMode);
            }
            break;

        case GLFW_KEY_TAB:
            g_tweakbarQuad.m_showQuadInWorld = !g_tweakbarQuad.m_showQuadInWorld;
            break;

        case GLFW_KEY_ENTER:
            g_gallery.ToggleShaderWorld();
            break;

        case GLFW_KEY_SLASH:
            g_tweakbarQuad.SetHoldingFlag(m_eyePoses[0], true);
            break;

        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(g_pMirrorWindow, 1);
            break;
    }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (key)
        {
        default:
            break;

        case GLFW_KEY_SLASH:
            g_tweakbarQuad.SetHoldingFlag(m_eyePoses[0], false);
            break;
        }
    }

    // Handle keyboard movement(WASD keys)
    const glm::vec3 forward(0.f, 0.f, -1.f);
    const glm::vec3 up(0.f, 1.f, 0.f);
    const glm::vec3 right(1.f, 0.f, 0.f);
    glm::vec3 keyboardMove(0.0f, 0.0f, 0.0f);
    if (m_keyStates['W'] != GLFW_RELEASE) { keyboardMove += forward; }
    if (m_keyStates['S'] != GLFW_RELEASE) { keyboardMove -= forward; }
    if (m_keyStates['A'] != GLFW_RELEASE) { keyboardMove -= right; }
    if (m_keyStates['D'] != GLFW_RELEASE) { keyboardMove += right; }
    if (m_keyStates['Q'] != GLFW_RELEASE) { keyboardMove -= up; }
    if (m_keyStates['E'] != GLFW_RELEASE) { keyboardMove += up; }
    if (m_keyStates[GLFW_KEY_UP] != GLFW_RELEASE) { keyboardMove += forward; }
    if (m_keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) { keyboardMove -= forward; }
    if (m_keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE) { keyboardMove -= right; }
    if (m_keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE) { keyboardMove += right; }

    float mag = 1.0f;
    if (m_keyStates[GLFW_KEY_LEFT_SHIFT] != GLFW_RELEASE)
        mag *= 0.1f;
    if (m_keyStates[GLFW_KEY_LEFT_CONTROL] != GLFW_RELEASE)
        mag *= 10.0f;
    m_keyboardMove = mag * keyboardMove;
}

void mouseDown(GLFWwindow* pWindow, int button, int action, int mods)
{
    (void)pWindow;
    (void)mods;
    g_tweakbarQuad.MouseClick(action); ///@todo button id
}

void mouseMove(GLFWwindow* pWindow, double xd, double yd)
{
    glfwGetCursorPos(pWindow, &xd, &yd);
    const int x = static_cast<int>(xd);
    const int y = static_cast<int>(yd);
    g_tweakbarQuad.MouseMotion(x, y);
}

void timestep()
{
    const double absT = g_timer.seconds();
    const double dt = absT - g_lastFrameTime;
    g_lastFrameTime = absT;
    if (g_pScene != NULL)
    {
        g_pScene->timestep(absT, dt);
    }

    // Move in the direction the viewer is facing.
    const glm::vec3 move_dt = m_keyboardMove * static_cast<float>(dt);
    const glm::mat4 moveTxfm = makeMatrixFromPose(m_eyePoses[0]);
    const glm::vec4 mv4 = moveTxfm * glm::vec4(move_dt, 0.f);
    m_chassisPos += glm::vec3(mv4);
}

void resize(GLFWwindow* pWindow, int w, int h)
{
    (void)pWindow;
    g_mirrorWindowSz.x = w;
    g_mirrorWindowSz.y = h;
}

void StartShaderLoad()
{
    g_gallery.LoadTextureLibrary();
    const bool g_loadShadertoysRecursive = false;
    g_gallery.DiscoverShaders(g_loadShadertoysRecursive);

    ///@todo It would save some time to compile all these shaders in parallel on
    /// a multicore machine. Even cooler would be compiling them in a background thread
    /// while display is running, but trying that yields large frame rate drops
    /// which would make the VR experience unacceptably uncomfortable.
    //g_app.LoadTextureLibrary();
    g_gallery.CompileShaders();
    g_gallery.RearrangePanes();
    g_gallery.RenderThumbnails();
}

// OpenGL debug callback
void GLAPIENTRY myCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *msg,
    const void *data)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    case GL_DEBUG_SEVERITY_MEDIUM:
    case GL_DEBUG_SEVERITY_LOW:
        LOG_INFO("[[GL Debug]] %x %x %x %x %s", source, type, id, severity, msg);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        break;
    }
}

int main(int argc, char** argv)
{
    initHMD();

    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);// : GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    GLFWwindow* l_Window = glfwCreateWindow(g_mirrorWindowSz.x, g_mirrorWindowSz.y, "Mirror window", NULL, NULL);
    if (!l_Window)
    {
        LOG_ERROR("Glfw failed to create a window. Exiting.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(l_Window);
    glfwSetKeyCallback(l_Window, keyboard);
    glfwSetMouseButtonCallback(l_Window, mouseDown);
    glfwSetCursorPosCallback(l_Window, mouseMove);
    glfwSetWindowSizeCallback(l_Window, resize);
    g_pMirrorWindow = l_Window;

    memset(m_keyStates, 0, GLFW_KEY_LAST*sizeof(int));

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        LOG_INFO("glewInit() error.");
        exit(EXIT_FAILURE);
    }

#ifdef _DEBUG
    // Debug callback initialization
    // Must be done *after* glew initialization.
    glDebugMessageCallback(myCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
        GL_DEBUG_SEVERITY_NOTIFICATION, -1 , "Start debugging");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

#ifdef USE_ANTTWEAKBAR
    TwInit(TW_OPENGL_CORE, NULL);
    initAnt();
#endif

    g_pScene = &g_gallery;// new ShaderGalleryScene();
    if (g_pScene != NULL)
    {
        g_pScene->initGL();
    }

    g_gallery.SetHmdPositionPointer(&m_hmdRo);
    g_gallery.SetHmdDirectionPointer(&m_hmdRd);
    g_gallery.SetChassisPosPointer(&m_chassisPos);
    g_gallery.SetChassisYawPointer(&m_chassisYaw);

    initVR();
    StartShaderLoad();
    glfwSwapInterval(0);
    while (!glfwWindowShouldClose(l_Window))
    {
        glfwPollEvents();
        timestep();

#ifdef USE_ANTTWEAKBAR
        TwRefreshBar(g_pMainTweakbar);
        TwRefreshBar(g_pShaderTweakbar);
#endif
        displayHMD();
        glfwSwapBuffers(l_Window);
    }
    exitVR();
    g_pScene->exitGL();
    glfwDestroyWindow(l_Window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// OVRSDK06AppSkeleton.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#include <OVR.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "OVRSDK06AppSkeleton.h"
#include "ShaderToy.h"
#include "ShaderFunctions.h"
#include "MatrixFunctions.h"
#include "GLUtils.h"
#include "Logger.h"

OVRSDK06AppSkeleton::OVRSDK06AppSkeleton()
: AppSkeleton()
, m_Hmd(NULL)
, m_pMirrorTex(NULL)
, m_frameIndex(0)
, m_usingDebugHmd(false)
, m_mirror(MirrorDistorted)
{
    m_pTexSet[0] = NULL;
    m_pTexSet[1] = NULL;

    m_eyePoseCached = OVR::Posef();
    _StoreHmdPose(m_eyePoseCached);

    ResetChassisTransformations();
}

OVRSDK06AppSkeleton::~OVRSDK06AppSkeleton()
{
}

void OVRSDK06AppSkeleton::RecenterPose()
{
    if (m_Hmd == NULL)
        return;
    ovrHmd_RecenterPose(m_Hmd);
}

void OVRSDK06AppSkeleton::ToggleMirroringType()
{
    int m = static_cast<int>(m_mirror);
    ++m %= NumMirrorTypes;
    m_mirror = static_cast<MirrorType>(m);
}

void OVRSDK06AppSkeleton::initGL()
{
    AppSkeleton::initGL();

    // sensible initial value?
    allocateFBO(m_rwwttBuffer, 800, 600);
}

void OVRSDK06AppSkeleton::exitVR()
{
    AppSkeleton::exitGL();
    deallocateFBO(m_rwwttBuffer);
    for (int i = 0; i < 2; ++i)
    {
        ovrHmd_DestroySwapTextureSet(m_Hmd, m_pTexSet[i]);
    }
    ovrHmd_Destroy(m_Hmd);
    ovr_Shutdown();
}

///@brief Set this up early so we can get the HMD's display dimensions to create a window.
void OVRSDK06AppSkeleton::initHMD()
{
    ovrInitParams initParams = { 0 };
    if (ovrSuccess != ovr_Initialize(NULL))
    {
        LOG_INFO("Failed to initialize the Oculus SDK");
    }

    if (ovrSuccess != ovrHmd_Create(0, &m_Hmd))
    {
        LOG_INFO("Could not create HMD");
        if (ovrSuccess != ovrHmd_CreateDebug(ovrHmd_DK2, &m_Hmd))
        {
            LOG_ERROR("Could not create Debug HMD");
        }
        m_usingDebugHmd = true;
    }

    const ovrBool ret = ovrHmd_ConfigureTracking(m_Hmd,
        ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position,
        ovrTrackingCap_Orientation);
    if (!OVR_SUCCESS(ret))
    {
        LOG_ERROR("Error calling ovrHmd_ConfigureTracking");
    }

    m_ovrScene.SetHmdPointer(m_Hmd);
}

void OVRSDK06AppSkeleton::initVR()
{
    if (m_Hmd == NULL)
        return;

    for (int i = 0; i < 2; ++i)
    {
        if (m_pTexSet[i])
        {
            ovrHmd_DestroySwapTextureSet(m_Hmd, m_pTexSet[i]);
            m_pTexSet[i] = nullptr;
        }
    }

    // Set up eye fields of view
    ovrLayerEyeFov& layer = m_layerEyeFov;
    layer.Header.Type = ovrLayerType_EyeFov;
    layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    // Create eye render target textures and FBOs
    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        const ovrFovPort& fov = layer.Fov[eye] = m_Hmd->MaxEyeFov[eye];
        const ovrSizei& size = layer.Viewport[eye].Size = ovrHmd_GetFovTextureSize(m_Hmd, eye, fov, 1.f);
        layer.Viewport[eye].Pos = { 0, 0 };
        LOG_INFO("Eye %d tex : %dx%d @ (%d,%d)", eye, size.w, size.h,
            layer.Viewport[eye].Pos.x, layer.Viewport[eye].Pos.y);

        ovrEyeRenderDesc & erd = m_eyeRenderDescs[eye];
        erd = ovrHmd_GetRenderDesc(m_Hmd, eye, m_Hmd->MaxEyeFov[eye]);
        ovrMatrix4f ovrPerspectiveProjection =
            ovrMatrix4f_Projection(erd.Fov, .1f, 10000.f, ovrProjection_RightHanded);
        m_eyeProjections[eye] = glm::transpose(glm::make_mat4(&ovrPerspectiveProjection.M[0][0]));
        m_eyeOffsets[eye] = erd.HmdToEyeViewOffset;

        // Allocate the frameBuffer that will hold the scene, and then be
        // re-rendered to the screen with distortion
        if (!OVR_SUCCESS(ovrHmd_CreateSwapTextureSetGL(m_Hmd, GL_RGBA, size.w, size.h, &m_pTexSet[eye])))
        {
            LOG_ERROR("Unable to create swap textures");
            return;
        }
        ovrSwapTextureSet& swapSet = *m_pTexSet[eye];
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
        {
            const GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
            if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
            {
                LOG_ERROR("Framebuffer status incomplete: %d %x", status, status);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        layer.ColorTexture[eye] = m_pTexSet[eye];
    }

    // Mirror texture for displaying to desktop window
    if (m_pMirrorTex)
    {
        ovrHmd_DestroyMirrorTexture(m_Hmd, m_pMirrorTex);
    }

    const ovrEyeType eye = ovrEyeType::ovrEye_Left;
    const ovrFovPort& fov = layer.Fov[eye] = m_Hmd->MaxEyeFov[eye];
    const ovrSizei& size = layer.Viewport[eye].Size = ovrHmd_GetFovTextureSize(m_Hmd, eye, fov, 1.f);
    ovrResult result = ovrHmd_CreateMirrorTextureGL(m_Hmd, GL_RGBA, size.w, size.h, &m_pMirrorTex);
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

    // Check status
    {
        const GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            LOG_ERROR("Framebuffer status incomplete: %d %x", status, status);
        }
    }

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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glEnable(GL_DEPTH_TEST);


    //if (UsingDebugHmd() == false)
    //{
    //    m_windowSize.x = m_EyeTexture[0].OGL.Header.RenderViewport.Size.w;
    //    m_windowSize.y = m_EyeTexture[0].OGL.Header.RenderViewport.Size.h;
    //}
}

ovrSizei OVRSDK06AppSkeleton::getHmdResolution() const
{
    if (m_Hmd == NULL)
    {
        ovrSizei empty = { 0, 0 };
        return empty;
    }
    return m_Hmd->Resolution;
}

/// Add together the render target size fields of the HMD laid out side-by-side.
ovrSizei calculateCombinedTextureSize(ovrHmd pHmd)
{
    ovrSizei texSz = {0};
    if (pHmd == NULL)
        return texSz;

    const ovrSizei szL = ovrHmd_GetFovTextureSize(pHmd, ovrEye_Left, pHmd->DefaultEyeFov[ovrEye_Left], 1.f);
    const ovrSizei szR = ovrHmd_GetFovTextureSize(pHmd, ovrEye_Right, pHmd->DefaultEyeFov[ovrEye_Right], 1.f);
    texSz.w = szL.w + szR.w;
    texSz.h = std::max(szL.h, szR.h);
    return texSz;
}

///@brief This function will detect a moderate tap on the Rift via the accelerometer.
///@return true if a tap was detected, false otherwise.
bool OVRSDK06AppSkeleton::CheckForTapOnHmd()
{
    const ovrTrackingState ts = ovrHmd_GetTrackingState(m_Hmd, ovr_GetTimeInSeconds());
    if (!(ts.StatusFlags & ovrStatus_OrientationTracked))
        return false;

    const OVR::Vector3f v(ts.RawSensorData.Accelerometer);
    // Arbitrary value and representing moderate tap on the side of the DK2 Rift.
    // When HMD is stationary, gravity alone should yield ~= 9.8^2 == 96.04
    const float lenSq = v.LengthSq();
    const float tapThreshold = 250.f;
    if (lenSq > tapThreshold)
    {
        // Limit tapping rate
        static double lastTapTime = 0.0;
        if (ovr_GetTimeInSeconds() - lastTapTime > 0.5)
        {
            lastTapTime = ovr_GetTimeInSeconds();
            DismissHealthAndSafetyWarning();
            ToggleShaderWorld();
            return true;
        }
    }
    return false;
}

/// Uses a cached copy of HMD orientation written to in display(which are const
/// functions, but m_eyePoseCached is a mutable member).
glm::mat4 OVRSDK06AppSkeleton::makeWorldToEyeMatrix() const
{
    return makeWorldToChassisMatrix() * makeMatrixFromPose(m_eyePoseCached, m_headSize);
}

// Store HMD position and direction for gaze tracking in timestep.
// OVR SDK requires head pose be queried between ovrHmd_BeginFrameTiming and ovrHmd_EndFrameTiming.
// Don't worry - we're just writing to _mutable_ members, it's still const!
void OVRSDK06AppSkeleton::_StoreHmdPose(const ovrPosef& eyePose) const
{
    m_hmdRo.x = eyePose.Position.x + m_chassisPos.x;
    m_hmdRo.y = eyePose.Position.y + m_chassisPos.y;
    m_hmdRo.z = eyePose.Position.z + m_chassisPos.z;

    const glm::mat4 w2eye = makeWorldToChassisMatrix() * makeMatrixFromPose(eyePose, m_headSize);
    const OVR::Matrix4f rotmtx = makeOVRMatrixFromGlmMatrix(w2eye);
    const OVR::Vector4f lookFwd(0.f, 0.f, -1.f, 0.f);
    const OVR::Vector4f rotvec = rotmtx.Transform(lookFwd);
    m_hmdRd.x = rotvec.x;
    m_hmdRd.y = rotvec.y;
    m_hmdRd.z = rotvec.z;

    // Store a separate copy of (ro,rd) in local space without chassis txfms applied.
    m_hmdRoLocal.x = eyePose.Position.x;
    m_hmdRoLocal.y = eyePose.Position.y;
    m_hmdRoLocal.z = eyePose.Position.z;

    const OVR::Matrix4f rotmtxLocal = OVR::Matrix4f(eyePose.Orientation);
    const OVR::Vector4f rotvecLocal = rotmtxLocal.Transform(OVR::Vector4f(0.0f, 0.0f, -1.0f, 0.0f));
    m_hmdRdLocal.x = rotvecLocal.x;
    m_hmdRdLocal.y = rotvecLocal.y;
    m_hmdRdLocal.z = rotvecLocal.z;

    m_eyePoseCached = eyePose; // cache this for movement direction
}

const ovrRecti getScaledRect(const ovrRecti& r, float s)
{
    const ovrRecti scaled = {
        static_cast<int>(s * r.Pos.x),
        static_cast<int>(s * r.Pos.y),
        static_cast<int>(s * r.Size.w),
        static_cast<int>(s * r.Size.h)
    };
    return scaled;
}

///@brief Draw all scenes to a 
void OVRSDK06AppSkeleton::_RenderScenesToStereoBuffer(
    const ovrHmd hmd,
    const glm::mat4* eyeProjMatrix,
    const glm::mat4* eyeMvMtxLocal,
    const glm::mat4* eyeMvMtxWorld,
    const ovrRecti* rvpFull
    ) const
{
    if (hmd == NULL)
        return;

    float fboScale = m_fboScale;

    // Draw to the surface that will be presented to OVR SDK via ovrHmd_EndFrame
    //bindFBO(m_rwwttBuffer);
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_SCISSOR_TEST); // cinemaScope letterbox bars
        for (std::vector<IScene*>::const_iterator it = m_scenes.begin();
            it != m_scenes.end();
            ++it)
        {
            const IScene* pScene = *it;
            if (pScene == NULL)
                continue;

            // Pre-render per-scene actions
            // These blocks depend on the order scenes were added to the vector.
            const bool doRaymarch = m_galleryScene.GetActiveShaderToy() != NULL;
            if (pScene == &m_galleryScene)
            {
                if (doRaymarch)
                {
                    glDisable(GL_DEPTH_TEST);
                    glDepthMask(GL_FALSE);
                }
                else
                {
                    // A state object would be nice here, disable scissoring temporarily.
                    //bindFBO(m_renderBuffer);
                    glDisable(GL_SCISSOR_TEST); // disable cinemascope scissor to fill render target
                    glClearColor(0.f, 0.f, 0.f, 0.f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glEnable(GL_SCISSOR_TEST); // re-enable for cinemascope
                    fboScale = 1.f;
                }
            }
            else if (pScene == &m_ovrScene)
            {
                if (doRaymarch)
                    glDepthMask(GL_TRUE);
            }
            else if (pScene == &m_floorScene)
            {
                if (doRaymarch)
                {
                    glEnable(GL_DEPTH_TEST);
                    break; // out of Scene loop; no need to draw floor
                }
            }

            // Draw eyes inside scene loop
            for (int eyeIndex=0; eyeIndex<ovrEye_Count; eyeIndex++)
            {
                const ovrEyeType e = hmd->EyeRenderOrder[eyeIndex];

                // Viewport setup
                const ovrRecti rvpScaled = getScaledRect(rvpFull[e], fboScale);
                const ovrRecti& rvp = rvpScaled;
                const int yoff = static_cast<int>(static_cast<float>(rvp.Size.h) * m_cinemaScopeFactor);
                glViewport(rvp.Pos.x, rvp.Pos.y, rvp.Size.w, rvp.Size.h);
                glScissor(0, yoff/2, rvp.Pos.x+rvp.Size.w, rvp.Size.h-yoff); // Assume side-by-side single render texture

                // Matrix setup
                const float* pPersp = glm::value_ptr(eyeProjMatrix[e]);
                const float* pMvWorld = glm::value_ptr(eyeMvMtxWorld[e]);
                const float* pMvLocal = glm::value_ptr(eyeMvMtxLocal[e]);
                const float* pMv = pScene->m_bChassisLocalSpace ? pMvLocal : pMvWorld;

                pScene->RenderForOneEye(pMv, pPersp);
            } // eye loop

#if 0
            // Post-render scene-specific actions
            if (doRaymarch && (pScene == &m_galleryScene))
            {
                // rwwtt scene is now rendered to downscaled buffer.
                // Stay bound to this FBO for UI accoutrements rendering.
                glDisable(GL_SCISSOR_TEST); // disable cinemascope scissor to fill render target
                bindFBO(m_renderBuffer);
                _StretchBlitDownscaledBuffer();
                glEnable(GL_SCISSOR_TEST); // re-enable for cinemascope
                fboScale = 1.f;
            }
#endif

        } // scene loop
        glDisable(GL_SCISSOR_TEST); // cinemaScope letterbox bars
    }
    //unbindFBO();
}

///@brief The extra blit imposes some overhead, so for better performance we can render
/// only the raymarch scene to a downscaled buffer and pass that directly to OVR.
void OVRSDK06AppSkeleton::_RenderOnlyRaymarchSceneToStereoBuffer(
    const ovrHmd hmd,
    const glm::mat4* eyeProjMatrix,
    const glm::mat4* eyeMvMtxWorld,
    const ovrRecti* rvpFull) const
{
    const float fboScale = m_fboScale;
    //bindFBO(m_renderBuffer);
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);

        // Draw eyes inside scene loop
        for (int eyeIndex=0; eyeIndex<ovrEye_Count; eyeIndex++)
        {
            const ovrEyeType e = hmd->EyeRenderOrder[eyeIndex];

            // Viewport setup
            const ovrRecti rvpScaled = getScaledRect(rvpFull[e], fboScale);
            const ovrRecti& rvp = rvpScaled;
            const int yoff = static_cast<int>(static_cast<float>(rvp.Size.h) * m_cinemaScopeFactor);
            glViewport(rvp.Pos.x, rvp.Pos.y, rvp.Size.w, rvp.Size.h);
            glScissor(0, yoff/2, rvp.Pos.x+rvp.Size.w, rvp.Size.h-yoff); // Assume side-by-side single render texture

            // Matrix setup
            const float* pPersp = glm::value_ptr(eyeProjMatrix[e]);
            const float* pMvWorld = glm::value_ptr(eyeMvMtxWorld[e]);

            m_galleryScene.RenderForOneEye(pMvWorld, pPersp);
        } // eye loop

        glDisable(GL_SCISSOR_TEST);
    }
    //unbindFBO();
}

///@brief Blit the contents of the downscaled render buffer to the full-sized
void OVRSDK06AppSkeleton::_StretchBlitDownscaledBuffer() const
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const GLuint prog = m_presentFbo.prog();
    glUseProgram(prog);
    m_presentFbo.bindVAO();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_rwwttBuffer.tex);
        glUniform1i(m_presentFbo.GetUniLoc("fboTex"), 0);
        glUniform1f(m_presentFbo.GetUniLoc("fboScale"), m_fboScale);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

///@brief Populate the given arrrays of size eyeCount with per-eye rendering parameters.
void OVRSDK06AppSkeleton::_CalculatePerEyeRenderParams(
    const ovrPosef eyePoses[2], // [in] Eye poses in local space from ovrHmd_GetEyePoses
    const ovrPosef eyePosesScaled[2], // [in] Eye poses from ovrHmd_GetEyePoses with head size applied
    ovrPosef* renderPose, // [out]
    glm::mat4* eyeProjMatrix, // [out]
    glm::mat4* eyeMvMtxLocal, // [out]
    glm::mat4* eyeMvMtxWorld, // [out]
    ovrRecti* renderVp // [out]
    ) const
{
    ovrHmd hmd = m_Hmd;
    if (hmd == NULL)
        return;
    // Calculate eye poses for rendering and to pass to OVR SDK after rendering
    for (int eyeIndex=0; eyeIndex<ovrEye_Count; eyeIndex++)
    {
        const ovrEyeType e = hmd->EyeRenderOrder[eyeIndex];
        const ovrPosef eyePose = eyePoses[e];
        const ovrPosef eyePoseScaled = eyePosesScaled[e];

        renderPose[e] = eyePose;

        eyeMvMtxLocal[e] = makeMatrixFromPose(eyePose);
        const glm::mat4 eyeMvMtxLocalScaled = makeMatrixFromPose(eyePoseScaled, m_headSize);
        eyeMvMtxWorld[e] = makeWorldToChassisMatrix() * eyeMvMtxLocalScaled;
        // These two matrices will be used directly for rendering
        eyeMvMtxLocal[e] = glm::inverse(eyeMvMtxLocal[e]);
        eyeMvMtxWorld[e] = glm::inverse(eyeMvMtxWorld[e]);

        if (eyeIndex == 0)
        {
            _StoreHmdPose(eyePose);
        }
    }
}

void OVRSDK06AppSkeleton::display_sdk() const
{
    ovrHmd hmd = m_Hmd;
    if (hmd == NULL)
        return;

    ovrTrackingState outHmdTrackingState = { 0 };
    ovrHmd_GetEyePoses(m_Hmd, m_frameIndex, m_eyeOffsets,
        m_eyePoses, &outHmdTrackingState);
    _StoreHmdPose(m_eyePoses[0]);

    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        const ovrSwapTextureSet& swapSet = *m_pTexSet[eye];
        glBindFramebuffer(GL_FRAMEBUFFER, m_swapFBO.id);
        ovrGLTexture& tex = (ovrGLTexture&)(swapSet.Textures[swapSet.CurrentIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.OGL.TexId, 0);
        {
            // Handle render target resolution scaling
            m_layerEyeFov.Viewport[eye].Size = ovrHmd_GetFovTextureSize(m_Hmd, eye, m_layerEyeFov.Fov[eye], m_fboScale);
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

            // Render the scene for the current eye
            const ovrPosef& eyePose = m_eyePoses[eye];
            const glm::mat4 viewLocal = makeMatrixFromPose(eyePose);
            const glm::mat4 viewWorld = makeWorldToChassisMatrix() * viewLocal;
            const glm::mat4& proj = m_eyeProjections[eye];

            {
                const float* pMvWorld = glm::value_ptr(glm::inverse(viewWorld));
                const float* pPersp = glm::value_ptr(proj);
                const float* pMvLocal = glm::value_ptr(glm::inverse(viewLocal));

                for (std::vector<IScene*>::const_iterator it = m_scenes.begin();
                    it != m_scenes.end();
                    ++it)
                {
                    const IScene* pScene = *it;
                    if (pScene != NULL)
                    {
                        const float* pMv = pScene->m_bChassisLocalSpace ? pMvLocal : pMvWorld;
                        pScene->RenderForOneEye(pMv, pPersp);
                    }
                }
            }

            m_layerEyeFov.RenderPose[eye] = eyePose;
        }

        // Grab a copy of the left eye's undistorted render output for presentation
        // to the desktop window instead of the barrel distorted mirror texture.
        // This blit, while cheap, could cost some framerate to the HMD.
        // An over-the-shoulder view is another option, at a greater performance cost.
        if (m_mirror == MirrorUndistorted)
        {
            if (eye == ovrEyeType::ovrEye_Left)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_swapFBO.id);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_undistortedFBO.id);
                glViewport(0, 0, m_undistortedFBO.w, m_undistortedFBO.h);
                glBlitFramebuffer(
                    0, static_cast<int>(static_cast<float>(m_swapFBO.h)*m_fboScale),
                    static_cast<int>(static_cast<float>(m_swapFBO.w)*m_fboScale), 0, ///@todo Fix for FBO scaling
                    0, 0, m_undistortedFBO.w, m_undistortedFBO.h,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, m_swapFBO.id);
            }
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ovrLayerEyeFov& layer = m_layerEyeFov;
    ovrLayerHeader* layers = &layer.Header;
    ovrResult result = ovrHmd_SubmitFrame(hmd, m_frameIndex, NULL, &layers, 1);

    // Increment counters in swap texture set
    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
        eye < ovrEyeType::ovrEye_Count;
        eye = static_cast<ovrEyeType>(eye + 1))
    {
        ovrSwapTextureSet& swapSet = *m_pTexSet[eye];
        ++swapSet.CurrentIndex %= swapSet.TextureCount;
    }

    // Blit output to main app window to show something on screen in addition
    // to what's in the Rift. This could optionally be the distorted texture
    // from the OVR SDK's mirror texture, or perhaps a single eye's undistorted
    // view, or even a third-person render(at a performance cost).
    if (m_mirror != MirrorNone)
    {
        glViewport(0, 0, m_appWindowSize.w, m_appWindowSize.h);
        const FBO& srcFBO = m_mirror == MirrorDistorted ? m_mirrorFBO : m_undistortedFBO;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO.id);
        glBlitFramebuffer(
            0, srcFBO.h, srcFBO.w, 0,
            0, 0, m_appWindowSize.w, m_appWindowSize.h,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    ++m_frameIndex;
}

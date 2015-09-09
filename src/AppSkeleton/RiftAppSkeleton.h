// RiftAppSkeleton.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "AppSkeleton.h"

#include <Kernel/OVR_Types.h> // Pull in OVR_OS_* defines 
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#include "FBO.h"

///@brief Encapsulates as much of the VR viewer state as possible,
/// pushing all viewer-independent stuff to Scene.
class RiftAppSkeleton : public AppSkeleton
{
public:
    RiftAppSkeleton();
    virtual ~RiftAppSkeleton();

    virtual void initGL();
    void initHMD();
    void initVR();
    void exitVR();
    void RecenterPose();
    int ConfigureRendering();
    int ConfigureSDKRendering();
    int ConfigureClientRendering();
    virtual void DismissHealthAndSafetyWarning() const;
    bool CheckForTapOnHmd();

    void display_stereo_undistorted() const;
    void display_sdk() const;
    void display_client() const;

    // Direct mode and SDK rendering hooks
    void AttachToWindow(void* pWindow) { ovrHmd_AttachToWindow(m_Hmd, pWindow, NULL, NULL); }
#if defined(OVR_OS_WIN32)
    void setWindow(HWND w) { m_Cfg.OGL.Window = w; }
#elif defined(OVR_OS_LINUX)
    void setWindow(_XDisplay* Disp) { m_Cfg.OGL.Disp = Disp; }
#endif

    ovrSizei getHmdResolution() const;
    ovrVector2i getHmdWindowPos() const;
    bool UsingDebugHmd() const { return m_usingDebugHmd; }
    bool UsingDirectMode() const { return m_directHmdMode; }

protected:
    void _initPresentDistMesh(ShaderWithVariables& shader, int eyeIdx);
    void _CalculatePerEyeRenderParams(
        const ovrPosef eyePoses[2],
        const ovrPosef eyePosesScaled[2],
        ovrPosef* renderPose,
        ovrTexture* eyeTexture,
        glm::mat4* eyeProjMatrix,
        glm::mat4* eyeMvMtxLocal,
        glm::mat4* eyeMvMtxWorld,
        ovrRecti* renderVp) const;
    void _RenderScenesToStereoBuffer(
        const ovrHmd hmd,
        const glm::mat4* eyeProjMatrix,
        const glm::mat4* eyeMvMtxLocal,
        const glm::mat4* eyeMvMtxWorld,
        const ovrRecti* rvpFull) const;
    void _RenderOnlyRaymarchSceneToStereoBuffer(
        const ovrHmd hmd,
        const glm::mat4* eyeProjMatrix,
        const glm::mat4* eyeMvMtxWorld,
        const ovrRecti* rvpFull) const;

    void _StoreHmdPose(const ovrPosef& eyePose) const;
    void _StretchBlitDownscaledBuffer() const;

    virtual glm::mat4 makeWorldToEyeMatrix() const;

    ovrHmd m_Hmd;
    ovrFovPort m_EyeFov[2];
    ovrGLConfig m_Cfg;
    ovrEyeRenderDesc m_EyeRenderDesc[2];
    ovrGLTexture m_EyeTexture[2];
    bool m_usingDebugHmd;
    bool m_directHmdMode;

    // For client rendering
    ovrRecti m_RenderViewports[2];
    ovrDistortionMesh m_DistMeshes[2];
    mutable ovrPosef m_eyePoseCached;

protected:
    FBO m_rwwttBuffer;

private: // Disallow copy ctor and assignment operator
    RiftAppSkeleton(const RiftAppSkeleton&);
    RiftAppSkeleton& operator=(const RiftAppSkeleton&);
};

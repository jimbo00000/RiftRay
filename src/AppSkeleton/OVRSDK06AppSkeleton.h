// OVRSDK06AppSkeleton.h

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
class OVRSDK06AppSkeleton : public AppSkeleton
{
public:
    enum MirrorType {
        MirrorNone = 0,
        MirrorDistorted,
        MirrorUndistorted,
        NumMirrorTypes
    };

    OVRSDK06AppSkeleton();
    virtual ~OVRSDK06AppSkeleton();

    virtual void initGL();
    void initHMD();
    void initVR();
    void RecenterPose();
    void exitVR();

    void ToggleMirroringType();
    bool CheckForTapOnHmd();

    void SetAppWindowSize(ovrSizei sz) { m_appWindowSize = sz; }

    ovrSizei getHmdResolution() const;
    bool UsingDebugHmd() const { return m_usingDebugHmd; }

    void display_stereo_undistorted() { display_sdk(); }
    void display_sdk() const;
    void display_client() const { display_sdk(); }

protected:
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
    ovrTexture* m_pMirrorTex;
    ovrSwapTextureSet* m_pTexSet[ovrEye_Count];
    ovrEyeRenderDesc m_eyeRenderDescs[ovrEye_Count];
    ovrVector3f m_eyeOffsets[ovrEye_Count];
    glm::mat4 m_eyeProjections[ovrEye_Count];

    mutable ovrPosef m_eyePoses[ovrEye_Count];
    mutable ovrLayerEyeFov m_layerEyeFov;
    mutable int m_frameIndex;

    FBO m_swapFBO;
    FBO m_mirrorFBO;
    FBO m_undistortedFBO;
    ovrSizei m_appWindowSize;
    bool m_usingDebugHmd;
    MirrorType m_mirror;

    mutable ovrPosef m_eyePoseCached;

protected:
    FBO m_rwwttBuffer;

private: // Disallow copy ctor and assignment operator
    OVRSDK06AppSkeleton(const OVRSDK06AppSkeleton&);
    OVRSDK06AppSkeleton& operator=(const OVRSDK06AppSkeleton&);
};

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

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include "FBO.h"
#include "Scene.h"
#include "HydraScene.h"
#include "OVRScene.h"
#include "RaymarchShaderScene.h"
#include "ShaderToyScene.h"
#include "ShaderGalleryScene.h"

#include "FlyingMouse.h"
#include "VirtualTrackball.h"

///@brief Encapsulates as much of the VR viewer state as possible,
/// pushing all viewer-independent stuff to Scene.
class RiftAppSkeleton
{
public:
    RiftAppSkeleton();
    virtual ~RiftAppSkeleton();

    void initGL();
    void initHMD();
    void initVR();
    void exitVR();

    void RecenterPose();
    void ResetAllTransformations();
    void SetChassisPosition(ovrVector3f p) { m_chassisPos = p; }
    ovrSizei getHmdResolution() const;
    ovrVector2i getHmdWindowPos() const;
    GLuint getRenderBufferTex() const { return m_renderBuffer.tex; }
    float GetFboScale() const { return m_fboScale; }

    int ConfigureSDKRendering();
    int ConfigureClientRendering();

#if defined(OVR_OS_WIN32)
    void setWindow(HWND w) { m_Cfg.OGL.Window = w; }
#elif defined(OVR_OS_LINUX)
    void setWindow(Window Win, Display* Disp)
    {
        m_Cfg.OGL.Win = Win;
        m_Cfg.OGL.Disp = Disp;
    }
#endif

    void DismissHealthAndSafetyWarning() const;
    bool CheckForTapOnHmd();

    void display_raw() const;
    void display_buffered(bool setViewport=true) const;
    void display_stereo_undistorted();// const;
    void display_sdk();// const;
    void display_client();// const;
    void timestep(float dt);

    void resize(int w, int h);

    void DiscoverShaders();
    void CompileShaders();
    void RenderThumbnails();
    void LoadTexturesFromFile();
    void ToggleShaderWorld();

    float GetFBOScale() const { return m_fboScale; }
    void SetFBOScale(float s);
#ifdef USE_ANTTWEAKBAR
    float* GetFBOScalePointer() { return &m_fboScale; }
#endif

protected:
    void _initPresentFbo();
    void _initPresentDistMesh(ShaderWithVariables& shader, int eyeIdx);
    void _resetGLState() const;
    void _drawSceneMono() const;
    void _DrawScenes(const float* pMview, const float* pPersp, const ovrRecti& rvp, const float* pScaledMview=NULL) const;
    void _StoreHmdPose(const ovrPosef& eyePose);

    ovrHmd m_Hmd;
    ovrFovPort m_EyeFov[2];
    ovrGLConfig m_Cfg;
    ovrEyeRenderDesc m_EyeRenderDesc[2];
    ovrGLTexture l_EyeTexture[2];

    // For client rendering
    ovrRecti m_RenderViewports[2];
    ovrVector2f m_uvScaleOffsetOut[4];
    ovrDistortionMesh m_DistMeshes[2];
    ovrQuatf m_eyeOri;

    // For eye ray tracking
    glm::vec3 m_hmdRo;
    glm::vec3 m_hmdRd;

public:
    // This public section is for exposing state variables to AntTweakBar
    Scene m_scene;
    HydraScene m_hydraScene;
    OVRScene m_ovrScene;
    RaymarchShaderScene m_raymarchScene;
    ShaderToyScene m_shaderToyScene;
    ShaderGalleryScene m_paneScene;

protected:
    std::vector<IScene*> m_scenes;
    FBO m_renderBuffer;
    float m_fboScale;
    ShaderWithVariables m_presentFbo;
    ShaderWithVariables m_presentDistMeshL;
    ShaderWithVariables m_presentDistMeshR;

    ovrVector3f m_chassisPos;
    float m_chassisYaw;

    VirtualTrackball m_hyif;
    std::vector<ShaderToy*> m_shaderToys;
    std::map<std::string, textureChannel> m_texLibrary;

public:
    float m_headSize;
    FlyingMouse m_fm;
    glm::vec3 m_keyboardMove;
    glm::vec3 m_joystickMove;
    glm::vec3 m_mouseMove;
    float m_keyboardYaw;
    float m_joystickYaw;
    float m_mouseDeltaYaw;
    float m_cinemaScopeFactor;
#ifdef USE_ANTTWEAKBAR
    TwBar* m_pTweakbar;
#endif

private: // Disallow copy ctor and assignment operator
    RiftAppSkeleton(const RiftAppSkeleton&);
    RiftAppSkeleton& operator=(const RiftAppSkeleton&);
};

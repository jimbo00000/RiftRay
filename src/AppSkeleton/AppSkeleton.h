// AppSkeleton.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include "FBO.h"

#ifdef USE_SIXENSE
#include "HydraScene.h"
#endif
#include "OVRScene.h"
#include "RaymarchShaderScene.h"
#include "ShaderGalleryScene.h"
#include "FloorScene.h"
#include "DashboardScene.h"

#include "FlyingMouse.h"
#include "VirtualTrackball.h"

class AppSkeleton
{
public:
    AppSkeleton();
    virtual ~AppSkeleton();

    virtual void ResetChassisTransformations();
    void SetChassisPosition(glm::vec3 p) { m_chassisPos = p; }
    virtual void initGL();
    virtual void exitGL();
    void _DrawScenes(
        const float* pMview,
        const float* pPersp,
        const ovrRecti& rvp,
        const float* pMvLocal) const;

    void DoSceneRenderPrePasses() const;

    void resize(int w, int h);


    void DiscoverShaders(bool recurse=true);
    void SetTextureLibraryPointer();
    void LoadTextureLibrary();
    void ToggleShaderWorld();
    void SaveShaderSettings();

    // For eye ray tracking - set during draw function
    mutable glm::vec3 m_hmdRo;
    mutable glm::vec3 m_hmdRd;
    mutable glm::vec3 m_hmdRoLocal;
    mutable glm::vec3 m_hmdRdLocal;

public:
    // This public section is for exposing state variables to AntTweakBar
    RaymarchShaderScene m_raymarchScene;
    ShaderGalleryScene m_galleryScene;
    DashboardScene m_dashScene;
    OVRScene m_ovrScene;
    FloorScene m_floorScene;
#ifdef USE_SIXENSE
    HydraScene m_hydraScene;
#endif

    GLuint getRenderBufferTex() const { return m_renderBuffer.tex; }

protected:
    void _resetGLState() const;
    void _drawSceneMono() const;
    void _RenderRaymarchSceneToCamBuffer() const;
    glm::mat4 makeWorldToChassisMatrix() const;
    void _ToggleShaderWorld();
    void _SaveShaderSettings(const std::string toFilename);

    std::vector<IScene*> m_scenes;
    float m_fboScale;
    FBO m_renderBuffer;

    glm::vec3 m_chassisPos;
    float m_chassisYaw;
    float m_chassisPitch;
    float m_chassisRoll;
    glm::vec3 m_chassisPosCached;
    float m_chassisYawCached;

    VirtualTrackball m_hyif;
    glm::ivec2 m_windowSize;

    Timer m_transitionTimer;
    int m_transitionState;

    std::map<std::string, textureChannel> m_texLibrary;

public:
    FlyingMouse m_fm;
    glm::vec3 m_keyboardMove;
    glm::vec3 m_joystickMove;
    glm::vec3 m_mouseMove;
    float m_keyboardYaw;
    float m_joystickYaw;
    float m_mouseDeltaYaw;
    float m_keyboardDeltaPitch;
    float m_keyboardDeltaRoll;

    float m_headSize;
    float m_cinemaScopeFactor;
#ifdef USE_ANTTWEAKBAR
    TwBar* m_pTweakbar;
    TwBar* m_pShaderTweakbar;
#endif

private: // Disallow copy ctor and assignment operator
    AppSkeleton(const AppSkeleton&);
    AppSkeleton& operator=(const AppSkeleton&);
};

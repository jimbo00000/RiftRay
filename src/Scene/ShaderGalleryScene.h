// ShaderGalleryScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "PaneScene.h"
#include "VirtualTrackball.h"
#include "ShaderToy.h"
#include "ShaderToyFunctions.h"
#include "ShaderToyGlobalState.h"
#include "FloorScene.h"

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

class Pane;
class ShaderToyPane;

///@brief 
class ShaderGalleryScene : public PaneScene
{
public:
    ShaderGalleryScene();
    virtual ~ShaderGalleryScene();

    virtual void initGL();
    virtual void timestep(double absTime, double dt);
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    virtual void LoadTextureLibrary();
    virtual void DiscoverShaders(bool recurse);
    virtual void CompileShaders();
    virtual void RenderThumbnails() const;

    virtual Pane* AddShaderToyPane(ShaderToy* pSt);
    virtual void RearrangePanes();
    virtual void ResetTimer() { if(m_pActiveShaderToy) m_pActiveShaderToy->ResetTimer(); }

    virtual void ToggleShaderWorld();
    virtual void SetActiveShaderToy(ShaderToy* pSt) { m_pActiveShaderToy = pSt; }
    virtual void SetActiveShaderToyPane(ShaderToyPane* pP) { m_pActiveShaderToyPane = pP; }

    virtual const ShaderToyPane* GetFocusedPane() const;
    virtual const ShaderToy* GetActiveShaderToy() const { return m_pActiveShaderToy; }
    virtual const ShaderToyPane* GetActiveShaderToyPane() const { return m_pActiveShaderToyPane; }

    // main_glfw... allows this class to set these global variables
    virtual void SetChassisPosPointer(glm::vec3* pCp) { m_pChassisPos = pCp; }
    virtual void SetChassisYawPointer(float* pY) { m_pChassisYaw = pY; }
    virtual void SetHeadSizePointer(float* pHS) { m_pHeadSize = pHS; }

protected:
    void _ToggleShaderWorld();

    ShaderToy* m_pActiveShaderToy;
    ShaderToyPane* m_pActiveShaderToyPane;
    std::map<std::string, textureChannel> m_texLibrary;
    Timer m_transitionTimer;
    int m_transitionState;
    FloorScene m_floor;

public:
#ifdef USE_ANTTWEAKBAR
    TwBar* m_pMainTweakbar;
    TwBar* m_pShaderTweakbar;
#endif
    unsigned int m_paneDimensionPixels;
    ShaderToyGlobalState m_globalShadertoyState;
    bool m_useFulldome;
    glm::vec3* m_pChassisPos;
    float* m_pChassisYaw;
    float* m_pHeadSize;
    glm::vec3 m_chassisPosInGallery;
    float m_chassisYawInGallery;

private: // Disallow copy ctor and assignment operator
    ShaderGalleryScene(const ShaderGalleryScene&);
    ShaderGalleryScene& operator=(const ShaderGalleryScene&);
};

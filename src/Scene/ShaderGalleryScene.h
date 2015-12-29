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

class Pane;
class ShaderToyPane;

///@brief 
class ShaderGalleryScene : public PaneScene
{
public:
    ShaderGalleryScene();
    virtual ~ShaderGalleryScene();

    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    virtual void DiscoverShaders(bool recurse);
    virtual void CompileShaders();
    virtual void RenderThumbnails() const;

    virtual Pane* AddShaderToyPane(ShaderToy* pSt);
    virtual void RearrangePanes();
    virtual void ResetTimer() { if(m_pActiveShaderToy) m_pActiveShaderToy->ResetTimer(); }

    virtual void SetTextureLibraryPointer(std::map<std::string, textureChannel>* pTL) { m_pTexLibrary = pTL; }
    virtual void SetActiveShaderToy(ShaderToy* pSt) { m_pActiveShaderToy = pSt; }
    virtual void SetActiveShaderToyPane(ShaderToyPane* pP) { m_pActiveShaderToyPane = pP; }

    virtual const ShaderToyPane* GetFocusedPane() const;
    virtual const ShaderToy* GetActiveShaderToy() const { return m_pActiveShaderToy; }
    virtual const ShaderToyPane* GetActiveShaderToyPane() const { return m_pActiveShaderToyPane; }

protected:
    ShaderToy* m_pActiveShaderToy;
    ShaderToyPane* m_pActiveShaderToyPane;
    std::map<std::string, textureChannel>* m_pTexLibrary;

public:
    unsigned int m_paneDimensionPixels;
    ShaderToyGlobalState m_globalShadertoyState;
    bool m_useFulldome;

private: // Disallow copy ctor and assignment operator
    ShaderGalleryScene(const ShaderGalleryScene&);
    ShaderGalleryScene& operator=(const ShaderGalleryScene&);
};

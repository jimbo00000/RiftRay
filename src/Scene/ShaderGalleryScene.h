// ShaderGalleryScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "PaneScene.h"

#include "VirtualTrackball.h"

class ShaderToy;
class Pane;
class ShaderPane;

///@brief 
class ShaderGalleryScene : public PaneScene
{
public:
    ShaderGalleryScene();
    virtual ~ShaderGalleryScene();

    virtual Pane* AddShaderPane(ShaderToy* pSt);
    virtual void RearrangePanes();
    virtual const ShaderPane* GetFocusedPane() const;

    virtual void SetActiveShaderToy(ShaderToy* pSt) { m_pActiveShaderToy = pSt; }

protected:
    ShaderToy* m_pActiveShaderToy;

private: // Disallow copy ctor and assignment operator
    ShaderGalleryScene(const ShaderGalleryScene&);
    ShaderGalleryScene& operator=(const ShaderGalleryScene&);
};

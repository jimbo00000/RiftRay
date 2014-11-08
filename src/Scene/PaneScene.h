// PaneScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <vector>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "IScene.h"
#include "ShaderWithVariables.h"
#include "FBO.h"

#include "Pane.h"
#include "VirtualTrackball.h"
#include "BMFont.h"

///@brief 
class PaneScene : public IScene
{
public:
    PaneScene();
    virtual ~PaneScene();

    virtual void initGL();
    virtual void timestep(float dt);
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    virtual std::vector<Transformation*> GetTransformationPointers();
    virtual void ResetTransformation();

    virtual void SendMouseClick(int state);
    virtual void SendHmdTap();

    virtual void SetFlyingMousePointer(FlyingMouse* pFM) { m_pFm = pFM; }
    virtual void SetHmdPositionPointer(glm::vec3* pRo) { m_pHmdRo = pRo; }
    virtual void SetHmdDirectionPointer(glm::vec3* pRd) { m_pHmdRd = pRd; }

    const ShaderWithVariables& GetFontShader() const { return m_fontShader; }
    const BMFont& GetFont() const { return m_font; }

protected:
    virtual void _InitPlaneAttributes();
    virtual void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection) const;

    virtual bool _GetFlyingMouseRightHandPaneRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt);
    virtual bool _GetHmdViewRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt);

    FlyingMouse* m_pFm;
    glm::vec3* m_pHmdRo;
    glm::vec3* m_pHmdRd;
    ShaderWithVariables m_paneShader;
    ShaderWithVariables m_fontShader;
    BMFont m_font;

public:
    std::vector<Pane*> m_panes;
    std::vector<glm::vec3> m_panePts;

private: // Disallow copy ctor and assignment operator
    PaneScene(const PaneScene&);
    PaneScene& operator=(const PaneScene&);
};

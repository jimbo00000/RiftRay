// Pane.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <vector>

#include "ShaderWithVariables.h"
#include "FBO.h"
#include "VirtualTrackball.h"

class BMFont;

///@brief 
class Pane
{
public:
    Pane();
    virtual ~Pane();

    virtual void initGL();
    virtual void timestep(float) {}

    virtual void OnMouseClick(int, int, int) {}
    virtual void OnMouseMove(int, int) {}
    virtual void OnHmdTap();

    virtual Transformation* GetTransformationPointer() { return &m_tx; }
    virtual void ResetTransformation()
    {
        m_tx.ResetOrientation();
        m_tx.ResetPosition();
        m_tx.ResetScale();
    }

    virtual glm::ivec2 GetFBOSize() const { return glm::ivec2(m_paneRenderBuffer.w, m_paneRenderBuffer.h); }
    virtual std::vector<glm::vec3> GetTransformedPanePoints() const;
    virtual bool GetPaneRayIntersectionCoordinates(glm::vec3 origin3, glm::vec3 dir3, glm::vec2& planePt);

    virtual void DrawPane() const;
    virtual void DrawPaneWithShader(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const ShaderWithVariables& sh) const {}
    virtual void DrawCursor() const;
    virtual void DrawTextOverlay(
        const std::string text,
        int x,
        int y,
        const ShaderWithVariables& sh,
        const BMFont& font) const;
    virtual void DrawToFBO() const;
    virtual void DrawInScene(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object) const;
protected:

    virtual void _InitPointerAttributes();
    virtual void _InitPlaneAttributes();

    ShaderWithVariables m_cursorShader;
    ShaderWithVariables m_plane;

public:
    FBO m_paneRenderBuffer;
    bool m_cursorInPane;
    glm::vec2 m_pointerCoords;

    std::vector<glm::vec3> m_panePts;
    Transformation m_tx;

private: // Disallow copy ctor and assignment operator
    Pane(const Pane&);
    Pane& operator=(const Pane&);
};

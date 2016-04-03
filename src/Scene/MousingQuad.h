// MousingQuad.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#include "HudQuad.h"
#include "ShaderWithVariables.h"
#include "Timer.h"

///@brief Holds the shader and state to draw a mouse cursor on a HudQuad.
class MousingQuad : public HudQuad
{
public:
    MousingQuad();
    virtual ~MousingQuad();

    virtual void initGL(ovrSession& session, ovrSizei sz);
    virtual void exitGL(ovrSession& session);
    virtual void DrawToQuad();
    virtual void DrawCursor();
    virtual void MouseClick(int state);
    virtual void MouseMotion(int x, int y);
    virtual void SetHmdEyeRay(ovrPosef pose);

protected:
    virtual void _InitPointerAttributes();

    ShaderWithVariables m_cursorShader;
    Timer m_mouseMotionCooldown;
    bool m_acceptMouseMotion;
    bool m_cursorInPane;
    glm::vec2 m_pointerCoords;

private: // Disallow copy ctor and assignment operator
    MousingQuad(const HudQuad&);
    MousingQuad& operator=(const HudQuad&);
};

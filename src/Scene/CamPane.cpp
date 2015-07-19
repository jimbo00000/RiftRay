// CamPane.cpp

#include "CamPane.h"

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "TextureFunctions.h"
#include "Logger.h"

CamPane::CamPane()
    : Pane()
{
}

CamPane::~CamPane()
{
}

void CamPane::initGL()
{
    Pane::initGL();
    allocateFBO(m_paneRenderBuffer, 300, 300);
}

void CamPane::DrawPaneWithShader(
    const glm::mat4&, // modelview
    const glm::mat4&, // projection
    const ShaderWithVariables& sh) const
{
    if (m_visible == false)
        return;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_paneRenderBuffer.tex);
    glUniform1i(sh.GetUniLoc("fboTex"), 0);

    glUniform1f(sh.GetUniLoc("u_brightness"), m_cursorInPane ? 1.0f : 0.5f);

    sh.bindVAO();
    {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
}

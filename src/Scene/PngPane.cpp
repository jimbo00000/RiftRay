// PngPane.cpp

#include "PngPane.h"

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

PngPane::PngPane()
: Pane()
{
    // Swap pane points around to flip texture coordinates
    const glm::vec3 t0 = m_panePts[0];
    const glm::vec3 t1 = m_panePts[1];
    m_panePts[0] = m_panePts[2];
    m_panePts[1] = m_panePts[3];
    m_panePts[2] = t0;
    m_panePts[3] = t1;
}

PngPane::~PngPane()
{
}

void PngPane::initGL()
{
    Pane::initGL();

    GLuint texId = 0;
    GLuint width = 0;
    GLuint height = 0;
    const std::string texName = "../textures/XboxControllerLayout.png";
    texId = LoadTextureFromPng(texName.c_str(), &width, &height, true);

    // Even though this buffer is not rendered to, we can store the static texture here.
    m_paneRenderBuffer.tex = texId;
    m_paneRenderBuffer.w = width;
    m_paneRenderBuffer.h = height;
}

void PngPane::DrawPaneWithShader(
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

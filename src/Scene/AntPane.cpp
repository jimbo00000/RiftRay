// AntPane.cpp

#include "AntPane.h"

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

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include "Logger.h"

AntPane::AntPane()
: Pane()
{
}

AntPane::~AntPane()
{
}

void AntPane::initGL()
{
    Pane::initGL();
    ResizeTweakbar();
}

void AntPane::ResizeTweakbar()
{
#ifdef USE_ANTTWEAKBAR
    ///@note This will override the Aux window's TwSize call.
    const glm::ivec2 fbsz = GetFBOSize();
    TwWindowSize(fbsz.x, fbsz.y);
#endif
}

void AntPane::DrawToFBO() const
{
    //glDisable(GL_SCISSOR_TEST); // Assume this is turned on by caller
    GLint bound_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &bound_prog);

    // Render a view of the shader to the FBO
    // We must keep the previously bound FBO and restore
    GLint bound_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);

    bindFBO(m_paneRenderBuffer);
    {
        if (m_cursorInPane || !m_tx.m_lockedAtClickPos)
            glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
        else
            glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef USE_ANTTWEAKBAR
        TwDraw();
#endif

        DrawCursor();
    }
    unbindFBO();

    // Restore previous state
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bound_fbo);
    glUseProgram(bound_prog);
    //glEnable(GL_SCISSOR_TEST);
}

void AntPane::DrawPaneWithShader(
    const glm::mat4&, // modelview
    const glm::mat4&, // projection
    const ShaderWithVariables& sh) const
{
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

void AntPane::timestep(double absTime, double dt)
{
    Pane::timestep(absTime, dt);
}

void AntPane::OnMouseClick(int state, int, int)
{
#ifdef USE_ANTTWEAKBAR
    if (state == 1)
    {
        TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
    }
    else if (state == 0)
    {
        TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
    }
#endif
}

void AntPane::OnMouseMove(int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    TwMouseMotion(x, y);
#endif
}

// AntQuad.cpp

#include "AntQuad.h"

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

AntQuad::AntQuad()
: MousingQuad()
{
}

AntQuad::~AntQuad()
{
}

void AntQuad::initGL(ovrHmd hmd, ovrSizei sz)
{
    MousingQuad::initGL(hmd, sz);

#ifdef USE_ANTTWEAKBAR
    ///@note This will override the Aux window's TwSize call.
    const glm::ivec2 fbsz(m_fbo.w, m_fbo.h);
    TwWindowSize(fbsz.x, fbsz.y);
#endif
}

void AntQuad::exitGL(ovrHmd hmd)
{
    MousingQuad::exitGL(hmd);
}

void AntQuad::DrawToQuad()
{
    _PrepareToDrawToQuad();
    {
        const float g = .05f;
        glClearColor(g, g, g, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

#ifdef USE_ANTTWEAKBAR
        TwDraw();
#endif

        DrawCursor();
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AntQuad::MouseClick(int state)
{
    MousingQuad::MouseClick(state);
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

void AntQuad::MouseMotion(int x, int y)
{
    MousingQuad::MouseMotion(x, y);
#ifdef USE_ANTTWEAKBAR
    TwMouseMotion(x, y);
#endif
}

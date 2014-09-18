// PaneScene.cpp

#include "PaneScene.h"

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

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include "Logger.h"

PaneScene::PaneScene()
: m_pFm(NULL)
, m_pHmdRo(NULL)
, m_pHmdRd(NULL)
, m_panes()
{
}

PaneScene::~PaneScene()
{
}

void PaneScene::initGL()
{
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        pP->initGL();
    }
}

/// Draw the scene(matrices have already been set up).
void PaneScene::DrawScene(
    const glm::mat4& modelview,
    const glm::mat4& projection) const
{
    for (std::vector<Pane*>::const_iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        const Pane* pP = *it;
        if (pP == NULL)
            continue;

#if 0
        // We must keep the previously bound FBO and restore
        GLint bound_fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);
        bindFBO(pP->m_paneRenderBuffer);

        pP->DrawToFBO();

        unbindFBO();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bound_fbo);
#endif

        pP->DrawInScene(modelview, projection, pP->m_tx.GetMatrix());
    }
}

void PaneScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);
    DrawScene(modelview, projection);
}


bool PaneScene::_GetFlyingMouseRightHandPaneRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt)
{
    if (pPane == NULL)
        return false;
    if (m_pFm == NULL)
        return false;

    if (m_pFm->ControllerIsOnBase(FlyingMouse::Right))
        return false;

    glm::vec3 origin3;
    glm::vec3 dir3;
    m_pFm->GetControllerOriginAndDirection(FlyingMouse::Right, origin3, dir3);

    return pPane->GetPaneRayIntersectionCoordinates(origin3, dir3, planePt);
}

bool PaneScene::_GetHmdViewRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt)
{
    if (pPane == NULL)
        return false;
    if (m_pHmdRo == NULL)
        return false;
    if (m_pHmdRd == NULL)
        return false;

    return pPane->GetPaneRayIntersectionCoordinates(*m_pHmdRo, *m_pHmdRd, planePt);
}


void PaneScene::timestep(float dt)
{
    (void)dt;
    if (!m_bDraw)
        return;

    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;

        pP->m_cursorInPane = _GetFlyingMouseRightHandPaneRayIntersectionCoordinates(pP, pP->m_pointerCoords);

        glm::vec2 hmdPt(0.0f);
        bool hmdInPane = _GetHmdViewRayIntersectionCoordinates(pP, hmdPt);
        if (hmdInPane)
        {
            pP->m_pointerCoords = hmdPt;
            pP->m_cursorInPane = true;
        }

        pP->m_tx.m_lock = !pP->m_cursorInPane;

        {
            const glm::ivec2 fbsz = pP->GetFBOSize();
            const int mx = static_cast<int>( pP->m_pointerCoords.x * static_cast<float>(fbsz.x) );
            const int my = static_cast<int>( pP->m_pointerCoords.y * static_cast<float>(fbsz.y) );
            pP->OnMouseMove(mx, my);

#ifdef USE_SIXENSE
            if (m_pFm->WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_START))
            {
                pP->OnMouseClick(1, mx, my);
            }
            else if (m_pFm->WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_START))
            {
                pP->OnMouseClick(0, mx, my);
            }
#endif
        }
    }
}

std::vector<Transformation*> PaneScene::GetTransformationPointers()
{
    std::vector<Transformation*> txs;
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        txs.push_back(pP->GetTransformationPointer());
    }
    return txs;
}

void PaneScene::ResetTransformation()
{
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        pP->ResetTransformation();
    }
}

void PaneScene::SendMouseClick(int state)
{
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        pP->OnMouseClick(state, 0, 0);
    }
}

void PaneScene::SendHmdTap()
{
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        pP->OnHmdTap();
    }
}

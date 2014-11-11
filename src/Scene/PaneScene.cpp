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
, m_paneShader()
, m_fontShader()
, m_font("../textures/arial.fnt")
, m_panes()
, m_panePts()
{
    m_panePts.push_back(glm::vec3(-0.5f, -0.5f, 0.0f));
    m_panePts.push_back(glm::vec3(0.5f, -0.5f, 0.0f));
    m_panePts.push_back(glm::vec3(0.5f, 0.5f, 0.0f));
    m_panePts.push_back(glm::vec3(-0.5f, 0.5f, 0.0f));

}

PaneScene::~PaneScene()
{
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void PaneScene::_InitPlaneAttributes()
{
    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_paneShader.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, m_panePts.size()*sizeof(glm::vec3), &m_panePts[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(m_paneShader.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const float texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_paneShader.AddVbo("vTexCoord", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_paneShader.GetAttrLoc("vTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_paneShader.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_paneShader.GetAttrLoc("vTexCoord"));
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

    m_paneShader.initProgram("shaderpane");
    m_paneShader.bindVAO();
    _InitPlaneAttributes();
    glBindVertexArray(0);

    // Font Shader
    m_fontShader.initProgram("fontrender");
    m_fontShader.bindVAO();

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_fontShader.AddVbo("vPosition", vertVbo);

    glEnableVertexAttribArray(m_fontShader.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_fontShader.GetAttrLoc("vTexCoord"));

    GLuint triVbo = 0;
    glGenBuffers(1, &triVbo);
    m_fontShader.AddVbo("elements", triVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triVbo);

    glBindVertexArray(0);

    m_font.initGL();
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
        const glm::mat4 object = pP->m_tx.GetMatrix();

        glUseProgram(m_paneShader.prog());
        {
            const glm::mat4 objectMatrix = modelview * object;
            glUniformMatrix4fv(m_paneShader.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(objectMatrix));
            glUniformMatrix4fv(m_paneShader.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));
            pP->DrawPaneWithShader(objectMatrix, projection, m_paneShader);
        }
        glUseProgram(0);
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

    if (glm::length(*m_pHmdRd) == 0)
    {
        return pPane->GetPaneRayIntersectionCoordinates(*m_pHmdRo, glm::vec3(0,0,1), planePt);
    }
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

        pP->m_cursorInPane = false;

        glm::vec2 fmPt(0.0f);
        bool fmInPane = _GetFlyingMouseRightHandPaneRayIntersectionCoordinates(pP, fmPt);
        if (fmInPane)
        {
            pP->m_pointerCoords = fmPt;
            pP->m_cursorInPane = true;
        }

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

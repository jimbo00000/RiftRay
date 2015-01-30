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

PaneScene::PaneScene(bool chassisLocal)
: m_pFm(NULL)
, m_pHmdRo(NULL)
, m_pHmdRd(NULL)
, m_paneShader()
, m_fontShader()
, m_font("../textures/arial.fnt")
, m_panes()
, m_panePts()
, m_chassisLocalSpace(chassisLocal)
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
    const glm::vec3 sumOffset =
        m_pFm->GetChassisPos();
    const glm::mat4 mvLocal = glm::rotate(
        glm::translate(modelview, sumOffset),
        -m_pFm->GetChassisYaw(), glm::vec3(0.f,1.f,0.f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (std::vector<Pane*>::const_iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        const Pane* pP = *it;
        if (pP == NULL)
            continue;

        const glm::mat4 object = pP->m_tx.GetMatrix();
        const glm::mat4& mv = m_chassisLocalSpace ? mvLocal : modelview;

        ///@todo Extract function
        //pP->DrawInScene(m_chassisLocalSpace ? mvLocal : modelview, projection, pP->m_tx.GetMatrix());
        glUseProgram(m_paneShader.prog());
        {
            const glm::mat4 objectMatrix = mv * object;
            glUniformMatrix4fv(m_paneShader.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(objectMatrix));
            glUniformMatrix4fv(m_paneShader.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));
            pP->DrawPaneWithShader(objectMatrix, projection, m_paneShader);
        }
        glUseProgram(0);
    }
    glDisable(GL_BLEND);
}

void PaneScene::RenderPrePass() const
{
    // We must keep the previously bound FBO and restore
    GLint bound_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);
    for (std::vector<Pane*>::const_iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        const Pane* pP = *it;
        if (pP == NULL)
            continue;

        bindFBO(pP->m_paneRenderBuffer);

        pP->DrawToFBO();

        unbindFBO();
    }
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bound_fbo);
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

    float t;
    return pPane->GetPaneRayIntersectionCoordinates(origin3, dir3, planePt, t);
}

bool PaneScene::_GetHmdViewRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt, float& tParam)
{
    if (pPane == NULL)
        return false;
    if (m_pHmdRo == NULL)
        return false;
    if (m_pHmdRd == NULL)
        return false;

    const glm::vec3 origin3 = *m_pHmdRo;
    const glm::vec3 dir3 = *m_pHmdRd;

    if (glm::length(dir3) == 0)
    {
        return pPane->GetPaneRayIntersectionCoordinates(origin3, glm::vec3(0,0,1), planePt, tParam);
    }
    return pPane->GetPaneRayIntersectionCoordinates(origin3, dir3, planePt, tParam);
}

/// Handle Gamepad-HMD motion of panes in space.
void PaneScene::_SetHeldPanePositionAndOrientation(Pane* pP)
{
    if (pP == NULL)
        return;
    if (m_pHmdRo == NULL)
        return;
    if (m_pHmdRd == NULL)
        return;

    const glm::vec3 hmd_origin3 = *m_pHmdRo;
    const glm::vec3 hmd_dir3 = *m_pHmdRd;

    holdingState& hold = pP->m_holdState;
    const glm::vec3 hmdHitPt = hmd_origin3 + hold.m_holdingTPoint * hmd_dir3;
    const glm::vec3 originalPos = hold.m_holdingPosAtClick;
    const glm::vec3 delta = hmdHitPt - hold.m_holdingPoint3;
    pP->m_tx.SetPosition(originalPos + delta);

    // Orient pane towards the viewer
    const glm::vec3 fwd = glm::normalize(hmd_dir3);
    const glm::vec3 upGuess(0.f, 1.f, 0.f);
    const glm::vec3 rightGuess = glm::normalize(glm::cross(fwd, upGuess));
    const glm::vec3 up = glm::cross(rightGuess, fwd);
    const glm::vec3 right = glm::cross(fwd, up);
    const glm::mat4 ori( // Swapped right and fwd to face towards viewer
        glm::vec4(right,0.f),
        glm::vec4(up,0.f),
        glm::vec4(fwd,0.f),
        glm::vec4(glm::vec3(0.f),1.f)
        );
    pP->m_tx.SetOrientation(ori);
}


void PaneScene::timestep(float dt)
{
    if (m_bDraw == false)
        return;

    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        pP->timestep(dt);

        pP->m_cursorInPane = false;

        glm::vec2 hmdPt(0.0f);
        float tHmd = 0.f;
        const bool hmdInPane = _GetHmdViewRayIntersectionCoordinates(pP, hmdPt, tHmd);
        if (hmdInPane)
        {
            pP->m_pointerCoords = hmdPt;
            pP->m_cursorInPane = true;
        }



#if 0
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
#endif


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

            const int modifier = SIXENSE_BUTTON_BUMPER; // match button used in FlyingMouse
            if (m_pFm->IsPressed(FlyingMouse::Right, modifier) == false)
            {
                if (m_pFm->WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_1))
                {
                    pP->OnHydraButton(1);
                }
                if (m_pFm->WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_3))
                {
                    pP->OnHydraButton(2);
                }
            }
#endif
        }

        if (pP->m_holdState.m_holding)
        {
            _SetHeldPanePositionAndOrientation(pP);
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
    if (m_bDraw == false)
        return;

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
    if (m_bDraw == false)
        return;

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
    if (m_bDraw == false)
        return;

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

void PaneScene::SetHoldingFlag(int state)
{
    if (m_bDraw == false)
        return;

    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it)
    {
        Pane* pP = *it;
        if (pP == NULL)
            continue;
        //pP->OnHmdTap();

        holdingState& hold = pP->m_holdState;
        if (state == 0)
        {
            hold.m_holding = false;
        }

        // Set a "holding" flag to true
        if (pP->m_cursorInPane)
        {
            hold.m_holding = (state == 1);

            // Store t param
            glm::vec2 hmdPt(0.0f);
            float tHmd = 0.f;
            const bool hmdInPane = _GetHmdViewRayIntersectionCoordinates(pP, hmdPt, tHmd);
            //if (hmdInPane)
            {
                glm::vec3 hmd_origin3 = *m_pHmdRo;
                glm::vec3 hmd_dir3 = *m_pHmdRd;

                const glm::vec3 hmdHitPt = hmd_origin3 + tHmd * hmd_dir3;
                const glm::vec3 originalPos = glm::vec3(pP->m_tx.GetMatrix() * glm::vec4(0.,0.,0.,1.));

                hold.m_holdingTPoint = tHmd;
                hold.m_holdingPoint3 = hmdHitPt;
                hold.m_holdingPosAtClick = originalPos;
            }
        }
    }
}

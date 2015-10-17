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
, m_chassisTransformCopy(1.f)
, m_paneShader()
, m_fontShader()
, m_font("../textures/arial.fnt")
, m_panes()
, m_panePts()
, m_mouseMotionCooldown()
{
    m_bChassisLocalSpace = chassisLocal;
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
    const glm::mat4& mv = modelview;
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
    glBindFramebuffer(GL_FRAMEBUFFER, bound_fbo);
}

void PaneScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);
    DrawScene(modelview, projection);
}

///@param planePt Intersection point in plane locap coordinates
///@param tParam [out] t parameter along ray of intersection point
bool PaneScene::_GetFlyingMouseRightHandPaneRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt, float& tParam)
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
    // Apply chassis-world transformation
    origin3 = glm::vec3(m_chassisTransformCopy * glm::vec4(origin3, 1.f));
    dir3 = glm::vec3(m_chassisTransformCopy * glm::vec4(dir3, 0.f));
    return pPane->GetPaneRayIntersectionCoordinates(origin3, dir3, planePt, tParam);
}

bool PaneScene::_GetHmdViewRayIntersectionCoordinates(Pane* pPane, glm::vec2& planePt, float& tParam)
{
    if (pPane == NULL)
        return false;
    if (m_pHmdRo == NULL)
        return false;
    if (m_pHmdRd == NULL)
        return false;

    glm::vec3 origin3 = *m_pHmdRo;
    glm::vec3 dir3 = *m_pHmdRd;
    if (glm::length(dir3) == 0)
    {
        dir3 = glm::vec3(0.f, 0.f, -1.f);
    }
    return pPane->GetPaneRayIntersectionCoordinates(origin3, dir3, planePt, tParam);
}

/// Handle Gamepad-HMD motion of panes in space.
void PaneScene::_SetHeldPanePositionAndOrientation(Pane* pP)
{
    if (pP == NULL)
        return;
    const holdingState& hold = pP->m_holdState;

    glm::vec3 origin3;
    glm::vec3 dir3;
    if (hold.m_holdingDevice == 0)
    {
        if (m_pHmdRo == NULL)
            return;
        if (m_pHmdRd == NULL)
            return;
        origin3 = *m_pHmdRo;
        dir3 = *m_pHmdRd;
    }
    else if (hold.m_holdingDevice == 1)
    {
        if (m_pFm == NULL)
            return;
        m_pFm->GetControllerOriginAndDirection(FlyingMouse::Right, origin3, dir3);
    }
    else
    {
        return;
    }

    const glm::vec3 hmdHitPt = origin3 + hold.m_holdingTPoint * dir3;
    const glm::vec3 originalPos = hold.m_holdingPosAtClick;
    const glm::vec3 delta = hmdHitPt - hold.m_holdingPoint3;
    pP->m_tx.SetPosition(originalPos + delta);

    // Orient pane towards the viewer
    const glm::vec3 fwd = glm::normalize(dir3);
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


void PaneScene::timestep(double absTime, double dt)
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
        pP->timestep(absTime, dt);

        if (pP->m_holdState.m_holding)
        {
            _SetHeldPanePositionAndOrientation(pP);
        }

        // Leave mouse alone for one second to resume HMD gaze controls.
        if (m_mouseMotionCooldown.seconds() < 1.f)
            continue;

        pP->m_cursorInPane = false;

        glm::vec2 hmdPt(0.0f);
        float tHmd = 0.f;
        const bool hmdInPane = _GetHmdViewRayIntersectionCoordinates(pP, hmdPt, tHmd);
        if (hmdInPane)
        {
            pP->m_pointerCoords = hmdPt;
            pP->m_cursorInPane = true;
        }

#ifdef USE_SIXENSE
        glm::vec2 fmPt(0.0f);
        float tHydra = 0.f;
        bool fmInPane = _GetFlyingMouseRightHandPaneRayIntersectionCoordinates(pP, fmPt, tHydra);
        if (fmInPane)
        {
            pP->m_pointerCoords = fmPt;
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
            if (m_pFm->WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_1))
            {
                pP->OnMouseClick(1, mx, my);
            }
            else if (m_pFm->WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_1))
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

///@brief Override HMD gaze direction and Hydra mouse motion action for a limited
/// time after each mouse motion input event.
void PaneScene::SendMouseMotion(int x, int y)
{
    if (m_bDraw == false)
        return;
    if (m_panes.empty())
        return;

    // Pass mouse events only to the first pane in the set(AntPane for DashboardScene)
    std::vector<Pane*>::iterator it = m_panes.begin();
    Pane* pP = *it;
    if (pP == NULL)
        return;

    const glm::ivec2 fbsz = pP->GetFBOSize();
    const glm::vec2 normPt(
        static_cast<float>(x) / static_cast<float>(fbsz.x),
        static_cast<float>(y) / static_cast<float>(fbsz.y));
    pP->m_pointerCoords = normPt;
    pP->m_cursorInPane = true;

    pP->OnMouseMove(x, y);
    m_mouseMotionCooldown.reset();
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

        holdingState& hold = pP->m_holdState;
        if (state == 0)
        {
            hold.m_holding = false;
        }

        // Set a "holding" flag to true
        if (pP->m_cursorInPane)
        {
            hold.m_holding = (state == 1);
            hold.m_holdingDevice = -1;

            // Store t param
            glm::vec2 hmdPt(0.0f);
            float tHmd = 0.f;
            const bool hmdInPane = _GetHmdViewRayIntersectionCoordinates(pP, hmdPt, tHmd);
            if (hmdInPane)
            {
                const glm::vec3 hmd_origin3 = *m_pHmdRo;
                const glm::vec3 hmd_dir3 = *m_pHmdRd;
                const glm::vec3 hmdHitPt = hmd_origin3 + tHmd * hmd_dir3;
                const glm::vec3 originalPos = glm::vec3(pP->m_tx.GetMatrix() * glm::vec4(0.,0.,0.,1.));

                hold.m_holdingTPoint = tHmd;
                hold.m_holdingPoint3 = hmdHitPt;
                hold.m_holdingPosAtClick = originalPos;
                hold.m_holdingDevice = 0;
            }
#ifdef USE_SIXENSE
            if (m_pFm != NULL)
            {
                glm::vec2 fmPt(0.0f);
                float tFm = 0.f;
                bool fmInPane = _GetFlyingMouseRightHandPaneRayIntersectionCoordinates(pP, fmPt, tFm);
                if (fmInPane)
                {
                    glm::vec3 fm_origin3;
                    glm::vec3 fm_dir3;
                    m_pFm->GetControllerOriginAndDirection(FlyingMouse::Right, fm_origin3, fm_dir3);
                    const glm::vec3 fmHitPt = fm_origin3 + tFm * fm_dir3;
                    const glm::vec3 originalPos = glm::vec3(pP->m_tx.GetMatrix() * glm::vec4(0.,0.,0.,1.));

                    hold.m_holdingTPoint = tFm;
                    hold.m_holdingPoint3 = fmHitPt;
                    hold.m_holdingPosAtClick = originalPos;
                    hold.m_holdingDevice = 1;
                }
            }
#endif
        }
    }
}

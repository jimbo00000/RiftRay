// MousingQuad.cpp

#include "MousingQuad.h"
#include "MatrixFunctions.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

MousingQuad::MousingQuad()
 : HudQuad()
 , m_cursorShader()
, m_mouseMotionCooldown()
, m_acceptMouseMotion(true)
, m_cursorInPane(true)
, m_pointerCoords(.5f)
{
}

MousingQuad::~MousingQuad()
{
}

void MousingQuad::initGL(ovrSession& session, ovrSizei sz)
{
    HudQuad::initGL(session, sz);

    m_cursorShader.initProgram("basic");
    m_cursorShader.bindVAO();
    _InitPointerAttributes();
    glBindVertexArray(0);
}

void MousingQuad::_InitPointerAttributes()
{
    const glm::vec3 verts[] = {
        glm::vec3(0.f),
        glm::vec3(.05f, .025f, .0f),
        glm::vec3(.025f, .05f, .0f),
    };

    const glm::vec3 cols[] = {
        glm::vec3(1.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f),
        glm::vec3(0.f, 0.f, 1.f),
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_cursorShader.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_cursorShader.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_cursorShader.AddVbo("vColor", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cols), cols, GL_STATIC_DRAW);
    glVertexAttribPointer(m_cursorShader.GetAttrLoc("vColor"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_cursorShader.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_cursorShader.GetAttrLoc("vColor"));
}

void MousingQuad::exitGL(ovrSession& session)
{
    HudQuad::exitGL(session);
}

void MousingQuad::DrawToQuad()
{
    _PrepareToDrawToQuad();
    {
        const float g = .05f;
        glClearColor(g, g, g, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        DrawCursor();
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _FinalizeDrawToQuad();
}

void MousingQuad::DrawCursor()
{
    if (!m_cursorInPane)
        return;

    // Restore current program when we're done; we are rendering to FBO
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

    glUseProgram(m_cursorShader.prog());
    {
        glm::mat4 modelview(1.f);
        modelview = glm::translate(modelview, glm::vec3(m_pointerCoords, 0.f));
        glm::mat4 projection = glm::ortho(0.f, 1.f, 1.f, 0.f, -1.f, 1.f);

        glUniformMatrix4fv(m_cursorShader.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_cursorShader.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

        m_cursorShader.bindVAO();
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
        }
        glBindVertexArray(0);
    }
    glUseProgram(prog);
}

void MousingQuad::MouseClick(int state)
{
}

///@brief Set the mouse cursor coordinates, overriding HMD look.
void MousingQuad::MouseMotion(int x, int y)
{
    if ((m_fbo.w == 0) || (m_fbo.h == 0))
        return;
    m_pointerCoords.x = static_cast<float>(x) / static_cast<float>(m_fbo.w);
    m_pointerCoords.y = static_cast<float>(y) / static_cast<float>(m_fbo.h);
    m_mouseMotionCooldown.reset();
}

///@brief Called on every render, sets the mouse cursor location to where the left eye is pointing
/// (without eye tracking, this is head-forward from the left eye's center).
void MousingQuad::SetHmdEyeRay(ovrPosef pose)
{
    HudQuad::SetHmdEyeRay(pose);

    glm::vec3 ro, rd;
    GetHMDEyeRayPosAndDir(pose, ro, rd);
    const glm::mat4 quadposeMatrix = makeMatrixFromPose(GetPose());

    glm::vec2 planePt;
    float tParam;
    ///@note this is calculated twice; once in base class
    const bool hit = GetPaneRayIntersectionCoordinates(quadposeMatrix, ro, rd, planePt, tParam);
    if (hit == true)
    {
        if (m_mouseMotionCooldown.seconds() > 1.f)
        {
            m_pointerCoords = planePt;
        }
    }
}

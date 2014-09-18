// RaymarchShaderScene.cpp

#include "RaymarchShaderScene.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

RaymarchShaderScene::RaymarchShaderScene()
: m_raymarch()
, m_tx()
, m_pFm(NULL)
{
}

RaymarchShaderScene::~RaymarchShaderScene()
{
}

void RaymarchShaderScene::initGL()
{
    m_raymarch.initProgram("raymarch");
    m_raymarch.bindVAO();
    _InitShaderRectAttributes();
    glBindVertexArray(0);
}

void RaymarchShaderScene::_InitShaderRectAttributes()
{
    const float verts[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f,
    };

    const float texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_raymarch.AddVbo("vPos", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts)*3*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_raymarch.GetAttrLoc("vPos"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_raymarch.AddVbo("vTex", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texs)*3*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_raymarch.GetAttrLoc("vTex"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_raymarch.GetAttrLoc("vPos"));
    glEnableVertexAttribArray(m_raymarch.GetAttrLoc("vTex"));
}

void RaymarchShaderScene::_DrawScreenQuad() const
{
    m_raymarch.bindVAO();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

void RaymarchShaderScene::DrawScene(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const glm::mat4& object) const
{
    glUseProgram(m_raymarch.prog());
    {
        glUniformMatrix4fv(m_raymarch.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_raymarch.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

#if 0
        glm::mat4 objectMatrix = modelview;
        objectMatrix = glm::translate(objectMatrix, glm::vec3(0.0f, 1.0f, 0.0f)); // Raise rotation center above floor
        // Rotate about cube center
        objectMatrix = glm::translate(objectMatrix, glm::vec3(0.5f));
        objectMatrix *= object;
        objectMatrix = glm::translate(objectMatrix, glm::vec3(-0.5f));
#endif
        glUniformMatrix4fv(m_raymarch.GetUniLoc("obmtx"), 1, false, glm::value_ptr(object));

        // Extract viewing parameters encoded in projection matrix.
        // Stereo separation is encoded here in riftskeleton during pre-translate by half IPD.
        const float tweak = glm::value_ptr(projection)[8];
        glUniform1f(m_raymarch.GetUniLoc("u_eyeballCenterTweak"), tweak);
        const float cot_fovby2 = glm::value_ptr(projection)[5];
        glUniform1f(m_raymarch.GetUniLoc("u_fov_y_scale"), 1.0f/cot_fovby2);
        const float aspect = cot_fovby2 / glm::value_ptr(projection)[0];
        glUniform3f(m_raymarch.GetUniLoc("iResolution"), aspect, 1.0, 0.0);

        _DrawScreenQuad();
    }
    glUseProgram(0);
}

void RaymarchShaderScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);

    DrawScene(modelview, projection, m_tx.GetMatrix());
}

void RaymarchShaderScene::timestep(float)
{
    if (m_pFm == NULL)
        return;

    // We could store the current matrices in a local buffer here, but
    // we might as well just get them on demand via the pointer.
    glm::vec3 origin3;
    glm::vec3 dir3;
    m_pFm->GetControllerOriginAndDirection(FlyingMouse::Right, origin3, dir3);

    const glm::vec3 center(0.0f, 0.0f, 0.0f);
    glm::vec4 c4(center, 1.0f); // apply translation too
    c4 = m_tx.GetMatrix() * c4;
    c4 += glm::vec4(glm::vec3(0.0f, 1.5f, 0.0f), 0.0f); // see fragment shaders, hardcoded 1.5 in there
    glm::vec3 c3(c4.x, c4.y, c4.z);

    const float radius = 0.5f;
    glm::vec3 pos, norm;
    const bool hydraHit = glm::intersectRaySphere(
        origin3, dir3,
        c3, radius,
        pos, norm);
    m_tx.m_lock = !hydraHit;

    // Store the t param along controller ray of the hit in the Transformation
    if (m_tx.m_controllerTParamAtClick <= 0.0f)
    {
        const glm::vec3 originToHitPt = pos - origin3;
        const float tParam = glm::length(originToHitPt);
        m_tx.m_controllerTParamAtClick = tParam;
    }

}

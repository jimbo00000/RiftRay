// ShaderToyScene.cpp

#include "ShaderToyScene.h"
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

ShaderToyScene::ShaderToyScene()
: m_pTexLibrary(NULL)
, m_currentShaderToy(NULL)
, m_vao(0)
{
    m_bDraw = false;
}

ShaderToyScene::~ShaderToyScene()
{
    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
    }
}

void ShaderToyScene::initGL()
{
    glGenVertexArrays(1, &m_vao);
}


void ShaderToyScene::_DrawScreenQuad() const
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

///@brief We can't get away with setting these just once on shader change as we need
/// the limited texture units(sometimes only 4) for renderbuffers.
///@todo We could cache some of the locations and texture IDs.
void ShaderToyScene::_SetTextureUniforms(const ShaderToy* pST) const
{
    SetTextureUniforms(pST, m_pTexLibrary);
}

void ShaderToyScene::DrawScene(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const glm::mat4& object) const
{
    ShaderToy* pST = m_currentShaderToy;
    if (pST == NULL)
        return;

    const GLuint prog = pST->prog();

    glUseProgram(prog);
    {
        const GLint u_mv = glGetUniformLocation(prog, "mvmtx");
        const GLint u_pr = glGetUniformLocation(prog, "prmtx");
        const GLint u_ob = glGetUniformLocation(prog, "obmtx");
        glUniformMatrix4fv(u_mv, 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(u_pr, 1, false, glm::value_ptr(projection));
        glUniformMatrix4fv(u_ob, 1, false, glm::value_ptr(object));

        // To transform only the vertices that define the quad being drawn.
        const GLint u_pm = glGetUniformLocation(prog, "paneMatrix");
        glUniformMatrix4fv(u_pm, 1, false, glm::value_ptr(glm::mat4(1.0f)));

        // Extract viewing parameters encoded in projection matrix.
        // Stereo separation is encoded here in riftskeleton during pre-translate by half IPD.
        const float tweak = glm::value_ptr(projection)[8];
        const GLint u_ebc = glGetUniformLocation(prog, "u_eyeballCenterTweak");
        glUniform1f(u_ebc, tweak);

        const GLint u_cf = glGetUniformLocation(prog, "u_fov_y_scale");
        const float cot_fovby2 = glm::value_ptr(projection)[5];
        glUniform1f(u_cf, 1.0f/cot_fovby2);
        //const float aspect = cot_fovby2 / glm::value_ptr(projection)[0];
        //glUniform3f(glGetUniformLocation(prog, "iResolution"), aspect, 1.0, 0.0);

        // Query viewport dimensions and pass to shader
        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, &vp[0]);
        const GLint u_res = glGetUniformLocation(prog, "iResolution");
        glUniform3f(u_res,
            static_cast<float>(vp[2]),
            static_cast<float>(vp[3]),
            0.0f);

        const GLint timeUniLoc = glGetUniformLocation(prog, "iGlobalTime");
        glUniform1f(timeUniLoc, pST->GlobalTime());

        _SetTextureUniforms(pST);
        _DrawScreenQuad();
    }
    glUseProgram(0);
}

void ShaderToyScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);

    DrawScene(modelview, projection, glm::mat4(1.0f));
}

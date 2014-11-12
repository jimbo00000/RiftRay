// ShaderToyScene.cpp

#include "ShaderToyScene.h"
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

ShaderToyScene::ShaderToyScene()
: m_quadVao()
, m_globalTime()
, m_pTexLibrary(NULL)
, m_currentShaderToy(NULL)
{
    m_bDraw = false;
}

ShaderToyScene::~ShaderToyScene()
{
}

void ShaderToyScene::initGL()
{
    m_quadVao.initProgram("raymarch"); ///@todo replace program
    m_quadVao.bindVAO();
    _InitShaderRectAttributes();
    glBindVertexArray(0);
}

void ShaderToyScene::_InitShaderRectAttributes()
{
    const float verts[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f,
    };

    ///@note Texture coordinates are redundant here as we can glean them
    /// from gl_FragCoord in the rwwtt shader.
    const float texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_quadVao.AddVbo("vPos", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts)*3*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_quadVao.GetAttrLoc("vPos"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_quadVao.AddVbo("vTex", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texs)*3*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_quadVao.GetAttrLoc("vTex"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_quadVao.GetAttrLoc("vPos"));
    glEnableVertexAttribArray(m_quadVao.GetAttrLoc("vTex"));
}

void ShaderToyScene::_DrawScreenQuad() const
{
    m_quadVao.bindVAO();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}

///@brief We can't get away with setting these just once on shader change as we need
/// the limited texture units(sometimes only 4) for renderbuffers.
///@todo We could cache some of the locations and texture IDs.
void ShaderToyScene::_SetTextureUniforms(const ShaderToy* pST) const
{
    if (pST == NULL)
        return;
    if (m_pTexLibrary == NULL)
        return;

    for (int i=0; i<4; ++i)
    {
        std::ostringstream oss;
        oss << "iChannel"
            << i;
        const GLint u_samp = glGetUniformLocation(pST->prog(), oss.str().c_str());
        const std::string texname = pST->GetTextureFilenameAtChannel(i);
        const std::map<std::string, textureChannel>::const_iterator it = m_pTexLibrary->find(texname);
        if (it != m_pTexLibrary->end()) // key not found
        {
            const textureChannel& t = it->second;
            if ((u_samp != -1) && (t.texID > 0))
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, t.texID);
                glUniform1i(u_samp, i);
            }
        }
    }
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
        glUniform1f(timeUniLoc, static_cast<float>(m_globalTime.seconds()));

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

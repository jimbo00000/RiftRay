// ShaderPane.cpp

#include "ShaderPane.h"
#include "ShaderToy.h"

ShaderPane::ShaderPane()
: Pane()
, m_pShadertoy(NULL)
{
}

ShaderPane::~ShaderPane()
{
}

void ShaderPane::initGL()
{
    allocateFBO(m_paneRenderBuffer, 600, 600);
}

void ShaderPane::DrawPaneAsPortal(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const glm::mat4& object,
    const ShaderWithVariables& sh) const
{
    ///@todo Consolidate this duplicated code
    ShaderToy* pST = m_pShadertoy;
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
        glUniformMatrix4fv(u_pm, 1, false, glm::value_ptr(projection * modelview));

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

        {
            // Query viewport dimensions
            GLint vp[4];
            glGetIntegerv(GL_VIEWPORT, &vp[0]);

            const GLint u_res = glGetUniformLocation(prog, "iResolution");
            glUniform3f(u_res,
                static_cast<float>(vp[2]),
                static_cast<float>(vp[3]),
                0.0f);
        }

        const GLint timeUniLoc = glGetUniformLocation(prog, "iGlobalTime");
        glUniform1f(timeUniLoc, static_cast<float>(0.0f));//m_globalTime.seconds()));

        ///@todo Consolidate texture library?
        //_SetTextureUniforms(pST);

        sh.bindVAO();
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);
}

///@brief Highlight pane when it's being pointed at.
void ShaderPane::DrawPaneWithShader(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const ShaderWithVariables& sh) const
{
    if (false)
    {
        ///@todo Line up eyePos and headScale to match initial view of shader from our vantage point in 3d
        ///@todo Fade in after time or after a selection tap/press
        if (m_cursorInPane)
        {
            DrawPaneAsPortal(modelview, projection, glm::mat4(1.0f), sh);
            return;
        }
    }

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

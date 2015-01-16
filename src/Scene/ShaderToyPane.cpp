// ShaderToyPane.cpp

#include "ShaderToyPane.h"
#include "ShaderToy.h"

#define _USE_MATH_DEFINES
#include <math.h>

ShaderToyPane::ShaderToyPane(unsigned int px)
: Pane()
, m_pShadertoy(NULL)
, m_pFontShader(NULL)
, m_pFont(NULL)
, m_pGlobalState(NULL)
, m_vao(0)
, m_paneSizePx(px)
{
}

ShaderToyPane::~ShaderToyPane()
{
    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
    }
}

void ShaderToyPane::initGL()
{
    //Pane::initGL();
    allocateFBO(m_paneRenderBuffer, m_paneSizePx, m_paneSizePx);
    glGenVertexArrays(1, &m_vao);
}

void ShaderToyPane::DrawPaneAsPortal(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const glm::mat4& object,
    const glm::mat4& paneMatrix,
    float panePointScale) const
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
        glUniformMatrix4fv(u_pm, 1, false, glm::value_ptr(paneMatrix));

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

        // Query viewport dimensions
        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, &vp[0]);

        const GLint u_res = glGetUniformLocation(prog, "iResolution");
        glUniform3f(u_res,
            static_cast<float>(vp[2]),
            static_cast<float>(vp[3]),
            0.0f);

        const GLint timeUniLoc = glGetUniformLocation(prog, "iGlobalTime");
        const float t = pST->GlobalTime();
        glUniform1f(timeUniLoc, t);

        const GLint u_pps = glGetUniformLocation(prog, "u_panePointScale");
        glUniform1f(u_pps, panePointScale);

        SetTweakUniforms(pST, prog);
        SetTextureUniforms(pST, m_pTexLibrary);

        glBindVertexArray(m_vao);
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }
        glBindVertexArray(0);
    }
    glUseProgram(0);
}

///@brief Draw 3 lines of text in the lower-left of the pane, like an MTV video:
/// Name, author and license.
void ShaderToyPane::DrawShaderInfoText(
    const ShaderWithVariables& fsh,
    const BMFont& fnt
    ) const
{
    bindFBO(m_paneRenderBuffer);

    const ShaderToy* pSt = m_pShadertoy;
    if (pSt == NULL)
        return;

    const int lineh = 62;
    const int margin = 22;
    int txh = 600 - 3*lineh - margin; // tuned to font size and string lengths
    const std::string title = pSt->GetStringByName("title");
    DrawTextOverlay(title.empty() ? pSt->GetSourceFile() : title, margin, txh, fsh, fnt);
    DrawTextOverlay(pSt->GetStringByName("author"), margin, txh += lineh, fsh, fnt);
    DrawTextOverlay(pSt->GetStringByName("license"), margin, txh += lineh, fsh, fnt);

    unbindFBO();
}


///@brief Highlight pane when it's being pointed at.
void ShaderToyPane::DrawPaneWithShader(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const ShaderWithVariables& sh) const
{
    bool portals = false;
    if (m_pGlobalState)
        portals = m_pGlobalState->panesAsPortals;

    if (portals == true)
    {
        ///@todo Line up eyePos and headScale to match initial view of shader from our vantage point in 3d
        ///@todo Fade in after time or after a selection tap/press
        ///@todo Check for viewer position along normal and at the right distance
        if (m_cursorInPane)
        {
            glm::mat4 adjustedMV = modelview;
            const ShaderToy* pSt = m_pShadertoy;
            if (pSt != NULL)
            {
                const glm::vec3 hp = pSt->GetHeadPos();
                adjustedMV = glm::rotate(adjustedMV, static_cast<float>(M_PI), glm::vec3(0.,1.,0.));
                adjustedMV = glm::translate(adjustedMV, -hp);
            }

            DrawPaneAsPortal(adjustedMV, projection, glm::mat4(1.0f), projection*modelview, 0.5f);
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

void ShaderToyPane::RenderThumbnail() const
{
    const ShaderToy* pSt = m_pShadertoy;
    if (pSt == NULL)
        return;

    bindFBO(m_paneRenderBuffer);
    {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        const glm::vec3 hp = pSt->GetHeadPos();
        const glm::vec3 LookVec(0.0f, 0.0f, -1.0f);
        const glm::vec3 up(0.0f, 1.0f, 0.0f);

        const glm::mat4 rot = glm::rotate(glm::mat4(1.0f), static_cast<float>(M_PI), glm::vec3(0.f,1.f,0.f));
        const glm::mat4 modelview = glm::translate(rot, hp*-1.f);

        const glm::mat4 persp = glm::perspective(
            90.0f,
            static_cast<float>(m_paneRenderBuffer.w) / static_cast<float>(m_paneRenderBuffer.h),
            0.004f,
            500.0f);

        DrawPaneAsPortal(
            modelview,
            persp,
            glm::mat4(1.0f));
    }
    unbindFBO();
}

void ShaderToyPane::DrawToFBO() const
{
    if (m_cursorInPane == false)
        return;

    bool animated = false;
    if (m_pGlobalState)
    {
        animated = m_pGlobalState->animatedThumbnails && !m_pGlobalState->panesAsPortals;
    }

    if (animated == false)
        return;

    GLint bound_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &bound_prog);

    // Render a view of the shader to the FBO
    // We must keep the previously bound FBO and restore
    GLint bound_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);

    RenderThumbnail();
    if (m_pFontShader!=NULL && m_pFont!=NULL)
    {
        glDisable(GL_DEPTH_TEST);
        DrawShaderInfoText(*m_pFontShader, *m_pFont);
        glEnable(GL_DEPTH_TEST);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bound_fbo);

    glUseProgram(bound_prog);
}

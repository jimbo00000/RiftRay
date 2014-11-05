// ShaderPane.cpp

#include "ShaderPane.h"

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
    m_font.initGL();
}

///@brief Highlight pane when it's being pointed at.
void ShaderPane::DrawPaneWithShader(const ShaderWithVariables& sh) const
{
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

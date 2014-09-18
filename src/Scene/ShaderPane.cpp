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
    m_plane.initProgram("shaderpane");
    m_plane.bindVAO();
    _InitPlaneAttributes();
    glBindVertexArray(0);

    allocateFBO(m_paneRenderBuffer, 600, 600);
}

///@brief Highlight pane when it's being pointed at.
void ShaderPane::DrawPane() const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_paneRenderBuffer.tex);
    glUniform1i(m_plane.GetUniLoc("fboTex"), 0);

    glUniform1f(m_plane.GetUniLoc("u_brightness"), m_cursorInPane ? 1.0f : 0.5f);

    m_plane.bindVAO();
    {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
}

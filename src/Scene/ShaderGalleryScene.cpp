// ShaderGalleryScene.cpp

#include "ShaderGalleryScene.h"
#include "ShaderToyPane.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

ShaderGalleryScene::ShaderGalleryScene()
: PaneScene()
, m_pActiveShaderToy(NULL)
, m_pActiveShaderToyPane(NULL)
, m_pTexLibrary(NULL)
, m_paneDimensionPixels(400)
, m_globalShadertoyState()
{
}

ShaderGalleryScene::~ShaderGalleryScene()
{
}

Pane* ShaderGalleryScene::AddShaderToyPane(ShaderToy* pSt)
{
    ShaderToyPane* pP = new ShaderToyPane(m_paneDimensionPixels);
    if (pP == NULL)
        return NULL;

    const int idx = static_cast<int>(m_panes.size());
    ///@todo Scalable positioning
    const glm::vec3 pos(-8.0f + static_cast<float>(idx)*1.1f, 1.5f, -2.0f);
    pP->m_tx.SetPosition(pos);
    pP->m_tx.SetDefaultPosition(pos);

    const glm::mat4 ori = glm::mat4(1.0f);
    pP->m_tx.SetDefaultOrientation(ori);
    pP->m_tx.SetOrientation(ori);

    pP->m_pShadertoy = pSt;
    pP->SetTextureLibraryPointer(m_pTexLibrary);
    // It feels ugly to do all this pointer setting, but is it worse than a singleton?
    pP->SetFontShaderPointer(&m_fontShader);
    pP->SetFontPointer(&m_font);
    pP->SetGlobalStatePointer(&m_globalShadertoyState);

    m_panes.push_back(pP);

    return pP;
}

void ShaderGalleryScene::RearrangePanes()
{
    int idx = 0;
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it, ++idx)
    {
        Pane* pP = *it;

        const int numrows = 3;
        const int rowsz = 1 + static_cast<int>(m_panes.size()) / numrows;
        const int rownum = idx / rowsz;
        const int rowpos = idx % rowsz;
        const float colstep = 1.1f;
#if 0
        // Lay the panes out in cylindrical rows in front of the viewer.
        const float radstep = static_cast<float>(M_PI) / 16.0f;
        const float rads = static_cast<float>(rowpos-rowsz/2) * radstep;
        const float radius = 6.0f;

        const glm::vec3 pos(
            radius*sin(rads),
            0.8f + colstep * static_cast<float>(rownum),
            2.0f - radius*cos(rads));
        const glm::mat4 ori = glm::rotate(glm::mat4(1.0f), -rads, glm::vec3(0,1,0));
#else
        // Lay out panes in a flat grid
        const float xspacing = 1.1f;
        const glm::vec3 pos(
            static_cast<float>(rowpos-rowsz/2) * xspacing,
            0.8f + colstep * static_cast<float>(rownum),
            -6.0f);
        const glm::mat4 ori = glm::mat4(1.0f);
#endif

        pP->m_tx.SetPosition(pos);
        pP->m_tx.SetDefaultPosition(pos);

        pP->m_tx.SetDefaultOrientation(ori);
        pP->m_tx.SetOrientation(ori);
    }
}

void ShaderGalleryScene::CompileShaders()
{
    std::vector<Pane*>& panes = m_panes;
    for (std::vector<Pane*>::iterator it = panes.begin();
        it != panes.end();
        ++it)
    {
        ShaderToyPane* pP = reinterpret_cast<ShaderToyPane*>(*it);
        if (pP == NULL)
            continue;
        ShaderToy* pSt = pP->m_pShadertoy;
        if (pSt == NULL)
            continue;

        Timer t;
        pSt->CompileShader();

        std::cout
            << "\t\t "
            << t.seconds()
            << "s"
            ;
    }
}

const ShaderToyPane* ShaderGalleryScene::GetFocusedPane() const
{
    int idx = 0;
    for (std::vector<Pane*>::const_iterator it = m_panes.begin();
        it != m_panes.end();
        ++it, ++idx)
    {
        const ShaderToyPane* pP = reinterpret_cast<ShaderToyPane*>(*it);
        if (pP == NULL)
            continue;
        if (pP->m_cursorInPane)
            return pP;
    }

    return NULL;
}

void ShaderGalleryScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    if (m_pActiveShaderToy == NULL)
    {
        // Draw the gallery of panes
        PaneScene::RenderForOneEye(pMview, pPersp);
        return;
    }

    const ShaderToyPane* pP = GetActiveShaderToyPane();
    if (pP == NULL)
        return;

    // Draw only the current ShaderToy encompassing the world
    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);

    pP->DrawPaneAsPortal(
        modelview,
        projection,
        glm::mat4(1.0f));
}

void ShaderGalleryScene::RenderThumbnails() const
{
    // Render a view of the shader to the FBO
    // We must keep the previously bound FBO and restore
    GLint bound_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);

    const ShaderWithVariables& fsh = GetFontShader();
    const BMFont& fnt = GetFont();
    const std::vector<Pane*>& panes = m_panes;
    for (std::vector<Pane*>::const_iterator it = panes.begin();
        it != panes.end();
        ++it)
    {
        const ShaderToyPane* pP = reinterpret_cast<ShaderToyPane*>(*it);
        if (pP == NULL)
            continue;

        pP->RenderThumbnail();
        pP->DrawShaderInfoText(fsh, fnt);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bound_fbo);
}

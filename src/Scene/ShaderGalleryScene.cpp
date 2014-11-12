// ShaderGalleryScene.cpp

#include "ShaderGalleryScene.h"
#include "ShaderPane.h"

#define _USE_MATH_DEFINES
#include <math.h>

ShaderGalleryScene::ShaderGalleryScene()
{
}

ShaderGalleryScene::~ShaderGalleryScene()
{
}

Pane* ShaderGalleryScene::AddShaderPane(ShaderToy* pSt)
{
    ShaderPane* pP = new ShaderPane();
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

        // Lay the panes out in cylindrical rows in front of the viewer.
        const int numrows = 3;
        const int rowsz = 1 + static_cast<int>(m_panes.size()) / numrows;
        const int rownum = idx / rowsz;
        const int rowpos = idx % rowsz;
        const float colstep = 1.1f;
        const float radstep = static_cast<float>(M_PI) / 16.0f;
        const float rads = static_cast<float>(rowpos-rowsz/2) * radstep;
        const float radius = 6.0f;

        const glm::vec3 pos(
            radius*sin(rads),
            0.8f + colstep * static_cast<float>(rownum),
            2.0f - radius*cos(rads));
        pP->m_tx.SetPosition(pos);
        pP->m_tx.SetDefaultPosition(pos);

        const glm::mat4 ori = glm::rotate(glm::mat4(1.0f), -rads, glm::vec3(0,1,0));
        pP->m_tx.SetDefaultOrientation(ori);
        pP->m_tx.SetOrientation(ori);
    }
}

ShaderToy* ShaderGalleryScene::GetFocusedShader() const
{
    int idx = 0;
    for (std::vector<Pane*>::const_iterator it = m_panes.begin();
        it != m_panes.end();
        ++it, ++idx)
    {
        const ShaderPane* pP = reinterpret_cast<ShaderPane*>(*it);
        if (pP == NULL)
            continue;
        if (pP->m_cursorInPane)
            return pP->m_pShadertoy;
    }

    return NULL;
}

// HydraScene.cpp
#ifdef USE_SIXENSE
#include "HydraScene.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

HydraScene::HydraScene()
: m_basic()
, m_hydra()
, m_numPts(0)
, m_numTris(0)
, m_pFm(NULL)
{
}

HydraScene::~HydraScene()
{
}

void HydraScene::initGL()
{
    m_basic.initProgram("basic");
    m_basic.bindVAO();
    _InitOriginAttributes();
    glBindVertexArray(0);

    m_hydra.initProgram("hydrabase");
    m_hydra.bindVAO();
    _InitHydraModelAttributes();
    glBindVertexArray(0);
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void HydraScene::_InitOriginAttributes()
{
    const glm::vec3 verts[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),

        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),

        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),

        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -5.0f),
    };

    const glm::vec3 cols[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),

        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),

        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),

        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(0.5f, 0.5f, 0.5f),
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_basic.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts)*3*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_basic.AddVbo("vColor", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cols)*3*sizeof(GLfloat), cols, GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vColor"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_basic.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_basic.GetAttrLoc("vColor"));
}

void HydraScene::_InitHydraModelAttributes()
{
    const glm::vec3 verts[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
    };

    std::vector<glm::vec3> meshverts;
    meshverts.push_back(verts[0]);

    const int subdivision = 5;
    const glm::vec3 startPtx = verts[0];
    const glm::vec3 endPtx = verts[1];
    const glm::vec3 startPty = verts[0];
    const glm::vec3 endPty = verts[2];
    for (int j=0; j<=subdivision+1; ++j)
    {
        const float u = static_cast<float>(j) / static_cast<float>(subdivision+1);
        const glm::vec3 startPt = (1.0f-u)*startPtx + u*endPtx;
        const glm::vec3 endPt = (1.0f-u)*startPty + u*endPty;

        if (j != 0)
            meshverts.push_back(startPt);
        for (int i=0; i<j-1; ++i)
        {
            const float t = static_cast<float>(i+1) / static_cast<float>(j);
            const glm::vec3 pt = (1.0f-t)*startPt + t*endPt;
            meshverts.push_back(pt);
        }
        if (j != 0)
            meshverts.push_back(endPt);
    }
    m_numPts = meshverts.size();

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_hydra.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, meshverts.size()*sizeof(glm::vec3), &meshverts[0], GL_STATIC_DRAW);
    glVertexAttribPointer(m_hydra.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_hydra.AddVbo("vColor", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(cols), cols, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, meshverts.size()*sizeof(glm::vec3), &meshverts[0], GL_STATIC_DRAW);
    glVertexAttribPointer(m_hydra.GetAttrLoc("vColor"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_hydra.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_hydra.GetAttrLoc("vColor"));

    std::vector<glm::ivec3> indices;
    int basevert = 0;
    for (int j=0; j<=subdivision; ++j)
    {
        const int rowvcnt = j+1;
        for (int i=0; i<rowvcnt; ++i)
        {
            const int v = i+basevert;
            const glm::ivec3 tri(v, v+rowvcnt, v+rowvcnt+1);
            indices.push_back(tri);
            if (i > 0)
            {
                const glm::ivec3 tri2(tri.x-1, tri.x, tri.y);
                indices.push_back(tri2);
            }
        }
        basevert += j+1;
    }
    m_numTris = indices.size();

    GLuint quadVbo = 0;
    glGenBuffers(1, &quadVbo);
    m_hydra.AddVbo("elements", quadVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(glm::ivec3), &indices[0], GL_STATIC_DRAW);
}

void HydraScene::_DrawOrigin(int verts) const
{
    m_basic.bindVAO();
    glDrawArrays(GL_LINES, 0, verts);
    glBindVertexArray(0);
}

void HydraScene::_DrawHydraModel() const
{
    m_hydra.bindVAO();
    //glPointSize(10.0f);
    //glDrawArrays(GL_POINTS, 0, m_numPts);

    glDrawElements(GL_TRIANGLES,
                   m_numTris*3,
                   GL_UNSIGNED_INT,
                   0);
    glBindVertexArray(0);
}

// Draw the scene(matrices have already been set up).
void HydraScene::DrawScene(
    const glm::mat4& modelview,
    const glm::mat4& projection) const
{
    const glm::vec3 sumOffset = m_pFm->m_baseOffset + m_pFm->GetChassisPos();

    glUseProgram(m_basic.prog());
    {
        if (m_pFm != NULL)
        {
            glUniformMatrix4fv(m_basic.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));
            const glm::mat4 mv = glm::translate(modelview, sumOffset);

            // Draw a unit origin at the base
            if(0)
            {
                const glm::mat4 id = mv * glm::mat4(1.0f);
                glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(id));
                _DrawOrigin(6);
            }


            // Flip the left handle's lines in its local frame
            const glm::mat4 mL = mv * glm::scale(glm::make_mat4(m_pFm->mtxL), glm::vec3(-1.0f, 1.0f, 1.0f));
            glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(mL));
            _DrawOrigin(6);

            if (m_pFm->ControllerIsOnBase(FlyingMouse::Right) == false)
            {
                const glm::mat4 mR = mv * glm::make_mat4(m_pFm->mtxR);
                glUniformMatrix4fv(m_basic.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(mR));
                _DrawOrigin(8);
            }
        }
    }

    glUseProgram(m_hydra.prog());
    {
        if (m_pFm != NULL)
        {
            glUniformMatrix4fv(m_hydra.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));
            glm::mat4 mv = glm::translate(modelview, sumOffset);

            // Draw the base at the origin
            for (int j=0; j<2; ++j)
            {
                for (int i=0; i<4; ++i)
                {
                    glUniformMatrix4fv(m_hydra.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(mv));
                    _DrawHydraModel(); // a single octant
                    mv = glm::rotate(mv, static_cast<float>(M_PI)*0.5f, glm::vec3(1,0,0));
                }
                mv = glm::rotate(mv, static_cast<float>(M_PI), glm::vec3(0,1,0));
            }
        }
    }

    glUseProgram(0);
}

void HydraScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);

    DrawScene(modelview, projection);
}
#endif

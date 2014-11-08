// Pane.cpp

#include "Pane.h"
#include "BMFont.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

Pane::Pane()
: m_plane()
, m_cursorShader()
, m_paneRenderBuffer()
, m_cursorInPane(false)
, m_pointerCoords(0.0f)
{
    m_panePts.push_back(glm::vec3(-0.5f, -0.5f, 0.0f));
    m_panePts.push_back(glm::vec3(0.5f, -0.5f, 0.0f));
    m_panePts.push_back(glm::vec3(0.5f, 0.5f, 0.0f));
    m_panePts.push_back(glm::vec3(-0.5f, 0.5f, 0.0f));

    m_tx.TranslatePosition(glm::vec3(0.5f, 1.0f, 0.0f));
}

Pane::~Pane()
{
}

void Pane::initGL()
{
    m_cursorShader.initProgram("basic");
    m_cursorShader.bindVAO();
    _InitPointerAttributes();
    glBindVertexArray(0);

    m_plane.initProgram("basictex");
    m_plane.bindVAO();
    _InitPlaneAttributes();
    glBindVertexArray(0);

    allocateFBO(m_paneRenderBuffer, 600, 600);
}

void Pane::_InitPointerAttributes()
{
    const glm::vec3 verts[] = {
        glm::vec3(0.0f),
        glm::vec3(0.1f, 0.05f, 0.0f),
        glm::vec3(0.05f, 0.1f, 0.0f),
    };

    const glm::vec3 cols[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_cursorShader.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_cursorShader.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_cursorShader.AddVbo("vColor", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cols), cols, GL_STATIC_DRAW);
    glVertexAttribPointer(m_cursorShader.GetAttrLoc("vColor"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_cursorShader.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_cursorShader.GetAttrLoc("vColor"));
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void Pane::_InitPlaneAttributes()
{
    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_plane.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, m_panePts.size()*sizeof(glm::vec3), &m_panePts[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer(m_plane.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const float texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_plane.AddVbo("vTexCoord", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_plane.GetAttrLoc("vTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_plane.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_plane.GetAttrLoc("vTexCoord"));
}

void Pane::DrawCursor() const
{
    if (!m_cursorInPane)
        return;

    // Restore current program when we're done; we are rendering to FBO
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

    glUseProgram(m_cursorShader.prog());
    {
        glm::mat4 modelview(1.0f);
        modelview = glm::translate(modelview, glm::vec3(m_pointerCoords, 0.0f));
        glm::mat4 projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);

        glUniformMatrix4fv(m_cursorShader.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_cursorShader.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));

        m_cursorShader.bindVAO();
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
        }
        glBindVertexArray(0);
    }
    glUseProgram(prog);
}


void Pane::DrawTextOverlay(
    const std::string text,
    int x,
    int y,
    const ShaderWithVariables& sh,
    const BMFont& font) const
{
    const glm::mat4 modelview(1.0f);
    const glm::mat4 projection = glm::ortho(
        0.0f,
        static_cast<float>(m_paneRenderBuffer.w),
        static_cast<float>(m_paneRenderBuffer.h),
        0.0f,
        -1.0f,
        1.0f);

    font.DrawString(text, x, y, modelview, projection, sh);
}

void Pane::DrawPane() const
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_paneRenderBuffer.tex);
    glUniform1i(m_plane.GetUniLoc("fboTex"), 0);

    m_plane.bindVAO();
    {
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
}

void Pane::DrawToFBO() const
{
    if (m_cursorInPane || !m_tx.m_lockedAtClickPos)
        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
    else
        glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawCursor();
}



/// Draw the scene(matrices have already been set up).
void Pane::DrawInScene(
    const glm::mat4& modelview,
    const glm::mat4& projection,
    const glm::mat4& object) const
{
    glUseProgram(m_plane.prog());
    {
        const glm::mat4 objectMatrix = modelview * object;
        //glUniformMatrix4fv(m_plane.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(modelview));
        glUniformMatrix4fv(m_plane.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(objectMatrix));
        glUniformMatrix4fv(m_plane.GetUniLoc("prmtx"), 1, false, glm::value_ptr(projection));
        DrawPane();
    }
    glUseProgram(0);
}



std::vector<glm::vec3> Pane::GetTransformedPanePoints() const
{
    std::vector<glm::vec3> pts = m_panePts; // make a copy to transform pts by object matrix
    for (std::vector<glm::vec3>::iterator it = pts.begin();
        it != pts.end();
        ++it)
    {
        glm::vec3& p = *it;
        glm::vec4 p4(p, 1.0f); // apply translation too
        p4 = m_tx.GetMatrix() * p4;
        // The #define GLM_SWIZZLE directive makes glm really spew in VS2010
        p.x = p4.x;
        p.y = p4.y;
        p.z = p4.z;
    }
    return pts;
}


///@param [out] planePt Intersection point on plane in local normalized coordinates
///@return true if ray hits pane quad, false otherwise
bool Pane::GetPaneRayIntersectionCoordinates(glm::vec3 origin3, glm::vec3 dir3, glm::vec2& planePt)
{
    std::vector<glm::vec3> pts = GetTransformedPanePoints();
    glm::vec3 retval1(0.0f);
    glm::vec3 retval2(0.0f);
    const bool hit1 = glm::intersectLineTriangle(origin3, dir3, pts[0], pts[1], pts[2], retval1);
    const bool hit2 = glm::intersectLineTriangle(origin3, dir3, pts[0], pts[2], pts[3], retval2);
    if ( !(hit1||hit2) )
        return false;

    glm::vec3 hitval(0.0f);
    glm::vec3 cartesianpos(0.0f);
    if (hit1)
    {
        hitval = retval1;
        // At this point, retval1 or retval2 contains hit data returned from glm::intersectLineTriangle.
        // This does not appear to be raw - y and z appear to be barycentric coordinates.
        // Fill out the x coord with the barycentric identity then convert using simple weighted sum.
        hitval.x = 1.0f - hitval.y - hitval.z;
        cartesianpos = 
            hitval.x * pts[0] +
            hitval.y * pts[1] +
            hitval.z * pts[2];
    }
    else if (hit2)
    {
        hitval = retval2;
        hitval.x = 1.0f - hitval.y - hitval.z;
        cartesianpos = 
            hitval.x * pts[0] +
            hitval.y * pts[2] +
            hitval.z * pts[3];
    }

    // Store the t param along controller ray of the hit in the Transformation
    if (m_tx.m_controllerTParamAtClick <= 0.0f)
    {
        const glm::vec3 originToHitPt = cartesianpos - origin3;
        const float tParam = glm::length(originToHitPt);
        m_tx.m_controllerTParamAtClick = tParam;
    }

    const glm::vec3 v1 = pts[1] - pts[0]; // x axis
    const glm::vec3 v2 = pts[3] - pts[0]; // y axis
    const float len = glm::length(v1); // v2 length should be equal
    const glm::vec3 vh = (cartesianpos - pts[0]) / len;
    planePt = glm::vec2(
               glm::dot(v1/len, vh),
        1.0f - glm::dot(v2/len, vh) // y coord flipped by convention
        );

    return true;
}

void Pane::OnHmdTap()
{
    OnMouseClick(1, m_pointerCoords.x, m_pointerCoords.y);
    OnMouseClick(0, m_pointerCoords.x, m_pointerCoords.y);
}

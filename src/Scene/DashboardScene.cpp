// DashboardScene.cpp

#include "DashboardScene.h"

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include <stdlib.h>
#include <string.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include "Logger.h"

DashboardScene::DashboardScene()
: PaneScene(true)
, m_antPane()
, m_pngPane()
, m_camPane()
{
    // Like OVRSDK05AppSkeleton's ctor with vector of Scenes, add as many as desired here
    // and set initial conditions. Some may have awkward custom pointer setters.
    const glm::vec3 pos(-0.2f, 0.0f, -0.7f);
    m_antPane.m_tx.SetPosition(pos);
    m_antPane.m_tx.SetDefaultPosition(pos);
    const glm::mat4 ori = glm::rotate(glm::mat4(1.0f), 0.6f, glm::vec3(0,1,0));
    m_antPane.m_tx.SetDefaultOrientation(ori);
    m_antPane.m_tx.SetOrientation(ori);

    {
        const glm::vec3 pos(0.7f, 0.0f, -0.7f);
        m_pngPane.m_tx.SetPosition(pos);
        m_pngPane.m_tx.SetDefaultPosition(pos);
        const glm::mat4 ori = glm::rotate(glm::mat4(1.0f), -0.6f, glm::vec3(0, 1, 0));
        m_pngPane.m_tx.SetDefaultOrientation(ori);
        m_pngPane.m_tx.SetOrientation(ori);
        m_pngPane.m_visible = true;
    }

    {
        const glm::vec3 pos(.3f, -.5f, -.9f);
        m_camPane.m_tx.SetPosition(pos);
        m_camPane.m_tx.SetDefaultPosition(pos);
        const glm::mat4 ori = glm::rotate(glm::mat4(1.0f), -0.2f, glm::vec3(1, 0, 0));
        m_camPane.m_tx.SetDefaultOrientation(ori);
        m_camPane.m_tx.SetOrientation(ori);
        m_camPane.m_visible = true;

        m_camPane.m_tx.m_accumulatedScale = .3f; // smaller window
    }

    m_panes.push_back(&m_antPane);
    m_panes.push_back(&m_pngPane);
    m_panes.push_back(&m_camPane);
    m_bDraw = false;
}

DashboardScene::~DashboardScene()
{
}

// DashboardScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "PaneScene.h"
#include "AntPane.h"

///@brief 
class DashboardScene : public PaneScene
{
public:
    DashboardScene();
    virtual ~DashboardScene();

    // This special case is for the AntTweakBar
    virtual glm::ivec2 GetAntFBOSize() const { return m_antPane.GetFBOSize(); }

protected:
    AntPane m_antPane;

private: // Disallow copy ctor and assignment operator
    DashboardScene(const DashboardScene&);
    DashboardScene& operator=(const DashboardScene&);
};

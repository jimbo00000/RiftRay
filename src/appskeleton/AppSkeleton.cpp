// AppSkeleton.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <GL/glew.h>

#include "AppSkeleton.h"

#include "GL/GLUtils.h"
#include "GL/ShaderFunctions.h"
#include "MatrixMath.h"
#include "utils/Logger.h"


AppSkeleton::AppSkeleton()
: m_windowWidth(1000)
, m_windowHeight(800)
{
}

AppSkeleton::~AppSkeleton()
{
}

/// Glew should be the first thing we initialize. After that, all initialization of GL state.
bool AppSkeleton::initGL(int argc, char **argv)
{
#if 1
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        // Problem: glewInit failed, something is seriously wrong.
        LOG_INFO("Error: %s\n", glewGetErrorString(err));
    }
    LOG_INFO("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    _InitShaders();

    return true;
}

// AppSkeleton.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
//#include <GL/GL.h>

#ifdef USE_CUDA
#else
#  include "vector_make_helpers.h"
#endif

#include "OVRkill.h"

///@brief Only the most basic init functions and window dimensions.
class AppSkeleton
{
public:
    AppSkeleton();
    virtual ~AppSkeleton();

    virtual void display(bool isControl=false, OVRkill::DisplayMode mode=OVRkill::SingleEye) {}
    virtual void mouseDown(int button, int state, int x, int y) {}
    virtual void mouseMove(int x, int y) {}
    virtual void mouseWheel(int x, int y) {}
    virtual void keyboard(int key, int action, int x, int y) {}
    virtual void resize(int w, int h) {}
    virtual bool initGL(int argc, char **argv);
    
    virtual const int w() const { return m_windowWidth; }
    virtual const int h() const { return m_windowHeight; }

protected:
    virtual void _InitShaders() {}

    int m_windowWidth;
    int m_windowHeight;

private: // Disallow copy ctor and assignment operator
    AppSkeleton(const AppSkeleton&);
    AppSkeleton& operator=(const AppSkeleton&);
};

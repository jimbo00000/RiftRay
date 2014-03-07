// AntOculusAppSkeleton.h

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

#include "OculusAppSkeleton.h"

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include "FPSTimer.h"
#include "OVRkill.h"

class ParamListGL;

///@brief Extends OculusAppSkeleton adding an AntTweakBar, which is
/// only displayed to the Control window.
class AntOculusAppSkeleton : public OculusAppSkeleton
{
public:
    AntOculusAppSkeleton();
    virtual ~AntOculusAppSkeleton();

    virtual void display(bool isControl=false, OVRkill::DisplayMode mode=OVRkill::SingleEye);
    virtual void mouseDown(int button, int state, int x, int y);
    virtual void mouseMove(int x, int y);
    virtual void mouseWheel(int x, int y);
    virtual void keyboard(int key, int action, int x, int y);
    virtual void charkey(unsigned int key);
    virtual void resize(int w, int h);
    virtual bool initGL(int argc, char **argv);
    virtual void frameStart() { m_timer.OnFrame(); m_fps = m_timer.GetFPS(); }

#ifdef USE_ANTTWEAKBAR
    virtual void GetShaderParams();
#endif

    virtual void displayTexSampler() const;

    float GetMegaPixelsPerSecond() const { return (float)GetMegaPixelCount() * m_fps; }

protected:
    FPSTimer  m_timer;
    float     m_fps;

#ifdef USE_ANTTWEAKBAR
    void _InitializeBar();
    TwBar* m_pBar;
    double speed;
#endif

private: // Disallow copy ctor and assignment operator
    AntOculusAppSkeleton(const AntOculusAppSkeleton&);
    AntOculusAppSkeleton& operator=(const AntOculusAppSkeleton&);
};

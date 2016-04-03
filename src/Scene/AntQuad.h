// AntQuad.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#include "MousingQuad.h"

///@brief Draws an AntTweakBar to a quad
class AntQuad : public MousingQuad
{
public:
    AntQuad();
    virtual ~AntQuad();

    virtual void initGL(ovrSession& session, ovrSizei sz);
    virtual void exitGL(ovrSession& session);
    virtual void DrawToQuad();
    virtual void MouseClick(int state);
    virtual void MouseMotion(int x, int y);
    virtual void SetHmdEyeRay(ovrPosef pose);

protected:

private: // Disallow copy ctor and assignment operator
    AntQuad(const AntQuad&);
    AntQuad& operator=(const AntQuad&);
};

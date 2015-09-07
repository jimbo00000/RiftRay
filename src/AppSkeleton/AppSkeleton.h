// AppSkeleton.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "FBO.h"
#include "HydraScene.h"

class AppSkeleton
{
public:
    AppSkeleton();
    virtual ~AppSkeleton();

    virtual void ResetChassisTransformations();
    virtual void initGL();

    // For eye ray tracking - set during draw function
    mutable glm::vec3 m_hmdRo;
    mutable glm::vec3 m_hmdRd;
    mutable glm::vec3 m_hmdRoLocal;
    mutable glm::vec3 m_hmdRdLocal;

protected:
    std::vector<IScene*> m_scenes;

    glm::vec3 m_chassisPos;
    float m_chassisYaw;
    float m_chassisPitch;
    float m_chassisRoll;

public:
    FlyingMouse m_fm;
    glm::vec3 m_keyboardMove;
    glm::vec3 m_joystickMove;
    glm::vec3 m_mouseMove;
    float m_keyboardYaw;
    float m_joystickYaw;
    float m_mouseDeltaYaw;
    float m_keyboardDeltaPitch;
    float m_keyboardDeltaRoll;

private: // Disallow copy ctor and assignment operator
    AppSkeleton(const AppSkeleton&);
    AppSkeleton& operator=(const AppSkeleton&);
};

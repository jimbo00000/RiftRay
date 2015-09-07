// AppSkeleton.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AppSkeleton.h"
#include "MatrixFunctions.h"

AppSkeleton::AppSkeleton()
: m_fm()
, m_keyboardMove(0.f)
, m_joystickMove(0.f)
, m_mouseMove(0.f)
, m_keyboardYaw(0.f)
, m_joystickYaw(0.f)
, m_mouseDeltaYaw(0.f)
, m_keyboardDeltaPitch(0.f)
, m_keyboardDeltaRoll(0.f)
, m_chassisYaw(0.0f)
{
    ResetChassisTransformations();
}

AppSkeleton::~AppSkeleton()
{
    m_fm.Destroy();
}

void AppSkeleton::ResetChassisTransformations()
{
    m_chassisPos = glm::vec3(0.f, 1.27f, 1.f); // my sitting height
    m_chassisYaw = 0.f;
    m_chassisPitch = 0.f;
    m_chassisRoll = 0.f;
}

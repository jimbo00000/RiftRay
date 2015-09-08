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
: m_hmdRo(0.f)
, m_hmdRd(0.f)

, m_galleryScene()
, m_raymarchScene()
, m_dashScene()
, m_ovrScene()
, m_floorScene()
#ifdef USE_SIXENSE
, m_hydraScene()
#endif

, m_scenes()
, m_chassisYaw(0.f)
, m_hyif()
, m_fm()
, m_keyboardMove(0.f)
, m_joystickMove(0.f)
, m_mouseMove(0.f)
, m_keyboardYaw(0.f)
, m_joystickYaw(0.f)
, m_mouseDeltaYaw(0.f)
, m_keyboardDeltaPitch(0.f)
, m_keyboardDeltaRoll(0.f)
, m_cinemaScopeFactor(0.f)
{
    // Add as many scenes here as you like. They will share color and depth buffers,
    // so drawing one after the other should just result in pixel-perfect integration -
    // provided they all do forward rendering. Per-scene deferred render passes will
    // take a little bit more work.
    //m_scenes.push_back(&m_raymarchScene);
    m_scenes.push_back(&m_galleryScene);
    m_scenes.push_back(&m_ovrScene);
    m_scenes.push_back(&m_dashScene);
    m_scenes.push_back(&m_floorScene);
#ifdef USE_SIXENSE
    m_scenes.push_back(&m_hydraScene);
#endif

    m_raymarchScene.SetFlyingMousePointer(&m_fm);
    m_galleryScene.SetFlyingMousePointer(&m_fm);
    m_galleryScene.SetHmdPositionPointer(&m_hmdRo);
    m_galleryScene.SetHmdDirectionPointer(&m_hmdRd);
    m_dashScene.SetFlyingMousePointer(&m_fm);
    m_dashScene.SetHmdPositionPointer(&m_hmdRoLocal);
    m_dashScene.SetHmdDirectionPointer(&m_hmdRdLocal);

    // Give this scene a pointer to get live Hydra data for display
#ifdef USE_SIXENSE
    m_hydraScene.SetFlyingMousePointer(&m_fm);
    m_hyif.AddTransformation(m_raymarchScene.GetTransformationPointer());
#endif

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

void AppSkeleton::initGL()
{
    for (std::vector<IScene*>::iterator it = m_scenes.begin();
        it != m_scenes.end();
        ++it)
    {
        IScene* pScene = *it;
        if (pScene != NULL)
        {
            pScene->initGL();
        }
    }
}

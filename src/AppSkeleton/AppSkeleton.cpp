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
, m_fboScale(1.f)
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
, m_headSize(1.f)
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

    m_raymarchScene.ResetTransformation();

    const ShaderToy* pST = m_galleryScene.GetActiveShaderToy();
    if (pST != NULL)
    {
        m_chassisPos = pST->GetHeadPos();
        m_chassisYaw = static_cast<float>(M_PI);
    }
}

glm::mat4 AppSkeleton::makeWorldToChassisMatrix() const
{
    return makeChassisMatrix_glm(m_chassisYaw, m_chassisPitch, m_chassisRoll, m_chassisPos);
}

///@brief Allocate all OpenGL resources
///@note Requires active GL context.
void AppSkeleton::initGL()
{
    m_fm.Init();
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

    // sensible initial value?
    allocateFBO(m_renderBuffer, 800, 600);
}

///@brief Destroy all OpenGL resources
void AppSkeleton::exitGL()
{
    deallocateFBO(m_renderBuffer);
}

///@brief Sometimes the OVR SDK modifies OpenGL state.
void AppSkeleton::_resetGLState() const
{
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.f, 1.f);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void AppSkeleton::_DrawScenes(
    const float* pMvWorld,
    const float* pPersp,
    const ovrRecti& rvp,
    const float* pMvLocal) const
{
    // Clip off top and bottom letterboxes
    glEnable(GL_SCISSOR_TEST);
    const float factor = m_cinemaScopeFactor;
    const int yoff = static_cast<int>(static_cast<float>(rvp.Size.h) * factor);
    // Assume side-by-side single render texture
    glScissor(0, yoff / 2, rvp.Pos.x + rvp.Size.w, rvp.Size.h - yoff);

    // Special case for the ShaderToyScene: if it is on, make it the only one.
    // This is because shadertoys typically don't write to the depth buffer.
    // If one did, it would take more time and complexity, but could be integrated
    // with rasterized world pixels.
    if (m_galleryScene.GetActiveShaderToy() != NULL)
    {
        m_galleryScene.RenderForOneEye(pMvWorld, pPersp);

        // Show the warning box if we get too close to edge of tracking cam's fov.
        glDisable(GL_DEPTH_TEST);
        m_ovrScene.RenderForOneEye(pMvLocal, pPersp); // m_bChassisLocalSpace
        m_dashScene.RenderForOneEye(pMvLocal, pPersp);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        for (std::vector<IScene*>::const_iterator it = m_scenes.begin();
            it != m_scenes.end();
            ++it)
        {
            const IScene* pScene = *it;
            if (pScene != NULL)
            {
                const float* pMv = pScene->m_bChassisLocalSpace ? pMvLocal : pMvWorld;
                pScene->RenderForOneEye(pMv, pPersp);
            }
        }
    }

    glDisable(GL_SCISSOR_TEST);
}

void AppSkeleton::DoSceneRenderPrePasses() const
{
    for (std::vector<IScene*>::const_iterator it = m_scenes.begin();
        it != m_scenes.end();
        ++it)
    {
        const IScene* pScene = *it;
        if (pScene != NULL)
        {
            pScene->RenderPrePass();
        }
    }
    _RenderRaymarchSceneToCamBuffer();
}

///@brief Render the raymarch scene to the camera FBO in DashboardScene's CamPane.
void AppSkeleton::_RenderRaymarchSceneToCamBuffer() const
{
    ///@note Not truly const as we are rendering to it - don't tell the compiler!
    const CamPane& camPane = m_dashScene.m_camPane;
    const FBO& fbo = camPane.m_paneRenderBuffer;
    bindFBO(fbo);
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_DEPTH_TEST);
        const glm::mat4 persp = glm::perspective(
            70.f,
            static_cast<float>(fbo.w) / static_cast<float>(fbo.h),
            .004f,
            500.f);
        const float* pMtx = m_fm.mtxR; ///@todo Iron out discrepancies in L/R Hydra controllers

        glm::mat4 camMtx = glm::make_mat4(pMtx);
        const float s = m_headSize; // scale translation by headSize
        camMtx[3].x *= s;
        camMtx[3].y *= s;
        camMtx[3].z *= s;

        const glm::mat4 mvmtx = makeWorldToChassisMatrix() * camMtx;
        const float* pMv = glm::value_ptr(glm::inverse(mvmtx));
        const float* pPersp = glm::value_ptr(persp);
        m_galleryScene.RenderForOneEye(pMv, pPersp);
    }
    unbindFBO();
}

void AppSkeleton::_drawSceneMono() const
{
    _resetGLState();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 mvLocal = glm::mat4(1.f);
    const glm::mat4 mvWorld = mvLocal *
        glm::inverse(makeWorldToChassisMatrix());

    const int w = static_cast<int>(m_fboScale * static_cast<float>(m_renderBuffer.w));
    const int h = static_cast<int>(m_fboScale * static_cast<float>(m_renderBuffer.h));
    const glm::mat4 persp = glm::perspective(
        90.f,
        static_cast<float>(w)/static_cast<float>(h),
        .004f,
        500.f);

    const ovrRecti rvp = {0,0,w,h};
    _DrawScenes(
        glm::value_ptr(mvWorld),
        glm::value_ptr(persp),
        rvp,
        glm::value_ptr(mvLocal)
        );
}
// RiftAppSkeleton.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#include <OVR.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "RiftAppSkeleton.h"
#include "ShaderPane.h"
#include "ShaderToy.h"
#include "ShaderFunctions.h"
#include "DirectoryFunctions.h"
#include "StringFunctions.h"
#include "TextureFunctions.h"
#include "GLUtils.h"

RiftAppSkeleton::RiftAppSkeleton()
: m_Hmd(NULL)
, m_usingDebugHmd(false)
, m_directHmdMode(true)
, m_hmdRo(0.0f)
, m_hmdRd(0.0f)

, m_scene()
#ifdef USE_SIXENSE
, m_hydraScene()
#endif
, m_ovrScene()
, m_raymarchScene()
, m_shaderToyScene()
, m_paneScene()
, m_scenes()

, m_fboScale(1.0f)
, m_presentFbo()
, m_presentDistMeshL()
, m_presentDistMeshR()
, m_chassisYaw(0.0f)
, m_hyif()
, m_shaderToys()
, m_texLibrary()
, m_headSize(1.0f)
, m_fm()
, m_keyboardMove(0.0f)
, m_joystickMove(0.0f)
, m_mouseMove(0.0f)
, m_keyboardYaw(0.0f)
, m_joystickYaw(0.0f)
, m_mouseDeltaYaw(0.0f)
, m_cinemaScopeFactor(0.0f)
, m_fboMinScale(0.05f)
#ifdef USE_ANTTWEAKBAR
, m_pTweakbar(NULL)
#endif
{
    m_eyeOri = OVR::Quatf();

    // Add as many scenes here as you like. They will share color and depth buffers,
    // so drawing one after the other should just result in pixel-perfect integration -
    // provided they all do forward rendering. Per-scene deferred render passes will
    // take a little bit more work.
    m_scenes.push_back(&m_scene);
#ifdef USE_SIXENSE
    m_scenes.push_back(&m_hydraScene);
#endif
    m_scenes.push_back(&m_ovrScene);
    //m_scenes.push_back(&m_raymarchScene);
    m_scenes.push_back(&m_shaderToyScene);
    m_scenes.push_back(&m_paneScene);

    m_raymarchScene.SetFlyingMousePointer(&m_fm);
    m_paneScene.SetFlyingMousePointer(&m_fm);
    m_paneScene.SetHmdPositionPointer(&m_hmdRo);
    m_paneScene.SetHmdDirectionPointer(&m_hmdRd);

    // Give this scene a pointer to get live Hydra data for display
#ifdef USE_SIXENSE
    m_hydraScene.SetFlyingMousePointer(&m_fm);
    m_hyif.AddTransformation(m_raymarchScene.GetTransformationPointer());
#endif

    ResetAllTransformations();
}

RiftAppSkeleton::~RiftAppSkeleton()
{
    m_fm.Destroy();

    for (std::map<std::string, textureChannel>::iterator it = m_texLibrary.begin();
        it != m_texLibrary.end();
        ++it)
    {
        textureChannel& tc = it->second;
        glDeleteTextures(1, &tc.texID);
    }
}

void RiftAppSkeleton::RecenterPose()
{
    if (m_Hmd == NULL)
        return;
    ovrHmd_RecenterPose(m_Hmd);
}

void RiftAppSkeleton::ResetAllTransformations()
{
    m_chassisPos.x = 0.0f;
    m_chassisPos.y = 1.27f; // my sitting height
    m_chassisPos.z = 1.0f;
    m_chassisYaw = 0.0f;

    m_raymarchScene.ResetTransformation();
}

ovrSizei RiftAppSkeleton::getHmdResolution() const
{
    if (m_Hmd == NULL)
    {
        ovrSizei empty = {0, 0};
        return empty;
    }
    return m_Hmd->Resolution;
}

ovrVector2i RiftAppSkeleton::getHmdWindowPos() const
{
    if (m_Hmd == NULL)
    {
        ovrVector2i empty = {0, 0};
        return empty;
    }
    return m_Hmd->WindowsPos;
}

void RiftAppSkeleton::initGL()
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

    m_presentFbo.initProgram("presentfbo");
    _initPresentFbo();
    m_presentDistMeshL.initProgram("presentmesh");
    m_presentDistMeshR.initProgram("presentmesh");
    // Init the present mesh VAO *after* initVR, which creates the mesh

    // sensible initial value?
    allocateFBO(m_renderBuffer, 800, 600);
    m_fm.Init();
}


void RiftAppSkeleton::_initPresentFbo()
{
    m_presentFbo.bindVAO();

    const float verts[] = {
        -1, -1,
        1, -1,
        1, 1,
        -1, 1
    };
    const float texs[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_presentFbo.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_presentFbo.GetAttrLoc("vPosition"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint texVbo = 0;
    glGenBuffers(1, &texVbo);
    m_presentFbo.AddVbo("vTex", texVbo);
    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_presentFbo.GetAttrLoc("vTex"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_presentFbo.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_presentFbo.GetAttrLoc("vTex"));

    glUseProgram(m_presentFbo.prog());
    {
        OVR::Matrix4f id = OVR::Matrix4f::Identity();
        glUniformMatrix4fv(m_presentFbo.GetUniLoc("mvmtx"), 1, false, &id.Transposed().M[0][0]);
        glUniformMatrix4fv(m_presentFbo.GetUniLoc("prmtx"), 1, false, &id.Transposed().M[0][0]);
    }
    glUseProgram(0);

    glBindVertexArray(0);
}


///@brief Set this up early so we can get the HMD's display dimensions to create a window.
void RiftAppSkeleton::initHMD()
{
    ovr_Initialize();

    m_Hmd = ovrHmd_Create(0);
    if (m_Hmd == NULL)
    {
        m_Hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
        m_usingDebugHmd = true;
        m_directHmdMode = false;
    }

    ///@todo Why does ovrHmd_GetEnabledCaps always return 0 when querying the caps
    /// through the field in ovrHmd appears to work correctly?
    //const unsigned int caps = ovrHmd_GetEnabledCaps(m_Hmd);
    const unsigned int caps = m_Hmd->HmdCaps;
    if ((caps & ovrHmdCap_ExtendDesktop) != 0)
    {
        m_directHmdMode = false;
    }

    m_ovrScene.SetHmdPointer(m_Hmd);
    m_ovrScene.SetChassisPosPointer(&m_chassisPos);
    m_ovrScene.SetChassisYawPointer(&m_chassisYaw);

    // Both ovrVector3f and glm::vec3 are at heart a float[3], so this works fine.
    m_fm.SetChassisPosPointer(reinterpret_cast<glm::vec3*>(&m_chassisPos));
    m_fm.SetChassisYawPointer(&m_chassisYaw);
}

void RiftAppSkeleton::initVR()
{
    if (m_Hmd != NULL)
    {
        const ovrBool ret = ovrHmd_ConfigureTracking(m_Hmd,
            ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position,
            ovrTrackingCap_Orientation);
        if (ret == 0)
        {
            std::cerr << "Error calling ovrHmd_ConfigureTracking." << std::endl;
        }
    }

    // The RTSize fields are used by all rendering paths
    ovrSizei l_ClientSize;
    l_ClientSize = getHmdResolution();
    m_Cfg.OGL.Header.RTSize.w = l_ClientSize.w;
    m_Cfg.OGL.Header.RTSize.h = l_ClientSize.h;

    ///@todo Do we need to choose here?
    ConfigureSDKRendering();
    ConfigureClientRendering();

    _initPresentDistMesh(m_presentDistMeshL, 0);
    _initPresentDistMesh(m_presentDistMeshR, 1);

    if (UsingDebugHmd() == false)
    {
        m_windowSize.x = m_EyeTexture[0].OGL.Header.RenderViewport.Size.w;
        m_windowSize.y = m_EyeTexture[0].OGL.Header.RenderViewport.Size.h;
    }
}

void RiftAppSkeleton::_initPresentDistMesh(ShaderWithVariables& shader, int eyeIdx)
{
    // Init left and right VAOs separately
    shader.bindVAO();

    ovrDistortionMesh& mesh = m_DistMeshes[eyeIdx];
    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    shader.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.VertexCount * sizeof(ovrDistortionVertex), &mesh.pVertexData[0].ScreenPosNDC.x, GL_STATIC_DRAW);

    const int a_pos = shader.GetAttrLoc("vPosition");
    glVertexAttribPointer(a_pos, 4, GL_FLOAT, GL_FALSE, sizeof(ovrDistortionVertex), NULL);
    glEnableVertexAttribArray(a_pos);

    const int a_texR = shader.GetAttrLoc("vTexR");
    if (a_texR > -1)
    {
        glVertexAttribPointer(a_texR, 2, GL_FLOAT, GL_FALSE, sizeof(ovrDistortionVertex),
            reinterpret_cast<void*>(offsetof(ovrDistortionVertex, TanEyeAnglesR)));
        glEnableVertexAttribArray(a_texR);
    }

    const int a_texG = shader.GetAttrLoc("vTexG");
    if (a_texG > -1)
    {
        glVertexAttribPointer(a_texG, 2, GL_FLOAT, GL_FALSE, sizeof(ovrDistortionVertex),
            reinterpret_cast<void*>(offsetof(ovrDistortionVertex, TanEyeAnglesG)));
        glEnableVertexAttribArray(a_texG);
    }

    const int a_texB = shader.GetAttrLoc("vTexB");
    if (a_texB > -1)
    {
        glVertexAttribPointer(a_texB, 2, GL_FLOAT, GL_FALSE, sizeof(ovrDistortionVertex),
            reinterpret_cast<void*>(offsetof(ovrDistortionVertex, TanEyeAnglesB)));
        glEnableVertexAttribArray(a_texB);
    }

    GLuint elementVbo = 0;
    glGenBuffers(1, &elementVbo);
    shader.AddVbo("elements", elementVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexCount * sizeof(GLushort), &mesh.pIndexData[0], GL_STATIC_DRAW);

    // We have copies of the mesh in GL, but keep a count of indices around for the GL draw call.
    const unsigned int tmp = mesh.IndexCount;
    ovrHmd_DestroyDistortionMesh(&mesh);
    mesh.IndexCount = tmp;

    glBindVertexArray(0);
}

void RiftAppSkeleton::exitVR()
{
    deallocateFBO(m_renderBuffer);
    ovrHmd_Destroy(m_Hmd);
    ovr_Shutdown();
}

// Active GL context is required for the following
int RiftAppSkeleton::ConfigureSDKRendering()
{
    if (m_Hmd == NULL)
        return 1;
    ovrSizei l_TextureSizeLeft = ovrHmd_GetFovTextureSize(m_Hmd, ovrEye_Left, m_Hmd->DefaultEyeFov[0], 1.0f);
    ovrSizei l_TextureSizeRight = ovrHmd_GetFovTextureSize(m_Hmd, ovrEye_Right, m_Hmd->DefaultEyeFov[1], 1.0f);
    ovrSizei l_TextureSize;
    l_TextureSize.w = l_TextureSizeLeft.w + l_TextureSizeRight.w;
    l_TextureSize.h = (l_TextureSizeLeft.h>l_TextureSizeRight.h ? l_TextureSizeLeft.h : l_TextureSizeRight.h);

    // Oculus Rift eye configurations...
    m_EyeFov[0] = m_Hmd->DefaultEyeFov[0];
    m_EyeFov[1] = m_Hmd->DefaultEyeFov[1];

    m_Cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    m_Cfg.OGL.Header.Multisample = 0;

    const int distortionCaps =
        ovrDistortionCap_Chromatic |
        ovrDistortionCap_TimeWarp |
        ovrDistortionCap_Vignette;
    ovrHmd_ConfigureRendering(m_Hmd, &m_Cfg.Config, distortionCaps, m_EyeFov, m_EyeRenderDesc);

    m_EyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
    m_EyeTexture[0].OGL.Header.TextureSize.w = l_TextureSize.w;
    m_EyeTexture[0].OGL.Header.TextureSize.h = l_TextureSize.h;
    m_EyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
    m_EyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
    m_EyeTexture[0].OGL.Header.RenderViewport.Size.w = l_TextureSize.w/2;
    m_EyeTexture[0].OGL.Header.RenderViewport.Size.h = l_TextureSize.h;
    m_EyeTexture[0].OGL.TexId = m_renderBuffer.tex;

    // Right eye the same, except for the x-position in the texture...
    m_EyeTexture[1] = m_EyeTexture[0];
    m_EyeTexture[1].OGL.Header.RenderViewport.Pos.x = (l_TextureSize.w+1) / 2;

    return 0;
}

int RiftAppSkeleton::ConfigureClientRendering()
{
    if (m_Hmd == NULL)
        return 1;

    ovrSizei l_TextureSizeLeft = ovrHmd_GetFovTextureSize(m_Hmd, ovrEye_Left, m_Hmd->DefaultEyeFov[0], 1.0f);
    ovrSizei l_TextureSizeRight = ovrHmd_GetFovTextureSize(m_Hmd, ovrEye_Right, m_Hmd->DefaultEyeFov[1], 1.0f);
    ovrSizei l_TextureSize;
    l_TextureSize.w = l_TextureSizeLeft.w + l_TextureSizeRight.w;
    l_TextureSize.h = std::max(l_TextureSizeLeft.h, l_TextureSizeRight.h);

    // Renderbuffer init - we can use smaller subsets of it easily
    deallocateFBO(m_renderBuffer);
    allocateFBO(m_renderBuffer, l_TextureSize.w, l_TextureSize.h);


    m_EyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
    m_EyeTexture[0].OGL.Header.TextureSize.w = l_TextureSize.w;
    m_EyeTexture[0].OGL.Header.TextureSize.h = l_TextureSize.h;
    m_EyeTexture[0].OGL.Header.RenderViewport.Pos.x = 0;
    m_EyeTexture[0].OGL.Header.RenderViewport.Pos.y = 0;
    m_EyeTexture[0].OGL.Header.RenderViewport.Size.w = l_TextureSize.w/2;
    m_EyeTexture[0].OGL.Header.RenderViewport.Size.h = l_TextureSize.h;
    m_EyeTexture[0].OGL.TexId = m_renderBuffer.tex;

    // Right eye the same, except for the x-position in the texture...
    m_EyeTexture[1] = m_EyeTexture[0];
    m_EyeTexture[1].OGL.Header.RenderViewport.Pos.x = (l_TextureSize.w+1) / 2;

    m_RenderViewports[0] = m_EyeTexture[0].OGL.Header.RenderViewport;
    m_RenderViewports[1] = m_EyeTexture[1].OGL.Header.RenderViewport;

    const int distortionCaps =
        ovrDistortionCap_Chromatic |
        ovrDistortionCap_TimeWarp |
        ovrDistortionCap_Vignette;

    // Generate distortion mesh for each eye
    for (int eyeNum = 0; eyeNum < 2; eyeNum++)
    {
        // Allocate & generate distortion mesh vertices.
        ovrHmd_CreateDistortionMesh(
            m_Hmd,
            m_EyeRenderDesc[eyeNum].Eye,
            m_EyeRenderDesc[eyeNum].Fov,
            distortionCaps,
            &m_DistMeshes[eyeNum]);
    }
    return 0;
}

///@brief The HSW will be displayed by default when using SDK rendering.
void RiftAppSkeleton::DismissHealthAndSafetyWarning() const
{
    ovrHSWDisplayState hswDisplayState;
    ovrHmd_GetHSWDisplayState(m_Hmd, &hswDisplayState);
    if (hswDisplayState.Displayed)
    {
        ovrHmd_DismissHSWDisplay(m_Hmd);
    }
}

///@brief This function will detect a moderate tap on the Rift via the accelerometer.
///@return true if a tap was detected, false otherwise.
bool RiftAppSkeleton::CheckForTapOnHmd()
{
    const ovrTrackingState ts = ovrHmd_GetTrackingState(m_Hmd, ovr_GetTimeInSeconds());
    if (!(ts.StatusFlags & ovrStatus_OrientationTracked))
        return false;

    const OVR::Vector3f v(ts.RawSensorData.Accelerometer);
    // Arbitrary value and representing moderate tap on the side of the DK2 Rift.
    if (v.LengthSq() > 250.f)
    {
        // Limit tapping rate
        static double lastTapTime = 0.0;
        if (ovr_GetTimeInSeconds() - lastTapTime > 0.5)
        {
            lastTapTime = ovr_GetTimeInSeconds();
            DismissHealthAndSafetyWarning();
            ToggleShaderWorld();
            return true;
        }
    }
    return false;
}

void RiftAppSkeleton::_resetGLState() const
{
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glDepthFunc(GL_LESS);

    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void RiftAppSkeleton::resize(int w, int h)
{
    (void)w;
    (void)h;
    m_windowSize.x = w;
    m_windowSize.y = h;
}

void RiftAppSkeleton::SetFBOScale(float s)
{
    m_fboScale = s;
    m_fboScale = std::max(m_fboMinScale, m_fboScale);
    m_fboScale = std::min(1.0f, m_fboScale);
}

void RiftAppSkeleton::timestep(float dt)
{
    for (std::vector<IScene*>::iterator it = m_scenes.begin();
        it != m_scenes.end();
        ++it)
    {
        IScene* pScene = *it;
        if (pScene != NULL)
        {
            pScene->timestep(dt);
        }
    }

    glm::vec3 hydraMove = glm::vec3(0.0f, 0.0f, 0.0f);
#ifdef USE_SIXENSE
    const sixenseAllControllerData& state = m_fm.GetCurrentState();
    for (int i = 0; i<2; ++i)
    {
        const sixenseControllerData& cd = state.controllers[i];
        const float moveScale = pow(10.0f, cd.trigger);
        hydraMove.x += cd.joystick_x * moveScale;

        const FlyingMouse::Hand h = static_cast<FlyingMouse::Hand>(i);
        if (m_fm.IsPressed(h, SIXENSE_BUTTON_JOYSTICK)) ///@note left hand does not work
            hydraMove.y += cd.joystick_y * moveScale;
        else
            hydraMove.z -= cd.joystick_y * moveScale;
    }

    if (m_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_START))
    {
        ToggleShaderWorld();
    }

    // Adjust cinemascope feel with left trigger
    // Mouse wheel will still work if Hydra is not present or not pressed(0.0 trigger value).
    const float trigger = m_fm.GetTriggerValue(FlyingMouse::Left); // [0,1]
    if (trigger > 0.0f)
    {
        const float deadzone = 0.1f;
        const float topval = 0.95f;
        const float trigScaled = (trigger - deadzone) / (1.0f - deadzone);
        m_cinemaScopeFactor = std::max(0.0f, topval * trigScaled);
    }
#endif

    const glm::vec3 move_dt = m_headSize * (m_keyboardMove + m_joystickMove + m_mouseMove + hydraMove) * dt;
    ovrVector3f kbm;
    kbm.x = move_dt.x;
    kbm.y = move_dt.y;
    kbm.z = move_dt.z;

    // Move in the direction the viewer is facing.
    const OVR::Matrix4f rotmtx = 
          OVR::Matrix4f::RotationY(-m_chassisYaw)
        * OVR::Matrix4f(m_eyeOri);
    const OVR::Vector3f kbmVec = rotmtx.Transform(OVR::Vector3f(kbm));

    m_chassisPos.x += kbmVec.x;
    m_chassisPos.y += kbmVec.y;
    m_chassisPos.z += kbmVec.z;

    m_chassisYaw += (m_keyboardYaw + m_joystickYaw + m_mouseDeltaYaw) * dt;

    m_fm.updateHydraData();
    m_hyif.updateHydraData(m_fm, 1.0f);
}

/// Scale the parallax translation and head pose motion vector by the head size
/// dictated by the shader. Thanks to the elegant design decision of putting the
/// head's default position at the origin, this is simple.
OVR::Matrix4f _MakeModelviewMatrix(
    ovrPosef eyePose,
    ovrVector3f viewAdjust,
    float chassisYaw,
    ovrVector3f chassisPos,
    float headScale=1.0f)
{
    const OVR::Matrix4f eyePoseMatrix =
        OVR::Matrix4f::Translation(OVR::Vector3f(eyePose.Position) * headScale)
        * OVR::Matrix4f(OVR::Quatf(eyePose.Orientation));

    const OVR::Matrix4f view =
        eyePoseMatrix.Inverted()
        * OVR::Matrix4f::RotationY(chassisYaw)
        * OVR::Matrix4f::Translation(-OVR::Vector3f(chassisPos));

    return view;
}

void RiftAppSkeleton::_DrawScenes(
    const float* pMview,
    const float* pPersp,
    const ovrRecti& rvp,
    const float* pScaledMview) const
{
    // Clip off top and bottom letterboxes
    glEnable(GL_SCISSOR_TEST);
    const float factor = m_cinemaScopeFactor;
    const int yoff = static_cast<int>(static_cast<float>(rvp.Size.h) * factor);
    // Assume side-by-side single render texture
    glScissor(0, yoff/2, rvp.Pos.x+rvp.Size.w, rvp.Size.h-yoff);

    // Special case for the ShaderToyScene: if it is on, make it the only one.
    // This is because shadertoys typically don't write to the depth buffer.
    // If one did, it would take more time and complexity, but could be integrated
    // with rasterized world pixels.
    if (m_shaderToyScene.m_bDraw)
    {
        m_shaderToyScene.RenderForOneEye(pScaledMview ? pScaledMview : pMview, pPersp);

        // Show the warning box if we get too close to edge of tracking cam's fov.
        glDisable(GL_DEPTH_TEST);
        m_ovrScene.RenderForOneEye(pMview, pPersp);
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
                pScene->RenderForOneEye(pMview, pPersp);
            }
        }
    }

    glDisable(GL_SCISSOR_TEST);
}


void RiftAppSkeleton::DiscoverShaders(bool recurse)
{
    const std::vector<std::string> shadernames = recurse ?
        GetListOfFilesFromDirectoryAndSubdirs(ShaderToy::s_shaderDir) :
        GetListOfFilesFromDirectory(ShaderToy::s_shaderDir);
    for (std::vector<std::string>::const_iterator it = shadernames.begin();
        it != shadernames.end();
        ++it)
    {
        const std::string& s = *it;

        ShaderToy* pSt = new ShaderToy(s);
        if (pSt == NULL)
            continue;

        m_shaderToys.push_back(pSt);

        Pane* pP = m_paneScene.AddShaderPane(pSt);
        pP->initGL();
    }
}

void RiftAppSkeleton::CompileShaders()
{
    std::vector<Pane*>& panes = m_paneScene.m_panes;
    for (std::vector<Pane*>::iterator it = panes.begin();
        it != panes.end();
        ++it)
    {
        ShaderPane* pP = reinterpret_cast<ShaderPane*>(*it);
        if (pP == NULL)
            continue;
        ShaderToy* pSt = pP->m_pShadertoy;

        Timer t;
        pSt->CompileShader();

        std::cout
            << "\t\t "
            << t.seconds()
            << "s"
            ;
    }
}

void RiftAppSkeleton::RenderThumbnails()
{
    std::vector<Pane*>& panes = m_paneScene.m_panes;
    for (std::vector<Pane*>::iterator it = panes.begin();
        it != panes.end();
        ++it)
    {
        ShaderPane* pP = reinterpret_cast<ShaderPane*>(*it);
        if (pP == NULL)
            continue;
        ShaderToy* pSt = pP->m_pShadertoy;

        // Render a view of the shader to the FBO
        // We must keep the previously bound FBO and restore
        GLint bound_fbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);
        bindFBO(pP->m_paneRenderBuffer);

        //pP->DrawToFBO();
        {
            const glm::vec3 hp = pSt->GetHeadPos();
            const glm::vec3 LookVec(0.0f, 0.0f, -1.0f);
            const glm::vec3 up(0.0f, 1.0f, 0.0f);

            ovrPosef eyePose;
            eyePose.Orientation = OVR::Quatf();
            eyePose.Position = OVR::Vector3f();
            const OVR::Matrix4f view = _MakeModelviewMatrix(
                eyePose,
                OVR::Vector3f(0.0f),
                static_cast<float>(M_PI),
                OVR::Vector3f(hp.x, hp.y, hp.z));

            const glm::mat4 persp = glm::perspective(
                90.0f,
                static_cast<float>(pP->m_paneRenderBuffer.w) / static_cast<float>(pP->m_paneRenderBuffer.h),
                0.004f,
                500.0f);

            const bool wasDrawing = m_shaderToyScene.m_bDraw;
            m_shaderToyScene.m_bDraw = true;
            m_shaderToyScene.SetShaderToy(pSt);
            m_shaderToyScene.RenderForOneEye(&view.Transposed().M[0][0], glm::value_ptr(persp));
            m_shaderToyScene.m_bDraw = wasDrawing;
            m_shaderToyScene.SetShaderToy(NULL);
        }

        const int lineh = 62;
        const int margin = 22;
        int txh = pP->m_paneRenderBuffer.h - 3*lineh - margin;
        const ShaderWithVariables& fsh = m_paneScene.GetFontShader();
        const BMFont& fnt = m_paneScene.GetFont();
        // Twisting together 3 strands of ownership: ShaderPane's function
        // taking ShaderToy's data and ShaderGalleryScene's font and shader.
        const std::string title = pSt->GetStringByName("title");
        pP->DrawTextOverlay(title.empty() ? pSt->GetSourceFile() : title, margin, txh, fsh, fnt);
        pP->DrawTextOverlay(pSt->GetStringByName("author"), margin, txh += lineh, fsh, fnt);
        pP->DrawTextOverlay(pSt->GetStringByName("license"), margin, txh += lineh, fsh, fnt);

        unbindFBO();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, bound_fbo);
    }
}

void RiftAppSkeleton::LoadTexturesFromFile()
{
    Timer t;
    const std::string texdir("../textures/");
    const std::vector<std::string> texturenames = GetListOfFilesFromDirectory(texdir);
    for (std::vector<std::string>::const_iterator it = texturenames.begin();
        it != texturenames.end();
        ++it)
    {
        const std::string& s = *it;
        const std::string fullName = texdir + s;

        GLuint texId = 0;
        GLuint width = 0;
        GLuint height = 0;
        ///@todo Case insensitivity?
        if (hasEnding(fullName, ".jpg"))
            texId = LoadTextureFromJpg(fullName.c_str(), &width, &height);
        else if (hasEnding(fullName, ".png"))
            texId = LoadTextureFromPng(fullName.c_str(), &width, &height);

        textureChannel tc = {texId, width, height};
        m_texLibrary[s] = tc;
    }
    std::cout << "Textures loaded in " << t.seconds() << " seconds." << std::endl;

    m_shaderToyScene.SetTextureLibraryPointer(&m_texLibrary);
}

#ifdef USE_ANTTWEAKBAR
static void TW_CALL GoToURLCB(void *clientData)
{
    const RiftAppSkeleton* pApp = reinterpret_cast<RiftAppSkeleton *>(clientData);
    if (!pApp)
        return;
    ShaderToy* pST = pApp->m_shaderToyScene.GetShaderToy();
    if (pST == NULL)
        return;

    const std::string url = pST->GetStringByName("url");
    if (url.empty())
        return;

#ifdef _WIN32
    ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif _LINUX
    std::string command = "x-www-browser ";
    command += url;
    system(command.c_str());
#elif __APPLE__
    std::string command = "open ";
    command += url;
    system(command.c_str());
#endif
}
#endif

void RiftAppSkeleton::ToggleShaderWorld()
{
    ShaderToy* pST = m_paneScene.GetFocusedShader();

    if (m_shaderToyScene.m_bDraw)
    {
        // Back into gallery
        ResetAllTransformations();
        m_headSize = 1.0f;
        m_shaderToyScene.m_bDraw = false;
        m_shaderToyScene.SetShaderToy(NULL);

#ifdef USE_ANTTWEAKBAR
        TwRemoveVar(m_pTweakbar, "title");
        TwRemoveVar(m_pTweakbar, "author");
        TwRemoveVar(m_pTweakbar, "gotourl");
#endif
    }
    else if (pST != NULL)
    {
        // Transitioning into shader world
        ///@todo Will we drop frames here? Clear to black if so.
        m_shaderToyScene.m_bDraw = true;
        m_shaderToyScene.SetShaderToy(pST);

        const glm::vec3 hp = pST->GetHeadPos();
        m_chassisPos.x = hp.x;
        m_chassisPos.y = hp.y;
        m_chassisPos.z = hp.z;
        m_headSize = pST->GetHeadSize();
        m_chassisYaw = static_cast<float>(M_PI);

#ifdef USE_ANTTWEAKBAR
        const std::string titleStr = "title: " + pST->GetSourceFile();
        const std::string authorStr = "author: " + pST->GetStringByName("author");

        std::stringstream ss;
        // Assemble a string to pass into help here
        ss << " label='"
           << titleStr
           << "' group=Shader ";
        TwAddButton(m_pTweakbar, "title", NULL, NULL, ss.str().c_str());

        ss.str("");
        ss << " label='"
           << authorStr
           << "' group=Shader ";
        TwAddButton(m_pTweakbar, "author", NULL, NULL, ss.str().c_str());

        TwAddButton(m_pTweakbar, "gotourl", GoToURLCB, this, " label='Go to URL'  group='Shader' ");
#endif
    }
}

// Store HMD position and direction for gaze tracking in timestep.
// OVR SDK requires head pose be queried between ovrHmd_BeginFrameTiming and ovrHmd_EndFrameTiming.
// Don't worry - we're just writing to _mutable_ members, it's still const!
void RiftAppSkeleton::_StoreHmdPose(const ovrPosef& eyePose) const
{
    m_hmdRo.x = eyePose.Position.x + m_chassisPos.x;
    m_hmdRo.y = eyePose.Position.y + m_chassisPos.y;
    m_hmdRo.z = eyePose.Position.z + m_chassisPos.z;

    const OVR::Matrix4f rotmtx = OVR::Matrix4f::RotationY(-m_chassisYaw) // Not sure why negative...
        * OVR::Matrix4f(eyePose.Orientation);
    const OVR::Vector4f rotvec = rotmtx.Transform(OVR::Vector4f(0.0f, 0.0f, -1.0f, 0.0f));
    m_hmdRd.x = rotvec.x;
    m_hmdRd.y = rotvec.y;
    m_hmdRd.z = rotvec.z;
}

void RiftAppSkeleton::_drawSceneMono() const
{
    _resetGLState();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::vec3 EyePos(m_chassisPos.x, m_chassisPos.y, m_chassisPos.z);
    const glm::vec3 LookVec(0.0f, 0.0f, -1.0f);
    const glm::vec3 up(0.0f, 1.0f, 0.0f);

    const ovrPosef eyePose = {
        OVR::Quatf(),
        OVR::Vector3f()
    };
    _StoreHmdPose(eyePose);
    const OVR::Matrix4f view = _MakeModelviewMatrix(
        eyePose,
        OVR::Vector3f(0.0f),
        m_chassisYaw,
        m_chassisPos);

    const int w = m_windowSize.x;
    const int h = m_windowSize.y;
    const glm::mat4 persp = glm::perspective(
        90.0f,
        static_cast<float>(w)/static_cast<float>(h),
        0.004f,
        500.0f);

    ovrRecti rvp = {0,0,w,h};
    _DrawScenes(&view.Transposed().M[0][0], glm::value_ptr(persp), rvp);
}

void RiftAppSkeleton::display_raw() const
{
    const int w = m_windowSize.x;
    const int h = m_windowSize.y;
    glViewport(0, 0, w, h);

    _drawSceneMono();
}

void RiftAppSkeleton::display_buffered(bool setViewport) const
{
    bindFBO(m_renderBuffer, m_fboScale);
    _drawSceneMono();
    unbindFBO();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (setViewport)
    {
        const int w = m_Cfg.OGL.Header.RTSize.w;
        const int h = m_Cfg.OGL.Header.RTSize.h;
        glViewport(0, 0, w, h);
    }

    // Present FBO to screen
    const GLuint prog = m_presentFbo.prog();
    glUseProgram(prog);
    m_presentFbo.bindVAO();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_renderBuffer.tex);
        glUniform1i(m_presentFbo.GetUniLoc("fboTex"), 0);

        // This is the only uniform that changes per-frame
        glUniform1f(m_presentFbo.GetUniLoc("fboScale"), m_fboScale);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

///@todo Even though this function shares most of its code with client rendering,
/// which appears to work fine, it is non-convergable. It appears that the projection
/// matrices for each eye are too far apart? Could be modelview...
void RiftAppSkeleton::display_stereo_undistorted() const
{
    ovrHmd hmd = m_Hmd;
    if (hmd == NULL)
        return;

    //ovrFrameTiming hmdFrameTiming =
    ovrHmd_BeginFrameTiming(hmd, 0);

    bindFBO(m_renderBuffer, m_fboScale);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
    {
        ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];
        ovrPosef eyePose = ovrHmd_GetHmdPosePerEye(hmd, eye);

        const ovrGLTexture& otex = m_EyeTexture[eye];
        const ovrRecti& rvp = otex.OGL.Header.RenderViewport;
        const ovrRecti rsc = {
            static_cast<int>(m_fboScale * rvp.Pos.x),
            static_cast<int>(m_fboScale * rvp.Pos.y),
            static_cast<int>(m_fboScale * rvp.Size.w),
            static_cast<int>(m_fboScale * rvp.Size.h)
        };
        glViewport(rsc.Pos.x, rsc.Pos.y, rsc.Size.w, rsc.Size.h);

        OVR::Quatf orientation = OVR::Quatf(eyePose.Orientation);
        OVR::Matrix4f proj = ovrMatrix4f_Projection(
            m_EyeRenderDesc[eye].Fov,
            0.01f, 10000.0f, true);

        //m_EyeRenderDesc[eye].DistortedViewport;
        OVR::Vector3f EyePos = m_chassisPos;
        OVR::Matrix4f view = OVR::Matrix4f(orientation.Inverted())
            * OVR::Matrix4f::RotationY(m_chassisYaw)
            * OVR::Matrix4f::Translation(-EyePos);
        OVR::Matrix4f eyeview = OVR::Matrix4f::Translation(m_EyeRenderDesc[eye].HmdToEyeViewOffset) * view;

        _resetGLState();

        _DrawScenes(&eyeview.Transposed().M[0][0], &proj.Transposed().M[0][0], rvp);
    }
    unbindFBO();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Present FBO to screen
    const GLuint prog = m_presentFbo.prog();
    glUseProgram(prog);
    m_presentFbo.bindVAO();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_renderBuffer.tex);
        glUniform1i(m_presentFbo.GetUniLoc("fboTex"), 0);

        // This is the only uniform that changes per-frame
        glUniform1f(m_presentFbo.GetUniLoc("fboScale"), m_fboScale);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
    glUseProgram(0);

    ovrHmd_EndFrameTiming(hmd);
}

void RiftAppSkeleton::display_sdk() const
{
    ovrHmd hmd = m_Hmd;
    if (hmd == NULL)
        return;

    //const ovrFrameTiming hmdFrameTiming =
    ovrHmd_BeginFrame(m_Hmd, 0);

    bindFBO(m_renderBuffer);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ovrVector3f e2v[2] = {
        OVR::Vector3f(m_EyeRenderDesc[0].HmdToEyeViewOffset) * m_headSize,
        OVR::Vector3f(m_EyeRenderDesc[1].HmdToEyeViewOffset) * m_headSize,
    };

    ovrTrackingState outHmdTrackingState;
    ovrPosef outEyePoses[2];
    ovrHmd_GetEyePoses(
        hmd,
        0,
        e2v, // could this parameter be const?
        outEyePoses,
        &outHmdTrackingState);

    // For passing to EndFrame once rendering is done
    ovrPosef renderPose[2];
    ovrTexture eyeTexture[2];
    for (int eyeIndex=0; eyeIndex<ovrEye_Count; eyeIndex++)
    {
        const ovrEyeType e = hmd->EyeRenderOrder[eyeIndex];

        const ovrPosef eyePose = outEyePoses[e];
        renderPose[e] = eyePose;
        eyeTexture[e] = m_EyeTexture[e].Texture;
        m_eyeOri = eyePose.Orientation; // cache this for movement direction
        _StoreHmdPose(eyePose);

        const ovrGLTexture& otex = m_EyeTexture[e];
        const ovrRecti& rvp = otex.OGL.Header.RenderViewport;
        glViewport(
            static_cast<int>(m_fboScale * rvp.Pos.x),
            static_cast<int>(m_fboScale * rvp.Pos.y),
            static_cast<int>(m_fboScale * rvp.Size.w),
            static_cast<int>(m_fboScale * rvp.Size.h)
            );

        const OVR::Matrix4f proj = ovrMatrix4f_Projection(
            m_EyeRenderDesc[e].Fov,
            0.01f, 10000.0f, true);

        const OVR::Matrix4f view = _MakeModelviewMatrix(
            eyePose,
            OVR::Vector3f(0.0f),
            m_chassisYaw,
            m_chassisPos);

        const OVR::Matrix4f scaledView = _MakeModelviewMatrix(
            eyePose,
            OVR::Vector3f(0.0f),
            m_chassisYaw,
            m_chassisPos,
            m_headSize);

        _resetGLState();

        _DrawScenes(&view.Transposed().M[0][0], &proj.Transposed().M[0][0], rvp, &scaledView.Transposed().M[0][0]);
    }
    unbindFBO();

    // Inform SDK of downscaled texture target size(performance scaling)
    for (int i=0; i<ovrEye_Count; ++i)
    {
        const ovrSizei& ts = m_EyeTexture[i].Texture.Header.TextureSize;
        ovrRecti& rr = eyeTexture[i].Header.RenderViewport;
        rr.Size.w = static_cast<int>(static_cast<float>(ts.w/2) * m_fboScale);
        rr.Size.h = static_cast<int>(static_cast<float>(ts.h) * m_fboScale);
        rr.Pos.x = i * rr.Size.w;
    }
    ovrHmd_EndFrame(m_Hmd, renderPose, eyeTexture);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void RiftAppSkeleton::display_client() const
{
    const ovrHmd hmd = m_Hmd;
    if (hmd == NULL)
        return;

    //ovrFrameTiming hmdFrameTiming =
    ovrHmd_BeginFrameTiming(hmd, 0);

    bindFBO(m_renderBuffer, m_fboScale);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ovrVector3f e2v[2] = {
        OVR::Vector3f(m_EyeRenderDesc[0].HmdToEyeViewOffset) * m_headSize,
        OVR::Vector3f(m_EyeRenderDesc[1].HmdToEyeViewOffset) * m_headSize,
    };

    ovrTrackingState outHmdTrackingState;
    ovrPosef outEyePoses[2];
    ovrHmd_GetEyePoses(
        hmd,
        0,
        e2v,
        outEyePoses,
        &outHmdTrackingState);

    for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
    {
        const ovrEyeType e = hmd->EyeRenderOrder[eyeIndex];

        const ovrPosef eyePose = outEyePoses[e];
        m_eyeOri = eyePose.Orientation; // cache this for movement direction
        _StoreHmdPose(eyePose);

        const ovrGLTexture& otex = m_EyeTexture[e];
        const ovrRecti& rvp = otex.OGL.Header.RenderViewport;
        const ovrRecti rsc = {
            static_cast<int>(m_fboScale * rvp.Pos.x),
            static_cast<int>(m_fboScale * rvp.Pos.y),
            static_cast<int>(m_fboScale * rvp.Size.w),
            static_cast<int>(m_fboScale * rvp.Size.h)
        };
        glViewport(rsc.Pos.x, rsc.Pos.y, rsc.Size.w, rsc.Size.h);

        const OVR::Matrix4f proj = ovrMatrix4f_Projection(
            m_EyeRenderDesc[e].Fov,
            0.01f, 10000.0f, true);

        ///@todo Should we be using this variable?
        //m_EyeRenderDesc[eye].DistortedViewport;

        const OVR::Matrix4f view = _MakeModelviewMatrix(
            eyePose,
            m_EyeRenderDesc[e].HmdToEyeViewOffset,
            m_chassisYaw,
            m_chassisPos);

        const OVR::Matrix4f scaledView = _MakeModelviewMatrix(
            eyePose,
            m_EyeRenderDesc[e].HmdToEyeViewOffset,
            m_chassisYaw,
            m_chassisPos,
            m_headSize);

        _resetGLState();

        _DrawScenes(&view.Transposed().M[0][0], &proj.Transposed().M[0][0], rsc, &scaledView.Transposed().M[0][0]);
    }
    unbindFBO();


    // Set full viewport...?
    const int w = m_Cfg.OGL.Header.RTSize.w;
    const int h = m_Cfg.OGL.Header.RTSize.h;
    glViewport(0, 0, w, h);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Now draw the distortion mesh...
    for(int eyeNum = 0; eyeNum < 2; eyeNum++)
    {
        const ShaderWithVariables& eyeShader = eyeNum == 0 ?
            m_presentDistMeshL :
            m_presentDistMeshR;
        const GLuint prog = eyeShader.prog();
        glUseProgram(prog);
        eyeShader.bindVAO();
        {
            const ovrDistortionMesh& mesh = m_DistMeshes[eyeNum];

            ovrVector2f uvScaleOffsetOut[2];
            ovrHmd_GetRenderScaleAndOffset(
                m_EyeFov[eyeNum],
                m_EyeTexture[eyeNum].Texture.Header.TextureSize,
                m_EyeTexture[eyeNum].OGL.Header.RenderViewport,
                uvScaleOffsetOut );

            const ovrVector2f uvscale = uvScaleOffsetOut[0];
            const ovrVector2f uvoff = uvScaleOffsetOut[1];

            glUniform2f(eyeShader.GetUniLoc("EyeToSourceUVOffset"), uvoff.x, uvoff.y);
            glUniform2f(eyeShader.GetUniLoc("EyeToSourceUVScale"), uvscale.x, uvscale.y);


#if 0
            // Setup shader constants
            DistortionData.Shaders->SetUniform2f(
                "EyeToSourceUVScale",
                DistortionData.UVScaleOffset[eyeNum][0].x,
                DistortionData.UVScaleOffset[eyeNum][0].y);
            DistortionData.Shaders->SetUniform2f(
                "EyeToSourceUVOffset",
                DistortionData.UVScaleOffset[eyeNum][1].x,
                DistortionData.UVScaleOffset[eyeNum][1].y);

            if (distortionCaps & ovrDistortionCap_TimeWarp)
            { // TIMEWARP - Additional shader constants required
                ovrMatrix4f timeWarpMatrices[2];
                ovrHmd_GetEyeTimewarpMatrices(HMD, (ovrEyeType)eyeNum, eyeRenderPoses[eyeNum], timeWarpMatrices);
                //WARNING!!! These matrices are transposed in SetUniform4x4f, before being used by the shader.
                DistortionData.Shaders->SetUniform4x4f("EyeRotationStart", Matrix4f(timeWarpMatrices[0]));
                DistortionData.Shaders->SetUniform4x4f("EyeRotationEnd", Matrix4f(timeWarpMatrices[1]));
            }

            // Perform distortion
            pRender->Render(
                &distortionShaderFill,
                DistortionData.MeshVBs[eyeNum],
                DistortionData.MeshIBs[eyeNum]);
#endif

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_renderBuffer.tex);
            glUniform1i(eyeShader.GetUniLoc("fboTex"), 0);

            // This is the only uniform that changes per-frame
            glUniform1f(eyeShader.GetUniLoc("fboScale"), m_fboScale);


            glDrawElements(
                GL_TRIANGLES,
                mesh.IndexCount,
                GL_UNSIGNED_SHORT,
                0);
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }

    ovrHmd_EndFrameTiming(hmd);
}

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
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AppSkeleton.h"
#include "MatrixFunctions.h"
#include "DirectoryFunctions.h"
#include "ShaderToyPane.h"
#include "Logger.h"

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
, m_presentFbo()
, m_presentDistMeshL()
, m_presentDistMeshR()
, m_chassisYaw(0.f)
, m_hyif()
, m_transitionTimer()
, m_transitionState(0)
, m_texLibrary()
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
, m_fboMinScale(.05f)
#ifdef USE_ANTTWEAKBAR
, m_pTweakbar(NULL)
, m_pShaderTweakbar(NULL)
#endif
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
    m_presentFbo.initProgram("presentfbo");
    _initPresentFbo();
    m_presentDistMeshL.initProgram("presentmesh");
    m_presentDistMeshR.initProgram("presentmesh");
    // Init the present mesh VAO *after* initVR, which creates the mesh
}

///@brief Destroy all OpenGL resources
void AppSkeleton::exitGL()
{
    deallocateFBO(m_renderBuffer);

    for (std::map<std::string, textureChannel>::iterator it = m_texLibrary.begin();
        it != m_texLibrary.end();
        ++it)
    {
        textureChannel& tc = it->second;
        glDeleteTextures(1, &tc.texID);
    }
}

void AppSkeleton::_initPresentFbo()
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

void AppSkeleton::display_raw() const
{
    const int w = m_windowSize.x;
    const int h = m_windowSize.y;
    glViewport(0, 0, w, h);

    _drawSceneMono();
}

void AppSkeleton::resize(int w, int h)
{
    (void)w;
    (void)h;
    m_windowSize.x = w;
    m_windowSize.y = h;
}

void AppSkeleton::DiscoverShaders(bool recurse)
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

        Pane* pP = m_galleryScene.AddShaderToyPane(pSt);
        pP->initGL();
    }
}

///@note One of these days the texture library will break down into a singleton.
void AppSkeleton::SetTextureLibraryPointer()
{
    m_galleryScene.SetTextureLibraryPointer(&m_texLibrary);
}

void AppSkeleton::LoadTextureLibrary()
{
    Timer t;
    std::map<std::string, textureChannel>& texLib = m_texLibrary;
    const std::string texdir("../textures/");
    LoadShaderToyTexturesFromDirectory(texLib, texdir);
    std::cout << "Textures loaded in " << t.seconds() << " seconds." << std::endl;
}

#ifdef USE_ANTTWEAKBAR
static void TW_CALL GoToURLCB(void *clientData)
{
    const AppSkeleton* pApp = reinterpret_cast<AppSkeleton *>(clientData);
    if (!pApp)
        return;

    const ShaderToyPane* pP = pApp->m_galleryScene.GetFocusedPane();
    if (pP == NULL)
        return;

    ShaderToy* pST = pP->m_pShadertoy;
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

static void TW_CALL ResetShaderVariablesCB(void *clientData)
{
    ShaderToy* pST = (ShaderToy*)clientData;
    if (pST == NULL)
        return;
    pST->ResetVariables();
}
#endif

///@brief Initiate the change, timestep will call _ToggleShaderWorld after a small delay.
void AppSkeleton::ToggleShaderWorld()
{
    m_transitionState = 1;
    m_transitionTimer.reset();
}

void AppSkeleton::_ToggleShaderWorld()
{
    if (m_galleryScene.GetActiveShaderToy() != NULL)
    {
        // Back into gallery
        LOG_INFO("Back to gallery");
        ResetChassisTransformations();
        m_chassisPos = m_chassisPosCached;
        m_chassisYaw = m_chassisYawCached;
        m_headSize = 1.0f;
        m_galleryScene.SetActiveShaderToy(NULL);
        m_galleryScene.SetActiveShaderToyPane(NULL);

#ifdef USE_ANTTWEAKBAR
        m_dashScene.SendMouseClick(0); // Leaving a drag in progress can cause a crash!
        TwRemoveVar(m_pTweakbar, "title");
        TwRemoveVar(m_pTweakbar, "author");
        TwRemoveVar(m_pTweakbar, "gotourl");
        TwRemoveAllVars(m_pShaderTweakbar);
#endif
        return;
    }

    ShaderToyPane* pP = const_cast<ShaderToyPane*>(m_galleryScene.GetFocusedPane());
    if (pP == NULL)
        return;

    ShaderToy* pST = pP->m_pShadertoy;
    if (pST == NULL)
        return;

    // Transitioning into shader world
    ///@todo Will we drop frames here? Clear to black if so.
    LOG_INFO("Transition to shadertoy: %s", pST->GetSourceFile().c_str());
    m_galleryScene.SetActiveShaderToy(pST);
    m_galleryScene.SetActiveShaderToyPane(pP);

    // Return to the gallery in the same place we left it
    m_chassisPosCached = m_chassisPos;
    m_chassisYawCached = m_chassisYaw;

    m_chassisPos = pST->GetHeadPos();
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

    TwAddButton(m_pShaderTweakbar, "Reset Variables", ResetShaderVariablesCB, pST, " label='Reset Variables' ");

    // for each var type, add vec3 direction control
    ///@todo Different type widths
    std::map<std::string, shaderVariable>& tweakVars = pST->m_tweakVars;
    for (std::map<std::string, shaderVariable>::iterator it = tweakVars.begin();
        it != tweakVars.end();
        ++it)
    {
        const std::string& name = it->first;
        const shaderVariable& var = it->second;

        std::ostringstream oss;
        oss << " group='Shader' ";

        ETwType t = TW_TYPE_FLOAT;
        if (var.width == 1)
        {
            // Assemble min/max/incr param string for ant
            oss
                << "min="
                << var.minVal.x
                << " max="
                << var.maxVal.x
                << " step="
                << var.incr
                << " ";
        }
        else if (var.width == 3)
        {
            t = TW_TYPE_DIR3F;
            if (var.varType == shaderVariable::Direction)
            {
                t = TW_TYPE_DIR3F;
            }
            else if (var.varType == shaderVariable::Color)
            {
                t = TW_TYPE_COLOR3F;
            }
            ///@todo handle free, non-normalized values
            else
            {
            }
        }
        const glm::vec4& tv = var.value;
        const std::string vn = name;
        TwAddVarRW(m_pShaderTweakbar, vn.c_str(), t, (void*)glm::value_ptr(tv), oss.str().c_str());
    }
#endif
}

void AppSkeleton::SaveShaderSettings()
{
    const ShaderToy* pST = m_galleryScene.GetActiveShaderToy();
    if (pST == NULL)
        return;
    pST->SaveSettings();
}

void AppSkeleton::timestep(double absTime, double dtd)
{
    const float dt = static_cast<float>(dtd);
    for (std::vector<IScene*>::iterator it = m_scenes.begin();
        it != m_scenes.end();
        ++it)
    {
        IScene* pScene = *it;
        if (pScene != NULL)
        {
            pScene->timestep(absTime, dt);
        }
    }

    glm::vec3 hydraMove = glm::vec3(0.0f, 0.0f, 0.0f);
#ifdef USE_SIXENSE
    const sixenseAllControllerData& state = m_fm.GetCurrentState();
    //for (int i = 0; i<2; ++i)
    {
        const int i = 0;
        const sixenseControllerData& cd = state.controllers[i];
        const float moveScale = pow(10.0f, cd.trigger);
        hydraMove.x += cd.joystick_x * moveScale;

        const FlyingMouse::Hand h = static_cast<FlyingMouse::Hand>(i);
        if (m_fm.IsPressed(h, SIXENSE_BUTTON_JOYSTICK)) ///@note left hand does not work
            hydraMove.y += cd.joystick_y * moveScale;
        else
            hydraMove.z -= cd.joystick_y * moveScale;
    }

    // Check all Hydra buttons for HSW dismissal
    if ((m_fm.WasJustPressed(FlyingMouse::Left, 0xFF)) ||
        (m_fm.WasJustPressed(FlyingMouse::Right, 0xFF)))
    {
        DismissHealthAndSafetyWarning();
    }

    if (m_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_START))
    {
        ToggleShaderWorld();
    }
    if (m_fm.WasJustPressed(FlyingMouse::Left, SIXENSE_BUTTON_START))
    {
        ToggleShaderWorld();
    }
    if (m_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_BUMPER))
    {
        m_dashScene.m_bDraw = !m_dashScene.m_bDraw;
    }
    if (m_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_2))
    {
        m_dashScene.SetHoldingFlag(1);
    }
    if (m_fm.WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_2))
    {
        m_dashScene.SetHoldingFlag(0);
    }

    ///@todo Extract function here - duplicated code in glfw_main's joystick function
    ///@todo Hand ids may switch if re-ordered on base
    const sixenseControllerData& cd = state.controllers[1];
    const float x = cd.joystick_x;
    const float y = cd.joystick_y;
    const float deadZone = 0.1f;
    if (fabs(y) > deadZone)
    {
        // Absolute "farthest push"
        //const float resMin = m_fboMinScale;
        const float resMax = 1.f;
        const float d = (-y - deadZone)/(1.f - deadZone); // [0,1]
        const float u = ( y - deadZone)/(1.f - deadZone);
        // Push up on stick to increase resolution, down to decrease
        const float s = GetFBOScale();
        if (d > 0.f)
        {
            SetFBOScale(std::min(s, 1.f - d));
        }
        else if (u > 0.f)
        {
            SetFBOScale(std::max(s, u * resMax));
        }
    }
    if (fabs(x) > deadZone)
    {
        //const float cinMin = 0.f;
        const float cinMax = .95f;
        const float l = (-x - deadZone)/(1.f - deadZone);
        const float r = ( x - deadZone)/(1.f - deadZone);
        // Push left on stick to close cinemascope, right to open
        if (l > 0.f)
        {
            m_cinemaScopeFactor = std::max(
                m_cinemaScopeFactor,
                l * cinMax);
        }
        else if (r > 0.f)
        {
            m_cinemaScopeFactor = std::min(
                m_cinemaScopeFactor,
                1.f - r);
        }
    }

#if 0
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

#endif

    const glm::vec3 move_dt = m_headSize * (m_keyboardMove + m_joystickMove + m_mouseMove + hydraMove) * dt;
    ovrVector3f kbm;
    kbm.x = move_dt.x;
    kbm.y = move_dt.y;
    kbm.z = move_dt.z;

    // Move in the direction the viewer is facing.
    const glm::vec4 mv4 = makeWorldToEyeMatrix() * glm::vec4(move_dt, 0.0f);

    m_chassisPos += glm::vec3(mv4);
    m_chassisYaw += (m_keyboardYaw + m_joystickYaw + m_mouseDeltaYaw) * dt;
    m_chassisPitch += m_keyboardDeltaPitch * dt;
    m_chassisRoll += m_keyboardDeltaRoll * dt;

    m_fm.updateHydraData();
    m_hyif.updateHydraData(m_fm, 1.0f);
    m_galleryScene.SetChassisTransformation(makeWorldToChassisMatrix());

    // Manage transition animations
    {
        const float duration = 0.15f;
        const float t = static_cast<float>(m_transitionTimer.seconds()) / duration;
        if (t >= 1.0f)
        {
            if (m_transitionState == 1)
            {
                _ToggleShaderWorld();
                m_transitionState = 2;
            }
        }
        if (t < 2.0f)
        {
            // eye blink transition
            const float fac = std::max(1.0f-t, t-1.0f);
            m_cinemaScopeFactor = 1.0f - fac;
        }
        else
        {
            m_transitionState = 0;
        }
    }
}

void AppSkeleton::SetFBOScale(float s)
{
    m_fboScale = s;
    m_fboScale = std::max(m_fboMinScale, m_fboScale);
    m_fboScale = std::min(1.0f, m_fboScale);
}

// glfw_main.cpp
// With humongous thanks to cThrough 2014 (Daniel Dekkers)

#include <GL/glew.h>

#if defined(_WIN32)
#  include <Windows.h>
#  define GLFW_EXPOSE_NATIVE_WIN32
#  define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__linux__)
#  include <X11/X.h>
#  include <X11/extensions/Xrandr.h>
#  define GLFW_EXPOSE_NATIVE_X11
#  define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3.h>

#if !defined(__APPLE__)
#  include <GLFW/glfw3native.h>
#endif

#include <glm/gtc/type_ptr.hpp>

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>

#ifdef USE_OCULUSSDK
#include "RiftAppSkeleton.h"
#else
#include "AppSkeleton.h"
#endif
#include "RenderingMode.h"
#include "Timer.h"
#include "FPSTimer.h"
#include "Logger.h"
#include "StringFunctions.h"
#include "version.h"

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#include <sys/stat.h>
#endif

#ifdef USE_OCULUSSDK
RiftAppSkeleton g_app;
#else
AppSkeleton g_app;
#endif
RenderingMode g_renderMode;
Timer g_timer;
double g_lastFrameTime = 0.0;
FPSTimer g_fps;
Timer g_logDumpTimer;

bool g_receivedFirstTap = false;
int m_keyStates[GLFW_KEY_LAST];

// mouse motion internal state
int oldx, oldy, newx, newy;
int which_button = -1;
int modifier_mode = 0;

ShaderWithVariables g_auxPresent;
GLFWwindow* g_pHMDWindow = NULL;
GLFWwindow* g_AuxWindow = NULL;
int g_auxWindow_w = 600;
int g_auxWindow_h = 600;

int g_joystickIdx = -1;

float g_fpsSmoothingFactor = 0.02f;
float g_fpsDeltaThreshold = 5.0f;
bool g_dynamicallyScaleFBO = true;
int g_targetFPS = 70;
bool g_drawToAuxWindow = false;
bool g_loadShadertoysRecursive = true;
bool g_allowPitch = false;
bool g_allowRoll = false;

#ifdef USE_ANTTWEAKBAR
TwBar* g_pTweakbar = NULL;
TwBar* g_pShaderTweakbar = NULL;
#endif

GLFWwindow* initializeAuxiliaryWindow(GLFWwindow* pRiftWindow);
void destroyAuxiliaryWindow(GLFWwindow* pAuxWindow);
void FindPreferredJoystick();

// Set VSync is framework-dependent and has to come before the include
///@param state 0=off, 1=on, -1=adaptive
// Set vsync for both contexts.
static void SetVsync(int state)
{
    LOG_INFO("SetVsync(%d)", state);

    // Since AuxWindow holds the tweakbar, this should never be NULL
    if (g_AuxWindow != NULL)
    {
        glfwMakeContextCurrent(g_AuxWindow);
        glfwSwapInterval(state);
    }
    glfwMakeContextCurrent(g_pHMDWindow);
    glfwSwapInterval(state);
}

#include "main_include.cpp"

static void ErrorCallback(int p_Error, const char* p_Description)
{
    (void)p_Error;
    (void)p_Description;
    LOG_INFO("ERROR: %d, %s", p_Error, p_Description);
}


void keyboard(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
    (void)pWindow;
    (void)codes;

    if ((key > -1) && (key <= GLFW_KEY_LAST))
    {
        m_keyStates[key] = action;
        //printf("key ac  %d %d\n", key, action);
    }

    const float f = 0.9f;
    const float ff = 0.99f;
    ShaderToyGlobalState& gs = g_app.m_galleryScene.m_globalShadertoyState;

    switch (key)
    {
        default: break;
        case GLFW_KEY_BACKSLASH:
            {
                if      (action==GLFW_PRESS  ) g_app.m_dashScene.SendMouseClick(1);
                else if (action==GLFW_RELEASE) g_app.m_dashScene.SendMouseClick(0);
            }
            break;
        case GLFW_KEY_SLASH:
            {
                if      (action==GLFW_PRESS  ) g_app.m_dashScene.SetHoldingFlag(1);
                else if (action==GLFW_RELEASE) g_app.m_dashScene.SetHoldingFlag(0);
            }
            break;
    }

    if (action == GLFW_PRESS)
    {
    g_app.DismissHealthAndSafetyWarning();
    switch (key)
    {
        default: break;

        case GLFW_KEY_F1:
            if (mods & GLFW_MOD_CONTROL)
            {
                g_renderMode.toggleRenderingTypeReverse();
            }
            else
            {
                g_renderMode.toggleRenderingType();
            }
            LOG_INFO("Render Type: %d", g_renderMode.outputType);
            break;

        case GLFW_KEY_F2:
            g_renderMode.toggleRenderingTypeMono();
            LOG_INFO("Render Type: %d", g_renderMode.outputType);
            break;

        case GLFW_KEY_F3:
            g_renderMode.toggleRenderingTypeHMD();
            LOG_INFO("Render Type: %d", g_renderMode.outputType);
            break;

        case GLFW_KEY_F4:
            g_renderMode.toggleRenderingTypeDistortion();
            LOG_INFO("Render Type: %d", g_renderMode.outputType);
            break;

        case GLFW_KEY_F5: g_dynamicallyScaleFBO = false; g_app.SetFBOScale(f * g_app.GetFBOScale()); break;
        case GLFW_KEY_F6: g_dynamicallyScaleFBO = false; g_app.SetFBOScale(ff * g_app.GetFBOScale()); break;
        case GLFW_KEY_F7: g_dynamicallyScaleFBO = false; g_app.SetFBOScale((1.f/ff) * g_app.GetFBOScale()); break;
        case GLFW_KEY_F8: g_dynamicallyScaleFBO = false; g_app.SetFBOScale((1.f/f) * g_app.GetFBOScale()); break;

        case GLFW_KEY_F9: SetVsync(0); break;
        case GLFW_KEY_F10: SetVsync(1); break;
        case GLFW_KEY_F11: SetVsync(-1); break;

        case GLFW_KEY_DELETE: g_dynamicallyScaleFBO = !g_dynamicallyScaleFBO; break;

        case GLFW_KEY_SEMICOLON: gs.animatedThumbnails = !gs.animatedThumbnails; break;
        case GLFW_KEY_APOSTROPHE: gs.panesAsPortals = !gs.panesAsPortals; break;

        case '`':
            ///@todo Is there a way to create an auxiliary window in Direct to rift mode?
            /// The call to glfwCreateWindow crashes the app in Win7.

#ifdef USE_OCULUSSDK
            if (g_app.UsingDirectMode() == false)
#endif
            {
                if (g_AuxWindow == NULL)
                {
                    g_AuxWindow = initializeAuxiliaryWindow(g_pHMDWindow);

#ifdef USE_ANTTWEAKBAR
                    ///@note This will break PaneScene's tweakbar positioning
                    TwWindowSize(g_auxWindow_w, g_auxWindow_h);
#endif
                }
                else
                {
                    destroyAuxiliaryWindow(g_AuxWindow);
                    glfwMakeContextCurrent(g_pHMDWindow);
                }
            }
            break;

        case GLFW_KEY_TAB:
            if (g_app.UsingDebugHmd() == false)
            {
                g_app.m_dashScene.m_bDraw = !g_app.m_dashScene.m_bDraw;
                if (g_app.m_dashScene.m_bDraw)
                {
                    g_app.m_dashScene.ResizeTweakbar();
                }
            }
            break;

        case GLFW_KEY_SPACE:
            g_app.RecenterPose();
            FindPreferredJoystick();
            break;

        case 'J':
            FindPreferredJoystick();
            break;

        case 'R':
            g_app.ResetChassisTransformations();
            break;

        case GLFW_KEY_PAGE_DOWN:
            g_app.m_cinemaScopeFactor += 0.1f;
            g_app.m_cinemaScopeFactor = std::min(0.95f, g_app.m_cinemaScopeFactor);
            break;

        case GLFW_KEY_PAGE_UP:
            g_app.m_cinemaScopeFactor -= 0.1f;
            g_app.m_cinemaScopeFactor = std::max(0.f, g_app.m_cinemaScopeFactor);
            break;

        case GLFW_KEY_ENTER:
            g_app.ToggleShaderWorld();
            break;

        case GLFW_KEY_PRINT_SCREEN:
            g_app.SaveShaderSettings();
            break;

        case GLFW_KEY_ESCAPE:
            if (g_AuxWindow == NULL)
            {
                // Clear the frame before calling all the destructors - even a few
                // frames worth of frozen video is enough to cause discomfort!
                ///@note This does not seem to work in Direct mode.
                glClearColor(58.f/255.f, 110.f/255.f, 165.f/255.f, 1.f); // Win7 default desktop color
                glClear(GL_COLOR_BUFFER_BIT);
                glfwSwapBuffers(g_pHMDWindow);
                glClear(GL_COLOR_BUFFER_BIT);
                glfwSwapBuffers(g_pHMDWindow);

                g_app.exitVR();
                glfwDestroyWindow(g_pHMDWindow);
                glfwTerminate();
                exit(0);
            }
            else
            {
                destroyAuxiliaryWindow(g_AuxWindow);
                glfwMakeContextCurrent(g_pHMDWindow);
            }
            break;
        }
    }

    //g_app.keyboard(key, action, 0,0);

    const glm::vec3 forward(0.f, 0.f, -1.f);
    const glm::vec3 up(0.f, 1.f, 0.f);
    const glm::vec3 right(1.f, 0.f, 0.f);
    // Handle keyboard movement(WASD keys)
    glm::vec3 keyboardMove(0.0f, 0.0f, 0.0f);
    if (m_keyStates['W'] != GLFW_RELEASE) { keyboardMove += forward; }
    if (m_keyStates['S'] != GLFW_RELEASE) { keyboardMove -= forward; }
    if (m_keyStates['A'] != GLFW_RELEASE) { keyboardMove -= right; }
    if (m_keyStates['D'] != GLFW_RELEASE) { keyboardMove += right; }
    if (m_keyStates['Q'] != GLFW_RELEASE) { keyboardMove -= up; }
    if (m_keyStates['E'] != GLFW_RELEASE) { keyboardMove += up; }
    if (m_keyStates[GLFW_KEY_UP] != GLFW_RELEASE) { keyboardMove += forward; }
    if (m_keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) { keyboardMove -= forward; }
    if (m_keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE) { keyboardMove -= right; }
    if (m_keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE) { keyboardMove += right; }

    float mag = 1.0f;
    if (m_keyStates[GLFW_KEY_LEFT_SHIFT] != GLFW_RELEASE) mag *= 0.1f;
    if (m_keyStates[GLFW_KEY_LEFT_CONTROL] != GLFW_RELEASE) mag *= 10.0f;
    if (m_keyStates[GLFW_KEY_RIGHT_SHIFT] != GLFW_RELEASE) mag *= 0.1f;
    if (m_keyStates[GLFW_KEY_RIGHT_CONTROL] != GLFW_RELEASE) mag *= 10.0f;

    // Yaw keys
    g_app.m_keyboardYaw = 0.0f;
    const float dyaw = 0.5f * mag; // radians at 60Hz timestep
    if (m_keyStates['1'] != GLFW_RELEASE) { g_app.m_keyboardYaw = -dyaw; }
    if (m_keyStates['3'] != GLFW_RELEASE) { g_app.m_keyboardYaw = dyaw; }

    // Pitch and roll controls - if yaw is VR poison,
    // this is torture and death!
    g_app.m_keyboardDeltaPitch = 0.0f;
    g_app.m_keyboardDeltaRoll = 0.0f;
    if (g_allowPitch)
    {
        if (m_keyStates['2'] != GLFW_RELEASE) { g_app.m_keyboardDeltaPitch = -dyaw; }
        if (m_keyStates['X'] != GLFW_RELEASE) { g_app.m_keyboardDeltaPitch = dyaw; }
    }
    if (g_allowRoll)
    {
        if (m_keyStates['Z'] != GLFW_RELEASE) { g_app.m_keyboardDeltaRoll = -dyaw; }
        if (m_keyStates['C'] != GLFW_RELEASE) { g_app.m_keyboardDeltaRoll = dyaw; }
    }

    g_app.m_keyboardMove = mag * keyboardMove;
}

void joystick_GravisGamepadPro(
    int, // joyidx
    const float*, // pAxisStates
    int numAxes,
    const unsigned char* pButtonStates,
    int numButtons,
    const char* pLastButtonStates)
{
    // Map joystick buttons to move directions
    // Gravis Gamepad Pro layout in glfw:
    // 0 Red (left position)
    // 1 Yellow (down position)
    // 2 Green (right position)
    // 3 Blue (up position)
    // 4 Left top shoulder
    // 5 Right top shoulder
    // 6 Left bottom shoulder
    // 7 Right bottom shoulder
    // 8 Select (left center)
    // 9 Start (right center)
    const glm::vec3 moveDirsGravisGamepadPro[8] = {
        glm::vec3(-1.f,  0.f,  0.f),
        glm::vec3( 0.f,  0.f,  1.f),
        glm::vec3( 1.f,  0.f,  0.f),
        glm::vec3( 0.f,  0.f, -1.f),
        glm::vec3( 0.f,  0.f,  0.f),
        glm::vec3( 0.f,  0.f,  0.f),
        glm::vec3( 0.f,  0.f,  0.f),
        glm::vec3( 0.f,  0.f,  0.f),
    };
    glm::vec3 joystickMove(0.0f, 0.0f, 0.0f);
    for (int i=0; i<std::min(8,numButtons); ++i)
    {
        if (pButtonStates[i] == GLFW_PRESS)
        {
            joystickMove += moveDirsGravisGamepadPro[i];
        }
    }
    
    float mag = 1.f;
    if (numAxes > 2)
    {
    }
    g_app.m_joystickMove = mag * joystickMove;

    const int buttonAdjustVfov = 8;
    const int buttonCloseVfov = 4;
    const int buttonOpenVfov = 6;
    if (pButtonStates[buttonAdjustVfov] == GLFW_PRESS)
    {
        const float incr = 0.05f;
        float scope = g_app.m_cinemaScopeFactor;
        if (
            (pButtonStates[buttonCloseVfov] == GLFW_PRESS) &&
            (pLastButtonStates[buttonCloseVfov] != GLFW_PRESS)
            )
        {
            scope += incr;
        }
        else if (
            (pButtonStates[buttonOpenVfov] == GLFW_PRESS) &&
            (pLastButtonStates[buttonOpenVfov] != GLFW_PRESS)
            )
        {
            scope -= incr;
        }
        scope = std::max(0.0f, scope);
        scope = std::min(0.95f, scope);
        g_app.m_cinemaScopeFactor = scope;
    }
}

void joystick_XboxController(
    int, // joyidx
    const float* pAxisStates,
    int numAxes,
    const unsigned char* pButtonStates,
    int numButtons,
    const char* pLastButtonStates)
{
    //ASSERT(numAxes == 5);
    //ASSERT(numButtons == 14);
    if (numAxes != 5)
        return;
    if (numButtons != 14)
        return;

    // Xbox controller layout in glfw:
    // numAxes 5, numButtons 14
    // 0 A (down position)
    // 1 B (right position)
    // 2 X (left position)
    // 3 Y (up position)
    // 4 L bumper
    // 5 R bumper
    // 6 Back (left center)
    // 7 Start (right center)
    // 8 Left stick push
    // 9 Right stick push
    // 10 Dpad Up
    // 11 Dpad right
    // 12 Dpad down
    // 13 Dpad left
    // Axis 0 1 Left stick x y
    // Axis 2 triggers, left positive right negative
    // Axis 3 4 right stick y x

    glm::vec3 joystickMove(0.0f, 0.0f, 0.0f);
    // Xbox controller Left stick controls movement
    if (numAxes >= 2)
    {
        const float x_move = pAxisStates[0];
        const float y_move = pAxisStates[1];
        const glm::vec3 forward(0.f, 0.f, -1.f);
        const glm::vec3 right(1.f, 0.f, 0.f);
        const float deadzone = 0.5f;
        if (fabs(x_move) > deadzone)
            joystickMove += x_move * right;
        if (fabs(y_move) > deadzone)
            joystickMove -= y_move * forward;
    }

    if (pButtonStates[0] == GLFW_PRESS) // A button
        joystickMove += glm::vec3( 0.f,  1.f,  0.f);
    if (pButtonStates[1] == GLFW_PRESS) // B button
        joystickMove += glm::vec3( 0.f, -1.f,  0.f);

    float mag = 1.f;
    if (numAxes > 2)
    {
        // Xbox left and right analog triggers control speed
        mag = pow(10.f, pAxisStates[2]);
    }
    g_app.m_joystickMove = mag * joystickMove;

    // Right stick controls yaw
    ///@todo Pitch, Roll(instant nausea!)
    if (numAxes > 3)
    {
        float x_move = pAxisStates[4];
        const glm::vec3 up(0.f, 1.f, 0.f);
        const float deadzone = 0.2f;
        if (fabs(x_move) < deadzone)
            x_move = 0.f;
        g_app.m_joystickYaw = 0.75f * static_cast<float>(x_move);
    }

    // Check for recent button pushes
    const float f = 0.9f;
    for (int i=0; i<numButtons; ++i)
    {
        const bool pressed = (pButtonStates[i] == GLFW_PRESS) &&
                             (pLastButtonStates[i] != GLFW_PRESS);
        const bool released = (pButtonStates[i] != GLFW_PRESS) &&
                              (pLastButtonStates[i] == GLFW_PRESS);
        if (pressed)
        {
            g_app.DismissHealthAndSafetyWarning();

            if (i == 13) // Dpad left
            {
                g_dynamicallyScaleFBO = false;
                g_app.SetFBOScale(f * g_app.GetFBOScale());
            }
            if (i == 11) // Dpad right
            {
                g_dynamicallyScaleFBO = false;
                g_app.SetFBOScale((1.f/f) * g_app.GetFBOScale());
            }
            if (i == 10) // Dpad up
            {
                g_app.m_cinemaScopeFactor -= 0.1f;
                g_app.m_cinemaScopeFactor = std::max(0.f, g_app.m_cinemaScopeFactor);
            }
            if (i == 12) // Dpad down
            {
                g_app.m_cinemaScopeFactor += 0.1f;
                g_app.m_cinemaScopeFactor = std::min(0.95f, g_app.m_cinemaScopeFactor);
            }
            if (i == 4) // Left Bumper
            {
                g_app.RecenterPose();
            }
            if (i == 5) // Right Bumper
            {
                g_app.ResetChassisTransformations();
            }
            if (i == 7) // Start
            {
                g_app.ToggleShaderWorld();
            }
            if (i == 3) // Y button
            {
                g_app.m_dashScene.m_bDraw = !g_app.m_dashScene.m_bDraw;
            }
        }
        if (pressed || released)
        {
            if (i == 2) // X button
            {
                g_app.m_dashScene.SendMouseClick(pressed?1:0);
            }
            if (i == 9) // Right stick push
            {
                g_app.m_dashScene.SetHoldingFlag(pressed?1:0);
            }
        }
    }
}

///@brief Check all available joysticks for an Xbox Controller
/// and store its idx in g_joystickIdx.
/// Unfortunately, this operation is too time-consuming to call every frame
/// in a VR app. The workaround is to call it on key press, space or 'G'.
void FindPreferredJoystick()
{
    g_joystickIdx = -1;
    for (int i=GLFW_JOYSTICK_1; i<=GLFW_JOYSTICK_LAST; ++i)
    {
        if (GL_FALSE == glfwJoystickPresent(i))
            continue;

        const char* pJoyName = glfwGetJoystickName(i);
        if (pJoyName == NULL)
            continue;

        int numAxes = 0;
        int numButtons = 0;
        glfwGetJoystickAxes(i, &numAxes);
        glfwGetJoystickButtons(i, &numButtons);
        LOG_INFO("Glfw found Joystick #%d: %s w/ %d axes, %d buttons", i, pJoyName, numAxes, numButtons);

        // Take an educated guess that this is an Xbox controller - glfw's
        // id string says "Microsoft PC Joystick" for most gamepad types.
        ///@todo Why does GLFW on Linux return a different, more descriptive string?
        if (numAxes == 5 && numButtons == 14)
        {
            g_joystickIdx = i;
            return;
        }
        else if (g_joystickIdx == -1)
        {
            g_joystickIdx = i;
        }
    }
}

void joystick()
{
    static char s_lastButtons[256] = {0};

    ///@todo Handle multiple joysticks
    if (g_joystickIdx == -1)
        return;

    ///@todo Do these calls take time? We can move them out if so
    int joyStick1Present = GL_FALSE;
    joyStick1Present = glfwJoystickPresent(g_joystickIdx);
    if (joyStick1Present != GL_TRUE)
    {
        if (g_joystickIdx == -1)
            return;
    }

    // Poll joystick
    int numAxes = 0;
    const float* pAxisStates = glfwGetJoystickAxes(g_joystickIdx, &numAxes);
    int numButtons = 0;
    const unsigned char* pButtonStates = glfwGetJoystickButtons(g_joystickIdx, &numButtons);

    // Take an educated guess that this is an Xbox controller - glfw's
    // id string says "Microsoft PC Joystick" for most gamepad types.
    ///@todo Why does GLFW on Linux return a different, more descriptive string?
    if (numAxes == 5 && numButtons == 14)
    {
        joystick_XboxController(g_joystickIdx, pAxisStates, numAxes, pButtonStates, numButtons, s_lastButtons);
    }
    else if (numAxes == 2 && numButtons == 10)
    {
        joystick_GravisGamepadPro(g_joystickIdx, pAxisStates, numAxes, pButtonStates, numButtons, s_lastButtons);
    }
    memcpy(s_lastButtons, pButtonStates, numButtons);
}

void mouseDown(GLFWwindow* pWindow, int button, int action, int mods)
{
    (void)mods;

    double xd, yd;
    glfwGetCursorPos(pWindow, &xd, &yd);
    const int x = static_cast<int>(xd);
    const int y = static_cast<int>(yd);

    // Hold right button and press left
    if ((action == GLFW_PRESS) &&
        (button == GLFW_MOUSE_BUTTON_LEFT) &&
        (which_button == GLFW_MOUSE_BUTTON_RIGHT))
    {
        g_app.ToggleShaderWorld();
    }

    which_button = button;
    oldx = newx = x;
    oldy = newy = y;
    if (action == GLFW_RELEASE)
    {
        which_button = -1;
    }

    if (action == GLFW_PRESS)
    {
        g_app.DismissHealthAndSafetyWarning();
    }

    if ((action==GLFW_PRESS)&&(button==GLFW_MOUSE_BUTTON_MIDDLE))
    {
        g_app.m_dashScene.m_bDraw = !g_app.m_dashScene.m_bDraw;
    }

    if      ((button==GLFW_MOUSE_BUTTON_LEFT)&&(action==GLFW_PRESS  )) g_app.m_dashScene.SendMouseClick(1);
    else if ((button==GLFW_MOUSE_BUTTON_LEFT)&&(action==GLFW_RELEASE)) g_app.m_dashScene.SendMouseClick(0);
}

void mouseMove(GLFWwindow* pWindow, double xd, double yd)
{
    glfwGetCursorPos(pWindow, &xd, &yd);
    const int x = static_cast<int>(xd);
    const int y = static_cast<int>(yd);

    oldx = newx;
    oldy = newy;
    newx = x;
    newy = y;
    const int mmx = x-oldx;
    const int mmy = y-oldy;

    g_app.m_mouseDeltaYaw = 0.0f;
    g_app.m_mouseMove = glm::vec3(0.0f);

    if (g_app.UsingDebugHmd() == true)
    {
        // Only allow mouse yaw adjustment when using DebugHMD rendering
        if (which_button == GLFW_MOUSE_BUTTON_1)
        {
            const float spinMagnitude = 0.05f;
            g_app.m_mouseDeltaYaw += static_cast<float>(mmx) * spinMagnitude;
        }
    }

    if (which_button == GLFW_MOUSE_BUTTON_2) // Right click
    {
        const float moveMagnitude = 0.5f;
        g_app.m_mouseMove.x += static_cast<float>(mmx) * moveMagnitude;
        g_app.m_mouseMove.z += static_cast<float>(mmy) * moveMagnitude;
    }
    else if (which_button == GLFW_MOUSE_BUTTON_3) // Middle click
    {
        const float moveMagnitude = 0.5f;
        g_app.m_mouseMove.x += static_cast<float>(mmx) * moveMagnitude;
        g_app.m_mouseMove.y -= static_cast<float>(mmy) * moveMagnitude;
    }

    g_app.m_dashScene.SendMouseMotion(x, y);
}

void mouseWheel(GLFWwindow* pWindow, double x, double y)
{
    (void)pWindow;
    (void)x;

    const float delta = static_cast<float>(y);
    const float incr = 0.05f;

    if (which_button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        float fbosc = g_app.GetFBOScale();
        fbosc += incr * delta;
        fbosc = std::max(.15f, fbosc);
        fbosc = std::min(1.f, fbosc);
        g_app.SetFBOScale(fbosc);
    }
    else
    {
        float cscope = g_app.m_cinemaScopeFactor;
        cscope += incr * delta;
        cscope = std::max(0.0f, cscope);
        cscope = std::min(0.95f, cscope);
        g_app.m_cinemaScopeFactor = cscope;
    }
}

void resize(GLFWwindow* pWindow, int w, int h)
{
    (void)pWindow;
    g_app.resize(w,h);
}

void keyboard_Aux(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventKeyGLFW(key, action);
    if (ant != 0)
        return;
#endif
    keyboard(pWindow, key, codes, action, mods);
}

void mouseDown_Aux(GLFWwindow* pWindow, int button, int action, int mods)
{
    (void)pWindow;
    (void)mods;

#ifdef USE_ANTTWEAKBAR
    int ant = TwEventMouseButtonGLFW(button, action);
    if (ant != 0)
        return;
#endif
    mouseDown(pWindow, button, action, mods);
}

void mouseMove_Aux(GLFWwindow* pWindow, double xd, double yd)
{
    (void)pWindow;

#ifdef USE_ANTTWEAKBAR
    int ant = TwEventMousePosGLFW(static_cast<int>(xd), static_cast<int>(yd));
    if (ant != 0)
        return;
#endif
    mouseMove(pWindow, xd, yd);
}

void mouseWheel_Aux(GLFWwindow* pWindow, double x, double y)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventMouseWheelGLFW(static_cast<int>(x));
    if (ant != 0)
        return;
#endif
    mouseWheel(pWindow, x, y);
}

void resize_Aux(GLFWwindow* pWindow, int w, int h)
{
    (void)pWindow;
    g_auxWindow_w = w;
    g_auxWindow_h = h;

#ifdef USE_ANTTWEAKBAR
    ///@note This will break PaneScene's tweakbar positioning
    TwWindowSize(w, h);
#endif
}

void timestep()
{
    const double absT = g_timer.seconds();
    const double dt = absT - g_lastFrameTime;
    g_lastFrameTime = absT;
    g_app.timestep(absT, dt);
}

void printGLContextInfo(GLFWwindow* pW)
{
    // Print some info about the OpenGL context...
    const int l_Major = glfwGetWindowAttrib(pW, GLFW_CONTEXT_VERSION_MAJOR);
    const int l_Minor = glfwGetWindowAttrib(pW, GLFW_CONTEXT_VERSION_MINOR);
    const int l_Profile = glfwGetWindowAttrib(pW, GLFW_OPENGL_PROFILE);
    if (l_Major >= 3) // Profiles introduced in OpenGL 3.0...
    {
        if (l_Profile == GLFW_OPENGL_COMPAT_PROFILE)
        {
            LOG_INFO("GLFW_OPENGL_COMPAT_PROFILE");
        }
        else
        {
            LOG_INFO("GLFW_OPENGL_CORE_PROFILE");
        }
    }
    LOG_INFO("OpenGL: %d.%d", l_Major, l_Minor);
    LOG_INFO("Vendor: %s", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    LOG_INFO("Renderer: %s", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
}

void initAuxPresentFboShader()
{
    g_auxPresent.initProgram("presentfbo");
    g_auxPresent.bindVAO();

    const float verts[] = {
        -1, -1,
        1, -1,
        1, 1,
        -1, 1
    };
    // The aspect ratio of one eye's view is half side-by-side(portrait), so we can chop
    // the top and bottom parts off to present something closer to landscape.
    const float texs[] = {
        0.0f, 0.25f,
        0.5f, 0.25f,
        0.5f, 0.75f,
        0.0f, 0.75f,
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    g_auxPresent.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(g_auxPresent.GetAttrLoc("vPosition"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint texVbo = 0;
    glGenBuffers(1, &texVbo);
    g_auxPresent.AddVbo("vTex", texVbo);
    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    glBufferData(GL_ARRAY_BUFFER, 4*2*sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(g_auxPresent.GetAttrLoc("vTex"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(g_auxPresent.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(g_auxPresent.GetAttrLoc("vTex"));

    glUseProgram(g_auxPresent.prog());
    {
        const glm::mat4 id(1.0f);
        glUniformMatrix4fv(g_auxPresent.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(id));
        glUniformMatrix4fv(g_auxPresent.GetUniLoc("prmtx"), 1, false, glm::value_ptr(id));
    }
    glUseProgram(0);

    glBindVertexArray(0);
}

void presentSharedFboTexture()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, g_auxWindow_w, g_auxWindow_h);

    // Present FBO to screen
    const GLuint prog = g_auxPresent.prog();
    glUseProgram(prog);
    g_auxPresent.bindVAO();
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_app.getRenderBufferTex());
        glUniform1i(g_auxPresent.GetUniLoc("fboTex"), 0);

        // This is the only uniform that changes per-frame
        const float fboScale = g_renderMode.outputType == RenderingMode::OVR_SDK ?
            1.0f :
            g_app.GetFBOScale();
        glUniform1f(g_auxPresent.GetUniLoc("fboScale"), fboScale);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

void displayToHMD()
{
    g_app.DoSceneRenderPrePasses();

    switch(g_renderMode.outputType)
    {
    case RenderingMode::Mono_Raw:
        g_app.display_raw();
        glfwSwapBuffers(g_pHMDWindow);
        break;

    case RenderingMode::Mono_Buffered:
        g_app.display_buffered();
        glfwSwapBuffers(g_pHMDWindow);
        break;

#ifdef USE_OCULUSSDK
    case RenderingMode::SideBySide_Undistorted:
        g_app.display_stereo_undistorted();
        glfwSwapBuffers(g_pHMDWindow);
        break;

    case RenderingMode::OVR_SDK:
        g_app.display_sdk();
        // OVR will do its own swap
        break;

    case RenderingMode::OVR_Client:
        g_app.display_client();
        glfwSwapBuffers(g_pHMDWindow);
        break;
#endif
    default:
        LOG_ERROR("Unknown display type: %d", g_renderMode.outputType);
        break;
    }
}

///@return An auxiliary "control view" window to display a monoscopic view of the world
/// that the Rift user is inhabiting(on the primary VR window). Yes, this takes resources
/// away from the VR user's rendering and will lower the rendering throughput(MPx/sec)
/// available to the HMD. It should not negatively impact latency until frame rate drops
/// below the display's refresh rate(which will happen sooner with this extra load, but
/// can be tuned). Pixel fill can be tuned by adjusting the FBO render target size with
/// the mouse wheel, but vertex rate cannot and another render pass adds 50%.
///@todo A more palatable solution is to share the FBO render target between this and
/// the Rift window and just present the left half of it.
GLFWwindow* initializeAuxiliaryWindow(GLFWwindow* pRiftWindow)
{
    ///@todo Set size to half FBO target width
    GLFWwindow* pAuxWindow = glfwCreateWindow(g_auxWindow_w, g_auxWindow_h, "Control Window", NULL, pRiftWindow);
    if (pAuxWindow == NULL)
    {
        return NULL;
    }

    glfwMakeContextCurrent(pAuxWindow);
    {
        // Create context-specific data here
        initAuxPresentFboShader();
    }

    glfwSetMouseButtonCallback(pAuxWindow, mouseDown_Aux);
    glfwSetCursorPosCallback(pAuxWindow, mouseMove_Aux);
    glfwSetScrollCallback(pAuxWindow, mouseWheel_Aux);
    glfwSetKeyCallback(pAuxWindow, keyboard_Aux);
    glfwSetWindowSizeCallback(pAuxWindow, resize_Aux);

    // The window will be shown whether we do this or not (on Windows)...
    glfwShowWindow(pAuxWindow);

    glfwMakeContextCurrent(pRiftWindow);

    return pAuxWindow;
}

void destroyAuxiliaryWindow(GLFWwindow* pAuxWindow)
{
    glfwMakeContextCurrent(pAuxWindow);
    g_auxPresent.destroy();
    glfwDestroyWindow(pAuxWindow);
    g_AuxWindow = NULL;
}

void LoadConfigFile()
{
    const std::string cgfFile = "../RiftRay.cfg";

    std::ifstream file;
    file.open(cgfFile.c_str(), std::ios::in);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file,line))
    {
        const std::vector<std::string> toks = split(line, '=');
        if (toks.size() < 2)
            continue;
        const std::string& t = toks[0];
        if (!t.compare("DynamicallyScaleFBO"))
        {
            const int v = atoi(toks[1].c_str());
            g_dynamicallyScaleFBO = (v != 0);
        }
        else if (!t.compare("LoadShadertoysRecursively"))
        {
            const int v = atoi(toks[1].c_str());
            g_loadShadertoysRecursive = (v != 0);
        }
        else if (!t.compare("FboMinimumScale"))
        {
            const float v = static_cast<float>(atof(toks[1].c_str()));
            g_app.m_fboMinScale = v;
        }
        else if (!t.compare("AnimatedThumbnails"))
        {
            const int at = static_cast<int>(atof(toks[1].c_str()));
            g_app.m_galleryScene.m_globalShadertoyState.animatedThumbnails = (at != 0);
        }
        else if (!t.compare("PanesAsPortals"))
        {
            const int pp = static_cast<int>(atof(toks[1].c_str()));
            g_app.m_galleryScene.m_globalShadertoyState.panesAsPortals = (pp != 0);
        }
        else if (!t.compare("ThumbnailFboSize"))
        {
            const int ts = static_cast<int>(atof(toks[1].c_str()));
            g_app.m_galleryScene.m_paneDimensionPixels = ts;
        }
    }
    file.close();
}

void StartShaderLoad()
{
    g_app.SetTextureLibraryPointer();

    g_app.DiscoverShaders(g_loadShadertoysRecursive);

    ///@todo It would save some time to compile all these shaders in parallel on
    /// a multicore machine. Even cooler would be compiling them in a background thread
    /// while display is running, but trying that yields large frame rate drops
    /// which would make the VR experience unacceptably uncomfortable.
    g_app.LoadTextureLibrary();
    g_app.m_galleryScene.CompileShaders();
    g_app.m_galleryScene.RearrangePanes();
    g_app.m_galleryScene.RenderThumbnails();

    std::cout
        << std::endl
        << std::endl;

#if 0
    // Create a context sharing data with our main window.
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    GLFWwindow* pThreadWindow = glfwCreateWindow(100, 100, "Control Window", NULL, g_pHMDWindow);
    if (pThreadWindow == NULL)
    {
        return;
    }

    // Start capture thread
    const int res = pthread_create(&g_shaderLoadThread, NULL, ThreadFunction, pThreadWindow);
    if (res)
    {
        printf("pthread_create failed\n");
    }
#endif
}

// OpenGL debug callback
void GLAPIENTRY myCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *msg,
#ifndef _LINUX
    const
#endif
    void *data)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    case GL_DEBUG_SEVERITY_MEDIUM:
    case GL_DEBUG_SEVERITY_LOW:
        LOG_INFO("[[GL Debug]] %x %x %x %x %s", source, type, id, severity, msg);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        break;
    }
}

int main(int argc, char** argv)
{
    LOG_INFO("RiftRay version %s", pRiftRayVersion);
#if defined(_WIN32)
    LOG_INFO("Windows build.");
#elif defined(_LINUX)
    LOG_INFO("Linux build.");
#elif defined(_MACOS)
    LOG_INFO("MacOS build.");
#endif

    bool useOpenGLCoreContext = false;

    g_renderMode.outputType = RenderingMode::OVR_SDK;

#ifdef USE_CORE_CONTEXT
    useOpenGLCoreContext = true;
#endif

#ifdef _LINUX
    // Linux driver seems to be lagging a bit
    useOpenGLCoreContext = false;
#endif

    LOG_INFO("Using GLFW3 backend.");
    LOG_INFO("Compiled against GLFW %i.%i.%i",
        GLFW_VERSION_MAJOR,
        GLFW_VERSION_MINOR,
        GLFW_VERSION_REVISION);
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    LOG_INFO("Running against GLFW %i.%i.%i", major, minor, revision);
    LOG_INFO("glfwGetVersionString: %s", glfwGetVersionString());

    // Command line options
    for (int i=0; i<argc; ++i)
    {
        const std::string a = argv[i];
        LOG_INFO("argv[%d]: %s", i, a.c_str());
        if (!a.compare("-sdk"))
        {
            g_renderMode.outputType = RenderingMode::OVR_SDK;
        }
        else if (!a.compare("-client"))
        {
            g_renderMode.outputType = RenderingMode::OVR_Client;
        }
        else if (!a.compare("-core"))
        {
            useOpenGLCoreContext = true;
        }
        else if (!a.compare("-compat"))
        {
            useOpenGLCoreContext = false;
        }
    }

    LoadConfigFile();

    g_app.initHMD();

    GLFWwindow* l_Window = NULL;
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

#ifdef __APPLE__
    // Set the working directory to the Resources dir of the .app bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX);
    CFRelease(resourcesURL);
    strcat( path, "/shaders" );
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
        chdir(path);
#endif

#ifndef _LINUX
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#  if defined(_MACOS)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#  else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#  endif
#endif //ndef _LINUX
    if (useOpenGLCoreContext)
    {
        LOG_INFO("Using OpenGL core context.");
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    }
    else
    {
#ifndef _LINUX
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
    }

    glfwWindowHint(GLFW_SAMPLES, 0);
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    std::string windowTitle = "RiftRay-v" + std::string(pRiftRayVersion);
#ifdef USE_OCULUSSDK
    const ovrSizei sz = g_app.getHmdResolution();
    const ovrVector2i pos = g_app.getHmdWindowPos();

    if (g_app.UsingDebugHmd() == true)
    {
        // Create a normal, decorated application window
        LOG_INFO("Using Debug HMD mode.");
        windowTitle += "-GLFW-DebugHMD";

        l_Window = glfwCreateWindow(sz.w, sz.h, windowTitle.c_str(), NULL, NULL);
        g_app.m_dashScene.m_bDraw = false;
        g_renderMode.outputType = RenderingMode::Mono_Buffered;
    }
    else if (g_app.UsingDirectMode())
    {
        LOG_INFO("Using Direct to Rift mode.");
        windowTitle += "-GLFW-Direct";

        l_Window = glfwCreateWindow(sz.w, sz.h, windowTitle.c_str(), NULL, NULL);

#if defined(_WIN32)
        g_app.AttachToWindow((void*)glfwGetWin32Window(l_Window));
#endif
    }
    else
    {
        LOG_INFO("Using Extended desktop mode.");
        windowTitle += "-GLFW-Extended";

        glfwWindowHint(GLFW_DECORATED, 0);
        l_Window = glfwCreateWindow(sz.w, sz.h, windowTitle.c_str(), NULL, NULL);
        glfwWindowHint(GLFW_DECORATED, 1);
        glfwSetWindowPos(l_Window, pos.x, pos.y);
    }

    resize(l_Window, sz.w, sz.h); // inform AppSkeleton of window size

#else
    const glm::vec2 sz(800, 600);
    // Create a normal, decorated application window
    LOG_INFO("Using No VR SDK.");
    windowTitle += "-GLFW-NoVRSDK";
    g_renderMode.outputType = RenderingMode::Mono_Buffered;

    l_Window = glfwCreateWindow(sz.x, sz.y, windowTitle.c_str(), NULL, NULL);
#endif //USE_OSVR|USE_OCULUSSDK

    if (!l_Window)
    {
        LOG_INFO("Glfw failed to create a window. Exiting.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

#ifdef USE_OCULUSSDK
    // Required for SDK rendering (to do the buffer swap on its own)
#  if defined(_WIN32)
    g_app.setWindow(glfwGetWin32Window(l_Window));
#  elif defined(__linux__)
    g_app.setWindow(glfwGetX11Display());
#  endif
#endif

    glfwMakeContextCurrent(l_Window);
    glfwSetWindowSizeCallback(l_Window, resize);
    glfwSetMouseButtonCallback(l_Window, mouseDown);
    glfwSetCursorPosCallback(l_Window, mouseMove);
    glfwSetScrollCallback(l_Window, mouseWheel);
    glfwSetKeyCallback(l_Window, keyboard);

    memset(m_keyStates, 0, GLFW_KEY_LAST*sizeof(int));

    FindPreferredJoystick();

    // Log system monitor information
    const GLFWmonitor* pPrimary = glfwGetPrimaryMonitor();
    int monitorCount = 0;
    GLFWmonitor** ppMonitors = glfwGetMonitors(&monitorCount);
    for (int i=0; i<monitorCount; ++i)
    {
        GLFWmonitor* pCur = ppMonitors[i];
        const GLFWvidmode* mode = glfwGetVideoMode(pCur);
        if (mode != NULL)
        {
            LOG_INFO("Monitor #%d: %dx%d @ %dHz %s",
                i,
                mode->width,
                mode->height,
                mode->refreshRate,
                pCur==pPrimary ? "Primary":"");
        }
    }

    printGLContextInfo(l_Window);
    glfwMakeContextCurrent(l_Window);
    g_pHMDWindow = l_Window;


    // Don't forget to initialize Glew, turn glewExperimental on to
    // avoid problems fetching function pointers...
    glewExperimental = GL_TRUE;
    const GLenum l_Result = glewInit();
    if (l_Result != GLEW_OK)
    {
        LOG_INFO("glewInit() error.");
        exit(EXIT_FAILURE);
    }

#ifdef _DEBUG
    // Debug callback initialization
    // Must be done *after* glew initialization.
    glDebugMessageCallback(myCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
        GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

#ifdef USE_ANTTWEAKBAR
    LOG_INFO("Using AntTweakbar.");
    TwInit(useOpenGLCoreContext ? TW_OPENGL_CORE : TW_OPENGL, NULL);
    InitializeBar();
#endif

    LOG_INFO("Calling initGL...");
    g_app.initGL();
    LOG_INFO("Calling initVR...");
    g_app.initVR();
    LOG_INFO("initVR complete.");

    SetVsync(1); // default to vsync on

    StartShaderLoad();

    while (!glfwWindowShouldClose(l_Window))
    {
        const bool tapped = g_app.CheckForTapOnHmd();
        if (tapped && (g_receivedFirstTap == false))
        {
            g_app.RecenterPose();
            g_receivedFirstTap = true;
        }

        glfwPollEvents();
        joystick();
        timestep();
        g_fps.OnFrame();
        if (g_dynamicallyScaleFBO)
        {
            DynamicallyScaleFBO();
        }

#ifdef USE_ANTTWEAKBAR
        TwRefreshBar(g_pTweakbar);
        TwRefreshBar(g_pShaderTweakbar);
#endif

        displayToHMD();

#ifndef _LINUX
        // Indicate FPS in window title
        // This is absolute death for performance in Ubuntu Linux 12.04
        {
            std::ostringstream oss;
            oss << windowTitle
                << " "
                << static_cast<int>(g_fps.GetFPS())
                << " fps";
            glfwSetWindowTitle(l_Window, oss.str().c_str());
            if (g_AuxWindow != NULL)
                glfwSetWindowTitle(g_AuxWindow, oss.str().c_str());
        }
#endif
        const float dumpInterval = 1.f;
        if (g_logDumpTimer.seconds() > dumpInterval)
        {
            LOG_INFO("Frame rate: %d fps", static_cast<int>(g_fps.GetFPS()));
            g_logDumpTimer.reset();
        }

        // Optionally display to auxiliary mono view
        if (g_AuxWindow != NULL)
        {
            glfwMakeContextCurrent(g_AuxWindow);
            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ///@note VAOs *cannot* be shared between contexts.
            ///@note GLFW windows are inextricably tied to their own unique context.
            /// For these two reasons, calling draw a third time for the auxiliary window
            /// is not possible. Furthermore, it is not strictly desirable for the extra
            /// rendering cost.
            /// Instead, we share the render target texture from the stereo render and present
            /// just the left eye to the aux window.
            if (g_drawToAuxWindow)
            {
                presentSharedFboTexture();
            }

#ifdef USE_ANTTWEAKBAR
            TwDraw(); ///@todo Should this go first? Will it write to a depth buffer?
#endif

            glfwSwapBuffers(g_AuxWindow);

            if (glfwWindowShouldClose(g_AuxWindow))
            {
                destroyAuxiliaryWindow(g_AuxWindow);
            }

            // Set context to Rift window when done
            glfwMakeContextCurrent(l_Window);
        }
    }

    g_app.exitVR();
    glfwDestroyWindow(l_Window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

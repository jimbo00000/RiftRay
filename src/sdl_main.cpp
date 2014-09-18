// sdl_main.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <sstream>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#undef main

#include <glm/gtc/type_ptr.hpp>

#ifdef USE_ANTTWEAKBAR
#  include <AntTweakBar.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sstream>

#include "RiftAppSkeleton.h"
#include "RenderingMode.h"
#include "Timer.h"
#include "FPSTimer.h"
#include "Logger.h"

RiftAppSkeleton g_app;
RenderingMode g_renderMode;
Timer g_timer;
FPSTimer g_fps;

int m_keyStates[4096];

// mouse motion internal state
int oldx, oldy, newx, newy;
int which_button = -1;
int modifier_mode = 0;

//ShaderWithVariables g_auxPresent;
SDL_Window* g_pHMDWindow = NULL;
SDL_Window* g_pAuxWindow = NULL;
Uint32 g_HMDWindowID = 0;
Uint32 g_AuxWindowID = 0;
int g_auxWindow_w = 600;
int g_auxWindow_h = 600;

SDL_Joystick* g_pJoy = NULL;

float g_fpsSmoothingFactor = 0.02f;
float g_fpsDeltaThreshold = 5.0f;
bool g_dynamicallyScaleFBO = true;
int g_targetFPS = 100;

#ifdef USE_ANTTWEAKBAR
TwBar* g_pTweakbar = NULL;
#endif

SDL_Window* initializeAuxiliaryWindow();
void destroyAuxiliaryWindow(SDL_Window* pAuxWindow);

// Set VSync is framework-dependent and has to come before the include
static void SetVsync(int state) {}

#include "main_include.cpp"

void timestep()
{
    float dt = (float)g_timer.seconds();
    g_timer.reset();
    g_app.timestep(dt);
}



void keyboard(const SDL_Event& event, int key, int codes, int action, int mods)
{
    (void)codes;
    (void)mods;

    const int KEYUP = 0;
    const int KEYDOWN = 1;
    if ((key > -1) && (key <= 4096))
    {
        int keystate = KEYUP;
        if (action == SDL_KEYDOWN)
            keystate = 1;
        m_keyStates[key] = keystate;
        //printf("key ac  %d %d\n", key, action);
    }

    if (action == SDL_KEYDOWN)
    {
        switch (key)
        {
        default:
            g_app.DismissHealthAndSafetyWarning();
            break;

        case SDLK_F1:
            if (SDL_GetModState() & KMOD_LCTRL)
                g_renderMode.toggleRenderingTypeReverse();
            else
                g_renderMode.toggleRenderingType();
            break;

        case SDLK_F2:
            g_renderMode.toggleRenderingTypeMono();
            break;

        case SDLK_F3:
            g_renderMode.toggleRenderingTypeHMD();
            break;

        case SDLK_F4:
            g_renderMode.toggleRenderingTypeDistortion();
            break;

        case '`':
            if (g_pAuxWindow == NULL)
            {
                g_pAuxWindow = initializeAuxiliaryWindow();
            }
            else
            {
                destroyAuxiliaryWindow(g_pAuxWindow);
                g_pAuxWindow = NULL;
            }
            break;

        case SDLK_SPACE:
            g_app.RecenterPose();
            break;

        case 'r':
            g_app.ResetAllTransformations();
            break;

        case SDLK_ESCAPE:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                if (event.key.windowID == g_HMDWindowID)
                {
                    exit(0);
                }
                else
                {
                    destroyAuxiliaryWindow(g_pAuxWindow);
                    g_pAuxWindow = NULL;
                }
            }
            break;
        }
    }

    //g_app.keyboard(key, action, 0,0);


    // Handle keyboard movement(WASD & QE keys)
    // Are these reversed??
    glm::vec3 keyboardMove(0.0f, 0.0f, 0.0f);
    if (m_keyStates['w'] != KEYUP)
    {
        keyboardMove += glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (m_keyStates['s'] != KEYUP)
    {
        keyboardMove += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (m_keyStates['a'] != KEYUP)
    {
        keyboardMove += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (m_keyStates['d'] != KEYUP)
    {
        keyboardMove += glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (m_keyStates['q'] != KEYUP)
    {
        keyboardMove += glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (m_keyStates['e'] != KEYUP)
    {
        keyboardMove += glm::vec3(0.0f, 1.0f, 0.0f);
    }

    float mag = 1.0f;
    if (SDL_GetModState() & KMOD_LSHIFT)
        mag *= 0.1f;
    if (SDL_GetModState() & KMOD_LCTRL)
        mag *= 10.0f;

    // Yaw keys
    g_app.m_keyboardYaw = 0.0f;
    const float dyaw = 0.5f * mag; // radians at 60Hz timestep
    if (m_keyStates['1'] != KEYUP)
    {
        g_app.m_keyboardYaw = -dyaw;
    }
    if (m_keyStates['3'] != KEYUP)
    {
        g_app.m_keyboardYaw = dyaw;
    }

    g_app.m_keyboardMove = mag * keyboardMove;
}

// Joystick state is polled here and stored within SDL.
void joystick()
{
    if (g_pJoy == NULL)
        return;

    glm::vec3 joystickMove(0.0f, 0.0f, 0.0f);

    if (SDL_JoystickGetButton(g_pJoy, 0) != 0)
    {
        joystickMove += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 1) != 0)
    {
        joystickMove += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 2) != 0)
    {
        joystickMove += glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 3) != 0)
    {
        joystickMove += glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 4) != 0)
    {
        joystickMove += glm::vec3(0.0f, 2.0f, 0.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 6) != 0)
    {
        joystickMove += glm::vec3(0.0f, -2.0f, 0.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 5) != 0)
    {
        joystickMove += glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (SDL_JoystickGetButton(g_pJoy, 7) != 0)
    {
        joystickMove += glm::vec3(0.0f, -1.0f, 0.0f);
    }

    float mag = 1.0f;
    g_app.m_joystickMove = mag * joystickMove;

    Sint16 x_move = SDL_JoystickGetAxis(g_pJoy, 0);
    const int deadZone = 512;
    if (abs(x_move) < deadZone)
        x_move = 0;
    g_app.m_joystickYaw = 0.00005f * static_cast<float>(x_move);
}

void mouseDown(int button, int state, int x, int y)
{
    which_button = button;
    oldx = newx = x;
    oldy = newy = y;
    if (state == SDL_RELEASED)
    {
        which_button = -1;
    }
}

void mouseMove(int x, int y)
{
    oldx = newx;
    oldy = newy;
    newx = x;
    newy = y;
    const int mmx = x-oldx;
    const int mmy = y-oldy;

    g_app.m_mouseDeltaYaw = 0.0f;
    g_app.m_mouseMove = glm::vec3(0.0f);

    if (which_button == SDL_BUTTON_LEFT)
    {
        const float spinMagnitude = 0.05f;
        g_app.m_mouseDeltaYaw += static_cast<float>(mmx) * spinMagnitude;
    }
    else if (which_button == SDL_BUTTON_RIGHT)
    {
        const float moveMagnitude = 0.5f;
        g_app.m_mouseMove.x += static_cast<float>(mmx) * moveMagnitude;
        g_app.m_mouseMove.z += static_cast<float>(mmy) * moveMagnitude;
    }
}

void mouseDown_Aux(int button, int state, int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    TwMouseAction action = TW_MOUSE_RELEASED;
    if (state == SDL_PRESSED)
    {
        action = TW_MOUSE_PRESSED;
    }
    TwMouseButtonID buttonID = TW_MOUSE_LEFT;
    if (button == SDL_BUTTON_LEFT)
        buttonID = TW_MOUSE_LEFT;
    else if (button == SDL_BUTTON_MIDDLE)
        buttonID = TW_MOUSE_MIDDLE;
    else if (button == SDL_BUTTON_RIGHT)
        buttonID = TW_MOUSE_RIGHT;
    const int ant = TwMouseButton(action, buttonID);
    if (ant != 0)
        return;
#endif
    mouseDown(button, state, x, y);
}

void mouseMove_Aux(int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    const int ant = TwMouseMotion(x, y);
    if (ant != 0)
        return;
#endif
    mouseMove(x, y);
}

void display()
{
    switch(g_renderMode.outputType)
    {
    case RenderingMode::Mono_Raw:
        g_app.display_raw();
        SDL_GL_SwapWindow(g_pHMDWindow);
        break;

    case RenderingMode::Mono_Buffered:
        g_app.display_buffered();
        SDL_GL_SwapWindow(g_pHMDWindow);
        break;

    case RenderingMode::SideBySide_Undistorted:
        g_app.display_stereo_undistorted();
        SDL_GL_SwapWindow(g_pHMDWindow);
        break;

    case RenderingMode::OVR_SDK:
        g_app.display_sdk();
        // OVR will do its own swap
        break;

    case RenderingMode::OVR_Client:
        g_app.display_client();
        SDL_GL_SwapWindow(g_pHMDWindow);
        break;

    default:
        LOG_ERROR("Unknown display type: %d", g_renderMode.outputType);
        break;
    }
}

void PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            keyboard(event, event.key.keysym.sym, 0, event.key.type, 0);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (event.window.windowID == g_AuxWindowID)
                mouseDown_Aux(event.button.button, event.button.state, event.button.x, event.button.y);
            else
                mouseDown(event.button.button, event.button.state, event.button.x, event.button.y);
            break;

        case SDL_MOUSEMOTION:
            if (event.window.windowID == g_AuxWindowID)
                mouseMove_Aux(event.motion.x, event.motion.y);
            else
                mouseMove(event.motion.x, event.motion.y);
            break;

        case SDL_MOUSEWHEEL:
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                if (event.window.windowID == g_AuxWindowID)
                {
                    destroyAuxiliaryWindow(g_pAuxWindow);
                    g_pAuxWindow = NULL;
                }
            }
            else if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                if (event.window.windowID != g_HMDWindowID)
                {
                    const int w = event.window.data1;
                    const int h = event.window.data2;
                    TwWindowSize(w,h);
                }
            }
            break;

        case SDL_QUIT:
            exit(0);
            break;

        default:
            break;
        }
    }

    SDL_JoystickUpdate();
    joystick();
}

SDL_Window* initializeAuxiliaryWindow()
{
    SDL_Window* pAuxWindow = SDL_CreateWindow(
        "GL Skeleton - SDL2",
        10, 10, //SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        g_auxWindow_w, g_auxWindow_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    g_AuxWindowID = SDL_GetWindowID(pAuxWindow);
    return pAuxWindow;
}

void destroyAuxiliaryWindow(SDL_Window* pAuxWindow)
{
    SDL_DestroyWindow(pAuxWindow);
    g_AuxWindowID = -1;
}

int main(void)
{
    ///@todo cmd line aargs

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return false;
    }

    // This call assumes the Rift display is in extended mode.
    g_app.initHMD();
    const ovrSizei sz = g_app.getHmdResolution();
    const ovrVector2i pos = g_app.getHmdWindowPos();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // According to the OVR SDK 0.3.2 Overview, WindowsPos will be set to (0,0)
    // if not supported. This will also be the case if the Rift DK1 display is
    // cloned/duplicated to the main(convenient for debug).
    int wx = SDL_WINDOWPOS_UNDEFINED;
    int wy = SDL_WINDOWPOS_UNDEFINED;
    if ((pos.x == 0) && (pos.y == 0))
    {
        // Windowed mode
    }
    else
    {
        wx = pos.x;
        wy = pos.y;
        g_renderMode.outputType = RenderingMode::OVR_SDK;
    }

    g_pHMDWindow = SDL_CreateWindow(
        "GL Skeleton - SDL2",
        wx, wy,
        sz.w, sz.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (g_pHMDWindow == NULL)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        SDL_Quit();
    }

    g_HMDWindowID = SDL_GetWindowID(g_pHMDWindow);

    // thank you http://www.brandonfoltz.com/2013/12/example-using-opengl-3-0-with-sdl2-and-glew/
    SDL_GLContext glContext = SDL_GL_CreateContext(g_pHMDWindow);
    if (glContext == NULL)
    {
        printf("There was an error creating the OpenGL context!\n");
        return 0;
    }

    const unsigned char *version = glGetString(GL_VERSION);
    if (version == NULL) 
    {
        printf("There was an error creating the OpenGL context!\n");
        return 1;
    }

    printf("OpenGL: %s ", version);
    printf("Vendor: %s\n", (char*)glGetString(GL_VENDOR));
    printf("Renderer: %s\n", (char*)glGetString(GL_RENDERER));
    LOG_INFO("OpenGL: %s ", version);
    LOG_INFO("Vendor: %s\n", (char*)glGetString(GL_VENDOR));
    LOG_INFO("Renderer: %s\n", (char*)glGetString(GL_RENDERER));

    SDL_GL_MakeCurrent(g_pHMDWindow, glContext);

    // Don't forget to initialize Glew, turn glewExperimental on to
    // avoid problems fetching function pointers...
    glewExperimental = GL_TRUE;
    const GLenum l_Result = glewInit();
    if (l_Result != GLEW_OK)
    {
        printf("glewInit() error.\n");
        LOG_INFO("glewInit() error.\n");
        exit(EXIT_FAILURE);
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(g_pHMDWindow, &info);
#if defined(OVR_OS_WIN32)
    g_app.setWindow(info.info.win.window);
#elif defined(OVR_OS_LINUX)
    g_app.setWindow(info.info.x11.window, info.info.x11.display);
#endif

    LOG_INFO("Calling initGL...");
    g_app.initGL();
    LOG_INFO("Calling initVR...");
    g_app.initVR();
    LOG_INFO("initVR complete.");

#ifdef USE_ANTTWEAKBAR
    TwInit(TW_OPENGL, NULL);
    InitializeBar();
#endif

    memset(m_keyStates, 0, 4096*sizeof(int));

    printf("\n");
    // Joysticks/gamepads
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    // Check for joystick
    if (SDL_NumJoysticks() > 0)
    {
        g_pJoy = SDL_JoystickOpen(0);
    }
    if (g_pJoy != NULL)
    {
        printf("Opened Joystick 0\n");
        printf("Name: %s\n", SDL_JoystickName(0));
        printf("Number of Axes: %d\n", SDL_JoystickNumAxes(g_pJoy));
        printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(g_pJoy));
        printf("Number of Balls: %d\n", SDL_JoystickNumBalls(g_pJoy));
    }

    int quit = 0;
    while (quit == 0)
    {
        g_app.CheckForTapToDismissHealthAndSafetyWarning();
        PollEvents();
        timestep();
        g_fps.OnFrame();
        if (g_dynamicallyScaleFBO)
        {
            DynamicallyScaleFBO();
        }

        display();

#ifndef _LINUX
        // Indicate FPS in window title
        // This is absolute death for performance in Ubuntu Linux 12.04
        {
            std::ostringstream oss;
            oss << "SDL Oculus Rift Test - "
                << static_cast<int>(g_fps.GetFPS())
                << " fps";
            SDL_SetWindowTitle(g_pHMDWindow, oss.str().c_str());
            if (g_pAuxWindow != NULL)
                SDL_SetWindowTitle(g_pAuxWindow, oss.str().c_str());
        }
#endif

        // Optionally display to auxiliary mono view
        if (g_pAuxWindow != NULL)
        {
            // SDL allows us to share contexts, so we can just call display on g_app
            // and use all of the VAOs resident in the HMD window's context.
            SDL_GL_MakeCurrent(g_pAuxWindow, glContext);

            // Get window size from SDL - is this worth caching?
            int w, h;
            SDL_GetWindowSize(g_pAuxWindow, &w, &h);

            glPushAttrib(GL_VIEWPORT_BIT);
            glViewport(0, 0, w, h);
            g_app.display_buffered(false);
            glPopAttrib(); // GL_VIEWPORT_BIT - if this is not misused!

#ifdef USE_ANTTWEAKBAR
            TwRefreshBar(g_pTweakbar);
            TwDraw(); ///@todo Should this go first? Will it write to a depth buffer?
#endif

            SDL_GL_SwapWindow(g_pAuxWindow);

            // Set context to Rift window when done
            SDL_GL_MakeCurrent(g_pHMDWindow, glContext);
        }
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(g_pHMDWindow);

    g_app.exitVR();

    if (g_pJoy != NULL)
        SDL_JoystickClose(g_pJoy);

    SDL_Quit();
}

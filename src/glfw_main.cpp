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

#include "RiftAppSkeleton.h"
#include "RenderingMode.h"
#include "Timer.h"
#include "FPSTimer.h"
#include "Logger.h"
#include "StringFunctions.h"

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#include <sys/stat.h>
#endif

RiftAppSkeleton g_app;
RenderingMode g_renderMode;
Timer g_timer;
FPSTimer g_fps;

bool g_receivedFirstTap = false;
int m_keyStates[GLFW_KEY_LAST];

// mouse motion internal state
int oldx, oldy, newx, newy;
int which_button = -1;
int modifier_mode = 0;

ShaderWithVariables g_auxPresent;
GLFWwindow* g_pHMDWindow = NULL;
GLFWwindow* g_AuxWindow = NULL;
int g_auxWindow_w = 330;
int g_auxWindow_h = 800;

int g_joystickIdx = -1;

float g_fpsSmoothingFactor = 0.02f;
float g_fpsDeltaThreshold = 5.0f;
bool g_dynamicallyScaleFBO = true;
int g_targetFPS = 100;
bool g_loadShadertoysRecursive = true;

#ifdef USE_ANTTWEAKBAR
TwBar* g_pTweakbar = NULL;
#endif

GLFWwindow* initializeAuxiliaryWindow(GLFWwindow* pRiftWindow);
void destroyAuxiliaryWindow(GLFWwindow* pAuxWindow);

// Set VSync is framework-dependent and has to come before the include
///@param state 0=off, 1=on, -1=adaptive
// Set vsync for both contexts.
static void SetVsync(int state)
{
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
    printf("ERROR: %d, %s\n", p_Error, p_Description);
    LOG_INFO("ERROR: %d, %s\n", p_Error, p_Description);
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

    if (action == GLFW_PRESS)
    {
    switch (key)
    {
        default:
            g_app.DismissHealthAndSafetyWarning();
            break;

        case GLFW_KEY_F1:
            if (mods & GLFW_MOD_CONTROL)
            {
                g_renderMode.toggleRenderingTypeReverse();
                LOG_INFO("Called toggleRenderingTypeReverse");
            }
            else
            {
                g_renderMode.toggleRenderingType();
                LOG_INFO("Called toggleRenderingType");
            }
            break;

        case GLFW_KEY_F2:
            g_renderMode.toggleRenderingTypeMono();
            LOG_INFO("Called toggleRenderingTypeMono");
            break;

        case GLFW_KEY_F3:
            g_renderMode.toggleRenderingTypeHMD();
            LOG_INFO("Called toggleRenderingTypeHMD");
            break;

        case GLFW_KEY_F4:
            g_renderMode.toggleRenderingTypeDistortion();
            LOG_INFO("Called toggleRenderingTypeDistortion");
            break;

        case '`':
            ///@todo Is there a way to create an auxiliary window in Direct to rift mode?
            /// The call to glfwCreateWindow crashes the app in Win7.
            if (g_app.UsingDirectMode() == false)
            {
                if (g_AuxWindow == NULL)
                {
                    g_AuxWindow = initializeAuxiliaryWindow(g_pHMDWindow);
                }
                else
                {
                    destroyAuxiliaryWindow(g_AuxWindow);
                    glfwMakeContextCurrent(g_pHMDWindow);
                }
            }
            break;

        case GLFW_KEY_SPACE:
            g_app.RecenterPose();
            break;

        case 'R':
            g_app.ResetAllTransformations();
            break;

        case GLFW_KEY_ENTER:
            g_app.ToggleShaderWorld();
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

    // Handle keyboard movement(WASD keys)
    glm::vec3 keyboardMove(0.0f, 0.0f, 0.0f);
    if (m_keyStates['W'] != GLFW_RELEASE)
    {
        keyboardMove += glm::vec3(0.0f, 0.0f, -1.0f);
    }
    if (m_keyStates['S'] != GLFW_RELEASE)
    {
        keyboardMove += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (m_keyStates['A'] != GLFW_RELEASE)
    {
        keyboardMove += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (m_keyStates['D'] != GLFW_RELEASE)
    {
        keyboardMove += glm::vec3(1.0f, 0.0f, 0.0f);
    }
    if (m_keyStates['Q'] != GLFW_RELEASE)
    {
        keyboardMove += glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (m_keyStates['E'] != GLFW_RELEASE)
    {
        keyboardMove += glm::vec3(0.0f, 1.0f, 0.0f);
    }

    float mag = 1.0f;
    if (m_keyStates[GLFW_KEY_LEFT_SHIFT ] != GLFW_RELEASE)
        mag *= 0.1f;
    if (m_keyStates[GLFW_KEY_LEFT_CONTROL ] != GLFW_RELEASE)
        mag *= 10.0f;

    // Yaw keys
    g_app.m_keyboardYaw = 0.0f;
    const float dyaw = 0.5f * mag; // radians at 60Hz timestep
    if (m_keyStates['1'] != GLFW_RELEASE)
    {
        g_app.m_keyboardYaw = -dyaw;
    }
    if (m_keyStates['3'] != GLFW_RELEASE)
    {
        g_app.m_keyboardYaw = dyaw;
    }

    g_app.m_keyboardMove = mag * keyboardMove;
}

void joystick()
{
    if (g_joystickIdx == -1)
        return;

    ///@todo Do these calls take time? We can move them out if so
    int joyStick1Present = GL_FALSE;
    joyStick1Present = glfwJoystickPresent(g_joystickIdx);
    if (joyStick1Present != GL_TRUE)
        return;

    // Poll joystick
    int numAxes = 0;
    const float* pAxisStates = glfwGetJoystickAxes(g_joystickIdx, &numAxes);
    if (numAxes < 2)
        return;

    static char s_lastButtons[256] = {0};
    int numButtons = 0;
    const unsigned char* pButtonStates = glfwGetJoystickButtons(g_joystickIdx, &numButtons);
    if (numButtons < 10)
        return;

    glm::vec3 joystickMove(0.0f, 0.0f, 0.0f);
    // Map joystick buttons to move directions
    glm::vec3 moveDirs[8] = {
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), // left shoulder
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), // left shoulder
        glm::vec3(0.0f, -1.0f, 0.0f),
    };

    for (int i=0; i<std::min(8,numButtons); ++i)
    {
        if (pButtonStates[i] == GLFW_PRESS)
        {
            joystickMove += moveDirs[i];
        }
    }

    // Check for recent button pushes
    for (int i=0; i<numButtons; ++i)
    {
        if (
            (pButtonStates[i] == GLFW_PRESS) &&
            (s_lastButtons[i] != GLFW_PRESS)
            )
        {
            if (i == 9)
            {
                g_app.ToggleShaderWorld();
            }
        }
    }

    float mag = 1.0f;
    // Left shoulder buttons - if "select" is pressed, adjust vertical FOV.
    // Otherwise, boost or limit movement speed.
    if (pButtonStates[8] == GLFW_PRESS)
    {
        const float incr = 0.05f;
        float scope = g_app.m_cinemaScopeFactor;
        if (
            (pButtonStates[4] == GLFW_PRESS) &&
            (s_lastButtons[4] != GLFW_PRESS)
            )
        {
            scope += incr;
        }
        else if (
            (pButtonStates[6] == GLFW_PRESS) &&
            (s_lastButtons[6] != GLFW_PRESS)
            )
        {
            scope -= incr;
        }
        scope = std::max(0.0f, scope);
        scope = std::min(0.95f, scope);
        g_app.m_cinemaScopeFactor = scope;
    }
    else
    {
        if (pButtonStates[4] == GLFW_PRESS)
            mag *= 10.0f;
        if (pButtonStates[6] == GLFW_PRESS)
            mag /= 10.0f;
    }
    g_app.m_joystickMove = mag * joystickMove;


    float x_move = pAxisStates[0];
    const float deadzone = 0.2f;
    if (fabs(x_move) < deadzone)
        x_move = 0.0f;
    g_app.m_joystickYaw = 0.5f * static_cast<float>(x_move);

    memcpy(s_lastButtons, pButtonStates, numButtons);
}

void mouseDown(GLFWwindow* pWindow, int button, int action, int mods)
{
    (void)mods;

    double xd, yd;
    glfwGetCursorPos(pWindow, &xd, &yd);
    const int x = static_cast<int>(xd);
    const int y = static_cast<int>(yd);

    which_button = button;
    oldx = newx = x;
    oldy = newy = y;
    if (action == GLFW_RELEASE)
    {
        which_button = -1;
    }
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

    if (which_button == GLFW_MOUSE_BUTTON_1)
    {
        const float spinMagnitude = 0.05f;
        g_app.m_mouseDeltaYaw += static_cast<float>(mmx) * spinMagnitude;
    }
    else if (which_button == GLFW_MOUSE_BUTTON_2) // Right click
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
}

void mouseWheel(GLFWwindow* pWindow, double x, double y)
{
    (void)pWindow;
    (void)x;

    const float delta = static_cast<float>(y);
    const float incr = 0.05f;
    float cscope = g_app.m_cinemaScopeFactor;
    cscope += incr * delta;
    cscope = std::max(0.0f, cscope);
    cscope = std::min(0.95f, cscope);
    g_app.m_cinemaScopeFactor = cscope;
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
    float dt = static_cast<float>(g_timer.seconds());
    g_timer.reset();
    g_app.timestep(dt);
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
            printf("GLFW_OPENGL_COMPAT_PROFILE\n");
            LOG_INFO("GLFW_OPENGL_COMPAT_PROFILE\n");
        }
        else
        {
            printf("GLFW_OPENGL_CORE_PROFILE\n");
            LOG_INFO("GLFW_OPENGL_CORE_PROFILE\n");
        }
    }
    printf("OpenGL: %d.%d ", l_Major, l_Minor);
    printf("Vendor: %s\n", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    printf("Renderer: %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    LOG_INFO("OpenGL: %d.%d ", l_Major, l_Minor);
    LOG_INFO("Vendor: %s\n", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    LOG_INFO("Renderer: %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
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
            g_app.GetFboScale();
        glUniform1f(g_auxPresent.GetUniLoc("fboScale"), fboScale);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

void displayToHMD()
{
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
    }
    file.close();
}

void StartShaderLoad()
{
    g_app.DiscoverShaders(g_loadShadertoysRecursive);

    ///@todo It would save some time to compile all these shaders in parallel on
    /// a multicore machine. Even cooler would be compiling them in a background thread
    /// while display is running, but trying that yields large frame rate drops
    /// which would make the VR experience unacceptably uncomfortable.
    g_app.LoadTexturesFromFile();
    g_app.CompileShaders();
    g_app.m_paneScene.RearrangePanes();
    g_app.RenderThumbnails();

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

int main(void)
{
    ///@todo Command line options
    LoadConfigFile();

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


#ifdef USE_CORE_CONTEXT
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#if defined(_MACOS)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_SAMPLES, 0);

    // This call assumes the Rift display is in extended mode.
    g_app.initHMD();
    const ovrSizei sz = g_app.getHmdResolution();
    const ovrVector2i pos = g_app.getHmdWindowPos();

    if (g_app.UsingDirectMode())
    {
        printf("Using Direct to Rift mode...\n");
        LOG_INFO("Using Direct to Rift mode...\n");
        const GLFWmonitor* pPrimary = glfwGetPrimaryMonitor();
        int monitorCount = 0;
        GLFWmonitor** ppMonitors = glfwGetMonitors(&monitorCount);
        for (int i=0; i<monitorCount; ++i)
        {
            GLFWmonitor* pCur = ppMonitors[i];
            if (pCur == pPrimary)
                continue;
            const GLFWvidmode* mode = glfwGetVideoMode(pCur);
        }

        ///@note Without decorations, we pull <1 FPS on NVIDIA Win7 344.60
        //glfwWindowHint(GLFW_DECORATED, 0);
        l_Window = glfwCreateWindow(sz.w, sz.h, "RiftRay", NULL, NULL);
        //glfwWindowHint(GLFW_DECORATED, 1);
        glfwSetWindowPos(l_Window, pos.x, pos.y);

#if defined(_WIN32)
        g_app.AttachToWindow((void*)glfwGetWin32Window(l_Window));
#endif
    }
    else
    {
        l_Window = glfwCreateWindow(sz.w, sz.h, "RiftRay", NULL, NULL);
    }
    g_app.resize(sz.w, sz.h);

    if (g_app.UsingDebugHmd() == false)
    {
        glfwSetWindowPos(l_Window, pos.x, pos.y);
        g_renderMode.outputType = RenderingMode::OVR_SDK;
    }

    if (!l_Window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(l_Window);
    glfwSetWindowSizeCallback(l_Window, resize);
    glfwSetMouseButtonCallback(l_Window, mouseDown);
    glfwSetCursorPosCallback(l_Window, mouseMove);
    glfwSetScrollCallback(l_Window, mouseWheel);
    glfwSetKeyCallback(l_Window, keyboard);

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

    printGLContextInfo(l_Window);

    // Required for SDK rendering (to do the buffer swap on its own)
#if defined(_WIN32)
    g_app.setWindow(glfwGetWin32Window(l_Window));
#elif defined(__linux__)
    g_app.setWindow(glfwGetX11Window(l_Window), glfwGetX11Display());
#endif

#ifdef USE_ANTTWEAKBAR
  #ifdef USE_CORE_CONTEXT
    TwInit(TW_OPENGL_CORE, NULL);
  #else
    TwInit(TW_OPENGL, NULL);
  #endif
    InitializeBar();
#endif

    LOG_INFO("Calling initGL...");
    g_app.initGL();
    LOG_INFO("Calling initVR...");
    g_app.initVR();
    LOG_INFO("initVR complete.");

    memset(m_keyStates, 0, GLFW_KEY_LAST*sizeof(int));

    // joysticks
    printf("\n\n");
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i)
    {
        if (GL_FALSE == glfwJoystickPresent(i))
            continue;

        const char* pJoyName = glfwGetJoystickName(i);
        if (pJoyName == NULL)
            continue;

        printf("Opened Joystick %d: %s\n", i, pJoyName);
        g_joystickIdx = i;
        break;
    }

    glfwMakeContextCurrent(l_Window);
    g_pHMDWindow = l_Window;

    StartShaderLoad();

    while (!glfwWindowShouldClose(l_Window))
    {
        bool tapped = g_app.CheckForTapOnHmd();
        if (tapped && (g_receivedFirstTap == false))
        {
            g_renderMode.useClientDistortion();
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
#endif

        displayToHMD();

#ifndef _LINUX
        // Indicate FPS in window title
        // This is absolute death for performance in Ubuntu Linux 12.04
        {
            std::ostringstream oss;
            oss << "GLFW Oculus Rift Test - "
                << static_cast<int>(g_fps.GetFPS())
                << " fps";
            glfwSetWindowTitle(l_Window, oss.str().c_str());
            if (g_AuxWindow != NULL)
                glfwSetWindowTitle(g_AuxWindow, oss.str().c_str());
        }
#endif

        // Optionally display to auxiliary mono view
        if (g_AuxWindow != NULL)
        {
            glfwMakeContextCurrent(g_AuxWindow);
            glClearColor(0,1,0,1);
            glClear(GL_COLOR_BUFFER_BIT);

            ///@note VAOs *cannot* be shared between contexts.
            ///@note GLFW windows are inextricably tied to their own unique context.
            /// For these two reasons, calling draw a third time for the auxiliary window
            /// is not possible. Furthermore, it is not strictly desirable for the extra
            /// rendering cost.
            /// Instead, we share the render target texture from the stereo render and present
            /// just the left eye to the aux window.
            presentSharedFboTexture();

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

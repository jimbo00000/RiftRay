// simple_glfw_main.cpp
// GLFW Skeleton for the basic Oculus Rift/OVR OpenGL app.

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#ifdef USE_CUDA
#else
#  include "vector_make_helpers.h"
#endif
#include "vectortypes.h"

#include "Logger.h"
#include "FBO.h"

#include "AntOculusAppSkeleton.h"

AntOculusAppSkeleton g_app;

int running = 0;

struct OutputStream {
    GLFWwindow*  pWindow;
    GLFWmonitor* pMonitor;
    OVRkill::DisplayMode outtype;
};

std::vector<OutputStream> g_outStreams;

Timer g_timer;


void CycleOutputType(OutputStream& os)
{
    if (os.outtype == OVRkill::SingleEye)
    {
        os.outtype = OVRkill::Stereo;
    }
    else if (os.outtype == OVRkill::Stereo)
    {
        os.outtype = OVRkill::StereoWithDistortion;
    }
    else
    {
        os.outtype = OVRkill::SingleEye;
    }
}

void display()
{
    int i=0;
    for (std::vector<OutputStream>::const_iterator it = g_outStreams.begin();
        it != g_outStreams.end();
        ++it, ++i)
    {
        const OutputStream& os = *it;
        GLFWwindow* pWin = os.pWindow;
        glfwMakeContextCurrent(pWin);

        int width, height;
        glfwGetWindowSize(pWin, &width, &height);

        glViewport(0,0, width, height);
        g_app.display(i==0, os.outtype);
        glfwSwapBuffers(pWin);
    }


    // Display hack - indicate that a draw(of a black screen) has taken place so we are free
    // to go unresponsive for a bit while we load and compile a new shader.
    // Next step we can from the timestep function.
    if (g_app.m_loadingShader)
    {
        g_app.m_drawnBlackFrame = true;
    }
}

void mouseDown(GLFWwindow* pWindow, int button, int action, int mods)
{
    double x,y;
    glfwGetCursorPos(pWindow, &x, &y);
    g_app.mouseDown(button, action, (int)x, (int)y);
}

void mouseMove(GLFWwindow* pWindow, double x, double y)
{
    g_app.mouseMove((int)x, (int)y);
}

void mouseWheel(GLFWwindow* pWindow, double x, double y)
{
    g_app.mouseWheel((int)x, (int)y);
}

void handleMonitorConfigurationKeystrokes(int key)
{
    if (g_outStreams.empty())
        return;

    switch (key)
    {
    default: break;

    case GLFW_KEY_F1:
        if (g_outStreams.size() >= 1)
            CycleOutputType(g_outStreams[0]);
        break;

    case GLFW_KEY_F2:
        if (g_outStreams.size() >= 2)
            CycleOutputType(g_outStreams[1]);
        break;

    case GLFW_KEY_F3:
        if (g_outStreams.size() >= 3)
            CycleOutputType(g_outStreams[2]);
        break;
    }
}

void keyboard(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
    switch (key)
    {
    default: break;

    case GLFW_KEY_ESCAPE:
        exit(0);
        break;
    }

    if (action == GLFW_PRESS)
    {
        handleMonitorConfigurationKeystrokes(key);
    }

    g_app.keyboard(key, action, 0,0);
}


void timestep()
{
    float dt = (float)g_timer.seconds();
    g_timer.reset();
    g_app.timestep(dt);
}


///@brief Window resized event callback
void resize(GLFWwindow* pWindow, int w, int h)
{
    g_app.resize(w,h);
    glViewport(0,0,(GLsizei)w, (GLsizei)h);

    ///@note We can mitigate the effect of resizing the control window on the Oculus user
    /// by continuing to track the head in timestep as we resize.
    /// However, when the mouse is held still while resizing, we do not refresh.
    timestep();
    display();
}


///@brief Attempt to determine which of the connected monitors is the Oculus Rift and which
/// are not. The only heuristic available for this purpose is resolution.
void IdentifyMonitors()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    for (int i=0; i<count; ++i)
    {
        GLFWmonitor* pMonitor = monitors[i];
        if (pMonitor == NULL)
            continue;
        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);

        // g_outStreams should be empty at this time
        // The first thing we will do is populate the global vector with
        // partially initialized outputStream structs - monitors only.
        // In initGlfw they will be paired with monitors.
        OutputStream os = {0};
        os.pMonitor = pMonitor;
        g_outStreams.push_back(os);
    }
}


///@brief Dump a list of monitor info to Log and stdout.
/// http://www.glfw.org/docs/3.0/monitor.html
void PrintMonitorInfo()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    printf("Found %d monitors:\n", count);
    LOG_INFO("Found %d monitors:", count);
    for (int i=0; i<count; ++i)
    {
        GLFWmonitor* pMonitor = monitors[i];
        if (pMonitor == NULL)
            continue;
        printf("  Monitor %d:\n", i);
        LOG_INFO("  Monitor %d:", i);

        /// Monitor name
        const char* pName = glfwGetMonitorName(pMonitor);
        printf("    Name: %s\n", pName);
        LOG_INFO("    Name: %s", pName);

        /// Monitor Physical Size
        int widthMM, heightMM;
        glfwGetMonitorPhysicalSize(pMonitor, &widthMM, &heightMM);
        //const double dpi = mode->width / (widthMM / 25.4);
        printf("    physical size: %d x %d mm\n", widthMM, heightMM);
        LOG_INFO("    physical size: %d x %d mm", widthMM, heightMM);

        /// Modes
        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);
        printf("    Current mode:  %dx%d @ %dHz (RGB %d %d %d bits)\n",
                mode->width,
                mode->height,
                mode->refreshRate,
                mode->redBits,
                mode->greenBits, 
                mode->blueBits);
        LOG_INFO("    Current mode:  %dx%d @ %dHz (RGB %d %d %d bits)",
                mode->width,
                mode->height,
                mode->refreshRate,
                mode->redBits,
                mode->greenBits, 
                mode->blueBits);

#if 0
        int modeCount;
        const GLFWvidmode* modes = glfwGetVideoModes(pMonitor, &modeCount);
        printf("    %d Modes:\n", modeCount);
        LOG_INFO("    %d Modes:", modeCount);
        for (int j=0; j<modeCount; ++j)
        {
            const GLFWvidmode& m = modes[j];
            printf("      %dx%d @ %dHz (RGB %d %d %d bits)\n",
                m.width, m.height, m.refreshRate, m.redBits, m.greenBits, m.blueBits);
            LOG_INFO("      %dx%d @ %dHz (RGB %d %d %d bits)",
                m.width, m.height, m.refreshRate, m.redBits, m.greenBits, m.blueBits);
        }
#endif
    }
}

// Init Control window containing AntTweakBar
GLFWwindow* InitControlWindow()
{
    GLFWwindow* pControlWindow = glfwCreateWindow(g_app.w(), g_app.h(), "Control Window", NULL, NULL);
    if (!pControlWindow)
    {
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(pControlWindow);

    glfwSetWindowSizeCallback (pControlWindow, resize);
    glfwSetMouseButtonCallback(pControlWindow, mouseDown);
    glfwSetCursorPosCallback  (pControlWindow, mouseMove);
    glfwSetScrollCallback     (pControlWindow, mouseWheel);
    glfwSetKeyCallback        (pControlWindow, keyboard);
    //glfwSetCharCallback       (pControlWindow, charkey);

    return pControlWindow;
}



static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool initGlfw(int argc, char **argv, bool fullScreen)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return false;

    PrintMonitorInfo();
    IdentifyMonitors();

    //LOG_INFO("Initializing Glfw and window @ %d x %d", m_windowWidth, m_windowHeight);

    ///@todo We are making some assumptions about vector indices here; be more general
    // Let's assume the first monitor in the list goes to the control window for now.
    GLFWwindow* pControlWindow = InitControlWindow();
    g_outStreams[0].pWindow = pControlWindow;

    // Init secondary window
    if (g_outStreams.size() > 1)
    {
        GLFWwindow* pOculusWindow = glfwCreateWindow(
            g_app.GetOculusWidth(), g_app.GetOculusHeight(),
            "Oculus Window",
            ///@note Fullscreen windows cannot be positioned. The fullscreen window over the Oculus
            /// monitor would be the preferred solution, but on Windows that fullscreen window will disappear
            /// on the first mouse input occuring outside of it, defeating the purpose of the first window.
            NULL, //g_pOculusMonitor,
            pControlWindow); // All rift windows will share with control

        if (!pOculusWindow)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(pOculusWindow);

        /// Position Oculus secondary monitor pseudo-fullscreen window
        if (g_outStreams[0].pMonitor != NULL)
        {
            const GLFWvidmode* pMode = glfwGetVideoMode(g_outStreams[0].pMonitor);
            if (pMode != NULL)
            {
                glfwSetWindowPos(pOculusWindow, pMode->width, 0);
            }
        }

        glfwSetKeyCallback(pOculusWindow, keyboard);
        glfwShowWindow(pOculusWindow);

        g_outStreams[1].pWindow = pOculusWindow;
        g_outStreams[1].outtype = OVRkill::StereoWithDistortion;
    }
    else if (!g_outStreams.empty())
    {
        // Only one screen found, set our single window to Oculus mode and maximize.
        glfwSetWindowPos(pControlWindow, 0, 0);
        g_outStreams[0].outtype = OVRkill::StereoWithDistortion;

        const GLFWvidmode* pMode = glfwGetVideoMode(g_outStreams[0].pMonitor);
        if (pMode != NULL)
        {
            glfwSetWindowSize(pControlWindow, pMode->width, pMode->height);
        }
    }
    else
    {
        LOG_INFO("Completely out of luck as no monitors have been found.");
    }

    /// If we are not sharing contexts between windows, make the appropriate one current here.
    //glfwMakeContextCurrent(g_pControlWindow);

    return true;
}



/// Initialize then enter the main loop
int main(int argc, char *argv[])
{
    bool fullScreen = false;

    // Call initVR before initGL to get recommended size for our FBO distortion buffer
    g_app.initVR(fullScreen);

    initGlfw(argc, argv, fullScreen);

    g_app.initGL(argc, argv);
    g_app.initJoysticks();

    // We can only set attributes after the TweakBar has been initialized.
    if (g_outStreams.size() == 1)
    {
        g_app.MinimizeTweakbar();
    }

    /// Main loop
    running = GL_TRUE;
    while (running)
    {
        timestep();
        g_app.frameStart();
        display();
        glfwPollEvents();

        for (std::vector<OutputStream>::const_iterator it = g_outStreams.begin();
            it != g_outStreams.end();
            ++it)
        {
            const OutputStream& os = *it;
            if (glfwWindowShouldClose(os.pWindow))
                running = GL_FALSE;
        }
    }

    return 0;
}

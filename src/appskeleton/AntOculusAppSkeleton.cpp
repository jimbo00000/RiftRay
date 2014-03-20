// AntOculusAppSkeleton.cpp

#include "AntOculusAppSkeleton.h"
#include "MatrixMath.h"
#include "GL/ShaderFunctions.h"

#include <sstream>

AntOculusAppSkeleton::AntOculusAppSkeleton()
: m_timer()
, m_fps(0.0f)
#ifdef USE_ANTTWEAKBAR
, m_pBar(NULL)
#endif
{
}

AntOculusAppSkeleton::~AntOculusAppSkeleton()
{
    ///@todo Delete this before glfw
    //delete m_pBar;
}

#ifdef USE_ANTTWEAKBAR

static void TW_CALL ResetDistortionParams(void *clientData)
{
    if (clientData)
    {
        RiftDistortionParams* pP = static_cast<RiftDistortionParams *>(clientData);
        RiftDistortionParams reset;
        memcpy(pP, &reset, sizeof(RiftDistortionParams));
    }
}

static void TW_CALL GetDistortionFboWidth(void *value, void *clientData)
{
    *static_cast<int *>(value) = static_cast<const OVRkill *>(clientData)->GetRenderBufferWidth();
}

static void TW_CALL GetDistortionFboHeight(void *value, void *clientData)
{
    *static_cast<int *>(value) = static_cast<const OVRkill *>(clientData)->GetRenderBufferHeight();
}

static void TW_CALL ReallocateDistortionFbo(void *clientData)
{
    if (clientData)
    {
        static_cast<AntOculusAppSkeleton *>(clientData)->ResizeFbo();
    }
}

static void TW_CALL SetBufferScaleCallback(const void *value, void *clientData)
{
    if (clientData)
    {
        static_cast<AntOculusAppSkeleton *>(clientData)->SetBufferScaleUp( *(float *)value);
        static_cast<AntOculusAppSkeleton *>(clientData)->ResizeFbo();
    }
}

static void TW_CALL GetBufferScaleCallback(void *value, void *clientData)
{
    if (clientData)
    {
        *(float *)value = static_cast<AntOculusAppSkeleton *>(clientData)->GetBufferScaleUp();
    }
}

static void TW_CALL GetMegaPxCount(void *value, void *clientData)
{
    *static_cast<float *>(value) = static_cast<const AntOculusAppSkeleton *>(clientData)->GetMegaPixelCount();
}

static void TW_CALL GetMegaPxPerSecond(void *value, void *clientData)
{
    *static_cast<float *>(value) = static_cast<const AntOculusAppSkeleton *>(clientData)->GetMegaPixelsPerSecond();
}

static void TW_CALL PrevShaderCB(void *clientData)
{
    static_cast<AntOculusAppSkeleton *>(clientData)->PrevShader();
}

static void TW_CALL NextShaderCB(void *clientData)
{
    static_cast<AntOculusAppSkeleton *>(clientData)->NextShader();
}

static void TW_CALL ResetTimerCB(void *clientData)
{
    static_cast<AntOculusAppSkeleton *>(clientData)->ResetSceneTimer();
}

static void TW_CALL GoToURLCB(void *clientData)
{
    const AntOculusAppSkeleton* pApp = static_cast<AntOculusAppSkeleton *>(clientData);
    if (!pApp)
        return;
    const std::map<std::string, std::string>& varMap = pApp->GetVarMap();

    if (varMap.find("url") == varMap.end())
        return;

    const std::string url = varMap.at("url");

#ifdef _WIN32
    ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif _LINUX
    system("x-www-browser url.c_str()");
#elif __APPLE__
    std::string command = "open ";
    command += url;
    system(command.c_str());
#endif
}


///@param state 0=off, 1=on
static void SetVsync(int state)
{
    glfwSwapInterval(state);
}

static void TW_CALL EnableVSyncCB(void *clientData)
{
    SetVsync(1);
}

static void TW_CALL DisableVSyncCB(void *clientData)
{
    SetVsync(0);
}

static void TW_CALL AdaptiveVSyncCB(void *clientData)
{
    SetVsync(-1);
}

void AntOculusAppSkeleton::_InitializeBar()
{
    ///@note Bad size errors will be thrown if this is not called at init
    TwWindowSize(m_windowWidth, m_windowHeight);

    // Create a tweak bar
    m_pBar = TwNewBar("TweakBar");
    TwDefine(" TweakBar refresh=0.1 ");
    TwDefine(" TweakBar fontsize=3 ");
    TwDefine(" TweakBar size='240 600' ");
    TwDefine(" TweakBar valueswidth=120 ");

    //
    // Performance Group
    //
    TwAddVarRO(m_pBar, "fps", TW_TYPE_FLOAT, &m_fps, 
               " label='fps' precision=0 group='Performance' ");

    TwAddVarCB(m_pBar, "FBO w", TW_TYPE_INT32, NULL, GetDistortionFboWidth, &m_ok,
        "precision=0 group='Performance' ");
    TwAddVarCB(m_pBar, "FBO h", TW_TYPE_INT32, NULL, GetDistortionFboHeight, &m_ok,
        "precision=0 group='Performance' ");
    TwAddVarCB(m_pBar, "MPx", TW_TYPE_FLOAT, NULL, GetMegaPxCount, this,
        "precision=2 group='Performance' ");
    TwAddVarCB(m_pBar, "MPx/sec", TW_TYPE_FLOAT, NULL, GetMegaPxPerSecond, this,
        "precision=0 group='Performance' ");

    TwAddVarCB(m_pBar, "FBO x", TW_TYPE_FLOAT,
        SetBufferScaleCallback, GetBufferScaleCallback, this,
        " min=0.125 max=16.0 step=0.01 group='Performance' ");

    TwAddVarRW(m_pBar, "FBO gutter", TW_TYPE_FLOAT, &m_bufferGutterPctg,
        " min=0.0 max=0.495 step=0.01 group='Performance' ");

    TwAddVarRW(m_pBar, "Gutter Focus", TW_TYPE_FLOAT, &m_bufferGutterFocus,
        " min=0.0 max=2.0 step=0.01 group='Performance' ");


    TwAddVarRW(m_pBar, "Display", TW_TYPE_BOOLCPP, &m_displaySceneInControl,
               " group='Performance' ");

    TwAddButton(m_pBar, "Disable VSync", DisableVSyncCB, NULL, " group='Performance' ");
    TwAddButton(m_pBar, "Enable VSync", EnableVSyncCB, NULL, " group='Performance' ");
    TwAddButton(m_pBar, "Adaptive VSync", AdaptiveVSyncCB, NULL, " group='Performance' ");

    //
    // HMD params passed to OVR Post Process Distortion shader
    //
    TwAddVarRW(m_pBar, "lensOff", TW_TYPE_FLOAT, &m_riftDist.lensOff,
               " label='lensOff'     min=0 max=0.1 step=0.001 group=HMD ");
    TwAddVarRW(m_pBar, "LensCenterX", TW_TYPE_FLOAT, &m_riftDist.LensCenterX,
               " label='LensCenterX' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "LensCenterY", TW_TYPE_FLOAT, &m_riftDist.LensCenterY,
               " label='LensCenterY' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScreenCenterX", TW_TYPE_FLOAT, &m_riftDist.ScreenCenterX,
               " label='ScreenCenterX' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScreenCenterY", TW_TYPE_FLOAT, &m_riftDist.ScreenCenterY,
               " label='ScreenCenterY' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleX", TW_TYPE_FLOAT, &m_riftDist.ScaleX,
               " label='ScaleX' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleY", TW_TYPE_FLOAT, &m_riftDist.ScaleY,
               " label='ScaleY' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleInX", TW_TYPE_FLOAT, &m_riftDist.ScaleInX,
               " label='ScaleInX' min=0 max=10.0 step=0.1 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleInY", TW_TYPE_FLOAT, &m_riftDist.ScaleInY,
               " label='ScaleInY' min=0 max=10.0 step=0.1 group=HMD ");
    TwAddVarRW(m_pBar, "DistScale", TW_TYPE_FLOAT, &m_riftDist.DistScale,
               " label='DistScale' min=0 max=3.0 step=0.01 group=HMD ");

    TwAddButton(m_pBar, "ResetDistortionParams", ResetDistortionParams, &m_riftDist,
        " label='ResetDistortionParams' group='HMD' ");

    TwAddVarRW(m_pBar, "m_flattenStereo", TW_TYPE_BOOLCPP, &m_flattenStereo,
               " label='m_flattenStereo' group=HMD ");

    TwAddVarRW(m_pBar, "m_headSize", TW_TYPE_FLOAT, &m_headSize,
               " label='Head Size' min=0.0001 max=10.0 step=0.001 group=HMD ");



    //
    // Camera params
    //
    TwAddVarRW(m_pBar, "followcam.x", TW_TYPE_FLOAT, &FollowCamDisplacement.x,
               " label='followcam.x' min=-30 max=30 step=0.01 group=Camera ");
    TwAddVarRW(m_pBar, "followcam.y", TW_TYPE_FLOAT, &FollowCamDisplacement.y,
               " label='followcam.y' min=-30 max=30 step=0.01 group=Camera ");
    TwAddVarRW(m_pBar, "followcam.z", TW_TYPE_FLOAT, &FollowCamDisplacement.z,
               " label='followcam.z' min=-30 max=30 step=0.01 group=Camera ");

    TwAddVarRW(m_pBar, "viewAngle", TW_TYPE_FLOAT, &m_viewAngleDeg,
               " label='viewAngle' min=30 max=90 step=0.1 group=Camera ");



    //
    // Scene parameters
    //
    // Trying to pass this in via CB and the _STDSTRING type led to access violations.
    TwAddVarRO(m_pBar,  "Shader: ", TW_TYPE_STDSTRING, &m_scene.m_curShaderName,
               " group='Scene' ");

    TwAddButton(m_pBar, "Prev Shader", PrevShaderCB, this,
        " label='Prev Shader' group='Scene' ");
    TwAddButton(m_pBar, "Next Shader", NextShaderCB, this,
        " label='Next Shader' group='Scene' ");
    TwAddButton(m_pBar, "Reset Timer", ResetTimerCB, this,
        " label='Reset Timer' group='Scene' ");

    TwAddVarRW(m_pBar, "Eye center", TW_TYPE_FLOAT, &m_scene.m_eyeballCenterTweak, 
               " label='Eye center' min=-1 max=1 step=0.001 group=Scene ");
    TwAddVarRW(m_pBar, "Scale", TW_TYPE_FLOAT, &m_scene.m_cubeScale, 
               " label='Scale' min=0.01 max=4 step=0.01 group=Scene ");

    TwAddVarRW(m_pBar, "head Size", TW_TYPE_FLOAT, &m_headSize, 
               " label='head Size' min=0.0001 step=0.01 group='Scene' ");
    TwAddVarRW(m_pBar, "EyePos.x", TW_TYPE_FLOAT, &EyePos.x, 
               " label='EyePos.x'  step=0.001 group='Scene' ");
    TwAddVarRW(m_pBar, "EyePos.y", TW_TYPE_FLOAT, &EyePos.y, 
               " label='EyePos.y'  step=0.001 group='Scene' ");
    TwAddVarRW(m_pBar, "EyePos.z", TW_TYPE_FLOAT, &EyePos.z, 
               " label='EyePos.z'  step=0.001 group='Scene' ");

    int opened = 0;
    TwSetParam(m_pBar, "HMD", "opened", TW_PARAM_INT32, 1, &opened);
    TwSetParam(m_pBar, "Camera", "opened", TW_PARAM_INT32, 1, &opened);
}

void AntOculusAppSkeleton::MinimizeTweakbar()
{
    TwDefine(" TweakBar iconified=true ");
}

#endif

#ifdef USE_ANTTWEAKBAR
// Skip over the variables we handle explicitly in Scene class as they don't need display.
bool isSceneVariable(const std::string& str)
{
    if (!str.compare("headSize"))
        return true;
    if (!str.compare("eyePos"))
        return true;
    if (!str.compare(0, 3, "tex"))
        return true;

    return false;
}

///@note It seems we are really going against the grain here with AntTweakBar and reaching
/// the limits of its capability with regards to strings. I had a hell of a time messing with
/// the TwCopyStdStringToClientFunc function and TW_TYPE_STDSTRING callbacks, always getting
/// access violation errors. Adding a NULL button per string variable was the only solution
/// I could get working to get the author and license strings in the main tweakbar. Putting
/// that info in the help string of the global help bar was the fallback option, but this
/// seemed cleaner by not having to show another bar.
void AntOculusAppSkeleton::GetShaderParams()
{
    // First delete all the old fields
    std::map<std::string, std::string>::iterator iter;
    for (iter = m_varMap.begin(); iter != m_varMap.end(); ++iter)
    {
        if (isSceneVariable(iter->first))
            continue;

        TwRemoveVar( m_pBar, iter->first.c_str() );
    }

    // Get the @var parameter lines from the shader code
    OculusAppSkeleton::GetShaderParams();

    // Add in all strings from the shader @var decls to a NULL action button in group Scene
    for (iter = m_varMap.begin(); iter != m_varMap.end(); ++iter)
    {
        if (isSceneVariable(iter->first))
            continue;

        if (!iter->first.compare("url"))
        {
            //GoToURLCB
            TwAddButton( m_pBar, iter->first.c_str(), GoToURLCB, this,
                " label='Go to URL'  group='Scene' ");
        }
        else
        {
            std::stringstream ss;
            // Assemble a string to pass into help here
            ss << " label='";
            ss  << iter->first
                << ": "
                << iter->second;
            ss << "' group=Scene ";

            TwAddButton( m_pBar, iter->first.c_str(), NULL, NULL, ss.str().c_str());
        }
    }
}
#endif

bool AntOculusAppSkeleton::initGL(int argc, char **argv)
{
#ifdef USE_ANTTWEAKBAR
    TwInit(TW_OPENGL, NULL);
    _InitializeBar();
#endif
    return OculusAppSkeleton::initGL(argc, argv);
}

void DrawScreenRect(const rect& r)
{
    const int verts[] = {
        r.x    , r.y,
        r.x+r.w, r.y,
        r.x+r.w, r.y+r.h,
        r.x    , r.y+r.h,
    };
    const float2 texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    const uint3 quads[] = {
        {0,3,2}, {1,0,2}, // ccw
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texs);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES,
                   3*2, // 2 triangle pairs
                   GL_UNSIGNED_INT,
                   &quads[0]);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

///@todo This function might better reside elsewhere - in scene?
void AntOculusAppSkeleton::displayTexSampler() const
{
    glViewport(0,0,w(),h());
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    const GLuint prog = m_basictexProg;
    glUseProgram(prog);
    {
        float mvmtx[16];
        float prmtx[16];
        MakeIdentityMatrix(mvmtx);
        glhOrtho(prmtx,
              0, w(),
              0, h(),
              -1.0f, 1.0f);

        glUniformMatrix4fv(getUniLoc(prog, "mvmtx"), 1, false, mvmtx);
        glUniformMatrix4fv(getUniLoc(prog, "prmtx"), 1, false, prmtx);

        const char* pUnis[] = {
            "iChannel0",
            "iChannel1",
            "iChannel2",
            "iChannel3",
        };
        const GLuint texs[] = {
            m_scene.m_texChan0,
            m_scene.m_texChan1,
            m_scene.m_texChan2,
            m_scene.m_texChan3,
        };

        int rw = 100;
        int xoff = 20;
        for (int i=0; i<4; ++i)
        {
            const GLint samp0UniLoc = glGetUniformLocation(prog, pUnis[0]);
            const GLuint tex = texs[i];
            if ((samp0UniLoc != -1) && (tex != 0))
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex);
                glUniform1i(samp0UniLoc, 0);

                const rect r(20+i*(rw+10),20, rw,rw);
                DrawScreenRect(r);
            }
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glUseProgram(0);
}

void AntOculusAppSkeleton::display(bool isControl, OVRkill::DisplayMode mode)
{
    OculusAppSkeleton::display(isControl, mode);

#ifdef USE_ANTTWEAKBAR
    // Displaying the maximized tweakbar is surprisingly not painfully obstrusive
    // on a mirrored 1920x1080. It can also be minimized.
    if (isControl)
    {
        displayTexSampler();
        TwRefreshBar(m_pBar);
        TwDraw(); ///@todo Should this go first? Will it write to a depth buffer?
    }
#endif
}

void AntOculusAppSkeleton::mouseDown(int button, int state, int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventMouseButtonGLFW(button, state);
    if (ant != 0)
        return;
#endif
    OculusAppSkeleton::mouseDown(button, state, x, y);
}

void AntOculusAppSkeleton::mouseMove(int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    TwEventMousePosGLFW(x, y);
#endif
    OculusAppSkeleton::mouseMove(x, y);
}

void AntOculusAppSkeleton::mouseWheel(int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    TwEventMouseWheelGLFW(x);
#endif
    OculusAppSkeleton::mouseWheel(x, y);
}


void AntOculusAppSkeleton::keyboard(int key, int action, int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventKeyGLFW(key, action);
    if (ant != 0)
        return;
#endif

    OculusAppSkeleton::keyboard(key, action, x, y);
}

void AntOculusAppSkeleton::charkey(unsigned int key)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventCharGLFW(key, 0);
    if (ant != 0)
        return;
#endif
   // OculusAppSkeleton::keyboard(key, 0, 0);
}

void AntOculusAppSkeleton::resize(int w, int h)
{
#ifdef USE_ANTTWEAKBAR
    TwWindowSize(m_windowWidth, m_windowHeight);
#endif
    OculusAppSkeleton::resize(w,h);
}

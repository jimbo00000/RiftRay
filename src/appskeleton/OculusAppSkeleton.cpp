// OculusAppSkeleton.cpp

#include "OculusAppSkeleton.h"
#include "VectorMath.h"

#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <string>

#include "Draw_Helpers.h"
#include "GL/ShaderFunctions.h"
#include "Logger.h"
#include "string_utils.h"

OculusAppSkeleton::OculusAppSkeleton()
: AppSkeleton()
, m_loadingShader(false)
, m_drawnBlackFrame(false)
// The world RHS coordinate system is defines as follows (as seen in perspective view):
//  Y - Up
//  Z - Back
//  X - Right
, UpVector(0.0f, 1.0f, 0.0f)
, ForwardVector(0.0f, 0.0f, -1.0f)
, RightVector(1.0f, 0.0f, 0.0f)
, YawInitial(3.141592f) /// We start out looking in the positive Z (180 degree rotation).
, Sensitivity(1.0f)
, MoveSpeed(3.0f) // m/s
, m_standingHeight(1.78f) /// From Oculus SDK p.13: 1.78m ~= 5'10"
, m_crouchingHeight(0.6f)
, EyePos(0.0f, m_standingHeight, -5.0f)
, EyeYaw(YawInitial)
, EyePitch(0)
, EyeRoll(0)
, LastSensorYaw(0)
, FollowCamDisplacement(0.0f, 0.0f, 0.01f)
, FollowCamPos(EyePos + FollowCamDisplacement)
, m_viewAngleDeg(45.0) ///< For the no HMD case
, preferredGamepadID(0)
, swapGamepadRAxes(false)
, lastGamepadButtons()
, which_button(-1)
, modifier_mode(0)
, m_ok()
, m_riftDist()
, m_bufferScaleUp(1.0f)
, m_bufferBoost(1.0f)
, m_bufferGutterPctg(0.0f)
, m_bufferGutterFocus(0.8f) ///< Set empirically on my DK1 and 64mm IPD
, m_flattenStereo(false)
, m_headSize(1.0f)
, m_scene()
, m_avatarProg(0)
, m_basictexProg(0)
, m_displaySceneInControl(true)
, m_varMap()
{
    memset(m_keyStates, 0, GLFW_KEY_LAST*sizeof(int));
}

OculusAppSkeleton::~OculusAppSkeleton()
{
    glDeleteProgram(m_avatarProg);
    glDeleteProgram(m_basictexProg);
    m_ok.DestroyOVR();
    glfwTerminate();
}

bool OculusAppSkeleton::initVR(bool fullScreen)
{
    m_ok.InitOVR();
    m_ok.SetDisplayMode(OVRkill::StereoWithDistortion);
    m_bufferScaleUp = 0.25f; // m_ok.GetRenderBufferScaleIncrease();

    return true;
}

/// Take into account the FBO gutters which cull edge pixels on each half of the FBO
/// using glScissor. See DrawScene.
float OculusAppSkeleton::GetMegaPixelCount() const
{
    const int fboWidth = m_ok.GetRenderBufferWidth();
    const int fboHeight = m_ok.GetRenderBufferHeight();

    const int halfWidth = fboWidth/2;
    const int halfHeight = fboHeight/2;
    const int gutterPxW = (int)(m_bufferGutterPctg * (float)halfWidth);
    const int gutterPxH = (int)(m_bufferGutterPctg * (float)halfHeight);
    const int widthGutter  = halfWidth - 2*gutterPxW;
    const int heightGutter = fboHeight - 2*gutterPxH;
    const float px = (float)widthGutter * (float)heightGutter
        * (m_displaySceneInControl ? 2.0f : 1.0f);
    return px / (float)(1024*1024);
}

void OculusAppSkeleton::ResizeFbo()
{
    m_ok.CreateRenderBuffer(m_bufferScaleUp * m_bufferBoost);
}

bool OculusAppSkeleton::initGL(int argc, char **argv)
{
    bool ret = AppSkeleton::initGL(argc, argv); /// calls _InitShaders

    LOG_INFO("Initializing skeleton shaders.");
    {
        m_avatarProg = makeShaderByName("avatar");
        m_basictexProg = makeShaderByName("basictex");
    }
    m_ok.CreateShaders();
    m_ok.CreateRenderBuffer(m_bufferScaleUp);

    LOG_INFO("Initializing scene shaders.");
    m_scene.initGL();
    SetupCurrentShader();

    return ret;
}

///@brief Load any pertinent variables embedded in comments in the source
/// and apply whatever variables they dicate to the VR avatar/scene.
///@todo We are loading the shader source twice - just load it once.
void OculusAppSkeleton::GetShaderParams()
{
    m_varMap.clear();

    const std::string& current = m_scene.GetCurrentShaderName();
    // Prepend a known directory where shaders reside
    ///@todo More sophisticated directory support
    const std::string shaderDir = "../shaders/shadertoy/";
    const std::string toy = shaderDir + current;

    std::ifstream file;
    file.open(toy.c_str(), std::ios::in);
    if (file.is_open())
    {
        std::string str;
        std::vector <std::string> lines;
        while (std::getline(file,str))
        {
            // Look through lines for variable decls
            const std::string needle = "@var "; //< Include the trailing space
            std::size_t found = str.find(needle);
            if (found != std::string::npos)
            {
                // Strip trailing whitespace
                const std::string stripped = trim(str);
                // Keep only the part after the magic decl string
                std::string vardecl = stripped.substr(found + needle.length());
                lines.push_back(vardecl);

                // Push {name,value} pair to hash map
                std::vector<std::string> tokens = split(vardecl, ' ');
                if (tokens.size() >= 2)
                {
                    const std::string& name = tokens[0];
                    const std::string value = vardecl.substr(name.length());
                    m_varMap[name] = value;
                }
            }
        }

        // Apply indicated variables
        std::vector<std::string> textureFilenames;
        for (std::vector<std::string>::iterator it = lines.begin();
            it != lines.end();
            ++it)
        {
            std::string l = *it;
            std::vector<std::string> tokens = split(l, ' ');
            if (tokens.size() < 2)
                continue;
            if (tokens[0].compare("headSize") == 0)
            {
                // Parse out float
                const std::string& valstr = tokens[1];
                double val = strtod(valstr.c_str(), NULL);
                m_headSize = float(val);
                printf("headSize: %f\n", m_headSize);
            }
            else if (tokens[0].compare("eyePos") == 0)
            {
                //@var eyePos 0.22735068 1.5425634 -1.3010134
                if (tokens.size() > 3)
                {
                    OVR::Vector3f val;
                    val.x = (float)strtod(tokens[1].c_str(), NULL);
                    val.y = (float)strtod(tokens[2].c_str(), NULL);
                    val.z = (float)strtod(tokens[3].c_str(), NULL);
                    EyePos = val;
                    printf("eyePos: %f %f %f\n", EyePos.x, EyePos.y, EyePos.z);
                }
            }
            else if (
                ///@todo Proper index handling here
                (tokens[0].compare("tex0") == 0) ||
                (tokens[0].compare("tex1") == 0) ||
                (tokens[0].compare("tex2") == 0) ||
                (tokens[0].compare("tex3") == 0)
                )
            {
                if (tokens.size() >= 2)
                {
                    textureFilenames.push_back(tokens[1]);
                    printf("Texture %s: [%s]\n", tokens[0].c_str(), tokens[1].c_str());
                }
            }
        }
        file.close();

        m_scene.LoadTextures(textureFilenames);
    }
}

///@brief Check out what joysticks we have and select a preferred one
bool OculusAppSkeleton::initJoysticks()
{
    for (int i=GLFW_JOYSTICK_1; i<GLFW_JOYSTICK_16; ++i)
    {
        const int present = glfwJoystickPresent(i);
        if (present == GL_TRUE)
        {
            /// Nostromo:                   6 axes, 24 buttons
            /// Gravis Gamepad Pro:         2 axes, 10 buttons
            /// Generic wireless dualshock: 4 axes, 12 buttons
            /// Eliminator Aftershock:      6 axes, 10 buttons
            int numAxes = 0;
            int numButs = 0;
            glfwGetJoystickAxes(i, &numAxes);
            glfwGetJoystickButtons(i, &numButs);
            printf("Joystick %d:  %d axes, %d buttons\n", i, numAxes, numButs);

            /// This just finds the first available joystick.
            if ( (numAxes == 2) && (numButs == 10))
            {
                preferredGamepadID = i;
                swapGamepadRAxes = false;
            }
            else if ( (numAxes == 6) && (numButs == 10))
            {
                preferredGamepadID = i;
                swapGamepadRAxes = false;
            }
            else if ( (numAxes == 4) && (numButs == 12))
            {
                preferredGamepadID = i;
                swapGamepadRAxes = true;
            }
        }
    }
    return true;
}

// Utility function for detecting gamepad button presses.
bool GamepadButtonJustPressed(
    std::vector<unsigned char>& lastGamepadButtons,
    const unsigned char* joy1but,
    int but)
{
    bool wasPressed = false;
    if ((int)lastGamepadButtons.size() > but)
    {
        wasPressed = lastGamepadButtons[but] == GLFW_PRESS;
        if ((joy1but[but] == GLFW_PRESS) && (!wasPressed))
            return true;
    }
    return false;
}

// Get hydra state from Scene and check it.
void OculusAppSkeleton::HandleHydra()
{
#ifdef USE_HYDRA
    const FlyingMouse& g_fm = m_scene.GetFlyingMouseState();
    if (!g_fm.IsActive())
        return;

    if (g_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_START))
    {
        NextShader();
    }
    else if (g_fm.WasJustPressed(FlyingMouse::Left, SIXENSE_BUTTON_START))
    {
        PrevShader();
    }

    if (g_fm.WasJustPressed(FlyingMouse::Left, SIXENSE_BUTTON_1))
    {
        m_displaySceneInControl = !m_displaySceneInControl;
    }

    if (g_fm.WasJustPressed(FlyingMouse::Left, SIXENSE_BUTTON_JOYSTICK))
    {
        SetBufferScaleUp(0.25f);
        ResizeFbo();
    }

    // L Joystick trigger for increased resolution
    const sixenseControllerData& cd = g_fm.GetCurrentState().controllers[FlyingMouse::Left];
    const float joyx = cd.joystick_x;
    const float joyy = cd.joystick_y;
    const float trigL = cd.trigger;

    // Incremental adjustment by pushing stick up/down
    {
        const float curScaleUp = GetBufferScaleUp();
        const float incr = pow(2.0f, 1.0f/12.0f);
        const float joyPush = 0.95f * joyy;
        float newval = curScaleUp * pow(incr, joyPush);
        newval = std::max(0.25f, newval);
        newval = std::min(8.0f, newval);
        if (fabs(newval - curScaleUp) > 0.01f)
        {
            SetBufferScaleUp(newval);
            ResizeFbo();
        }
    }

    {
        // Boost resolution while culling enough gutter width to push the
        // same number of MPx.
        const float boostFactor = std::max(0.0f, trigL);
        if (boostFactor > 0.0f)
        {
            const float curScaleUp = GetBufferScaleUp();
            const float curBoost = m_bufferBoost;
            const float MPx = GetMegaPixelCount();
            // (1.0-gutter)^2 * FBO = MPx
            const float maxPctg = 0.45f;
            const float unculledPctg = 1.0f - boostFactor*maxPctg;
            const float unculledRatio2 = unculledPctg * unculledPctg;

            m_bufferBoost = 1.0f / unculledRatio2;
            m_bufferGutterPctg = maxPctg * boostFactor;

            if (fabs(m_bufferBoost - curBoost) > 0.01f)
            {
                ResizeFbo();
            }
        }
    }

#if 0
    // L Joystick vertical for gutter scale
    {
        // The last 0.15 of the scale is dead = so let's clamp it off for smoother action.
        const float minStickVal = -0.85f;
        const float clampedJoyY = std::max(minStickVal, joyy) / minStickVal;
        const float maxScale = 0.45f;

        m_bufferGutterPctg = maxScale * std::max(0.0f, pow(clampedJoyY, 1.0f/3.0f));
        m_bufferGutterPctg = std::max(0.0f, m_bufferGutterPctg);
        m_bufferGutterPctg = std::min(maxScale, m_bufferGutterPctg);
    }
#endif

    // Right stick for movement
    const sixenseControllerData& cdR = g_fm.GetCurrentState().controllers[FlyingMouse::Right];
    const float moveBoost = 1.0f + (30.0f-1.0f)*pow(cdR.trigger,3.0f);
    const float joyRx = moveBoost * cdR.joystick_x;
    const float joyRy = moveBoost * cdR.joystick_y;
    HydraMove = OVR::Vector3f(0,0,0);
    if (g_fm.IsPressed(FlyingMouse::Right, SIXENSE_BUTTON_BUMPER))
    {
        // Transform move vector by right hand matrix if bumper held
        const float* pH = g_fm.mtxR;
        const OVR::Matrix4f hydraMtx(
            pH[0], pH[4], pH[8],
            pH[1], pH[5], pH[9],
            pH[2], pH[6], pH[10]
        );
        OVR::Vector3f move = OVR::Vector3f(joyRx, 0, -joyRy);
        move = hydraMtx.Transform(move);
        HydraMove += move;
    }
    else
    {
        HydraMove += OVR::Vector3f(joyRx, 0, -joyRy);
    }
#endif
}

/// HandleGlfwJoystick - translates joystick states into movement vectors.
void OculusAppSkeleton::HandleGlfwJoystick()
{
    const int joyID = preferredGamepadID;
    int joyStick1Present = GL_FALSE;
    joyStick1Present = glfwJoystickPresent(joyID);
    if (joyStick1Present != GL_TRUE)
        return;

    int numAxes = 0;
    int numButs = 0;
    glfwGetJoystickAxes(joyID, &numAxes);
    glfwGetJoystickButtons(joyID, &numButs);

    std::vector<unsigned char> oldbuts = lastGamepadButtons;

    if (numAxes <= 0)
        return;

    int retAxes = 0;
    const float* joy1pos = glfwGetJoystickAxes(joyID, &retAxes);
    if (retAxes <= 0)
        return;

    int retButs = 0;
    const unsigned char* joy1but = glfwGetJoystickButtons(joyID, &retButs);
    if (retButs <= 0)
        return;

    // Check for pressed buttons
    {
        if (GamepadButtonJustPressed(lastGamepadButtons, joy1but, 8)) // Select
        {
            PrevShader();
        }
        if (GamepadButtonJustPressed(lastGamepadButtons, joy1but, 9)) // Start
        {
            NextShader();
        }
    }

    lastGamepadButtons.resize(retButs);
    memcpy(&lastGamepadButtons[0], joy1but, numButs);

    GamepadMove = OVR::Vector3f(0,0,0);

    float padLx = joy1pos[0];
    float padLy = joy1pos[1];
    float padRx = joy1pos[2];
    float padRy = joy1pos[3];

    if (swapGamepadRAxes)
    {
        float temp = padRx;
        padRx = padRy;
        padRy = temp;
    }

    const float threshold = 0.2f;
    if (fabs(padLx) < threshold)
        padLx = 0.0f;
    if (fabs(padLy) < threshold)
        padLy = 0.0f;
    if (fabs(padRx) < threshold)
        padRx = 0.0f;
    if (fabs(padRy) < threshold)
        padRy = 0.0f;

    GamepadRotate = OVR::Vector3f(2 * padLx, -2 * padLy,  0);
    GamepadMove  += OVR::Vector3f(2 * padRy, 0         , 2 * padRx);

    // Dpad up/down for gutter scale
    {
        const float incr = 0.01f;
        m_bufferGutterPctg += incr * (float)padLy;
        m_bufferGutterPctg = std::max(0.0f, m_bufferGutterPctg);
        m_bufferGutterPctg = std::min(1.0f, m_bufferGutterPctg);
    }

    // next clause
    float joy1buts[4] = {
        joy1but[0] == GLFW_PRESS ? -1.0f : 0.0f,
        joy1but[1] == GLFW_PRESS ? -1.0f : 0.0f,
        joy1but[2] == GLFW_PRESS ? 1.0f : 0.0f,
        joy1but[3] == GLFW_PRESS ? 1.0f : 0.0f,
    };
    if (swapGamepadRAxes)
    {
        float temp = -joy1buts[0];
        joy1buts[0] = -joy1buts[3];
        joy1buts[3] = temp;

        temp = -joy1buts[2];
        joy1buts[2] = -joy1buts[1];
        joy1buts[1] = temp;
    }
    float padLxRadial = joy1buts[0] + joy1buts[2];
    float padLyRadial = joy1buts[1] + joy1buts[3];

    GamepadMove += OVR::Vector3f(padLxRadial * padLxRadial * (padLxRadial > 0 ? 1 : -1),
                                 0,
                                 padLyRadial * padLyRadial * (padLyRadial > 0 ? -1 : 1));

    /// Two right shoulder buttons are [5] and [7] on gravis Gamepad pro
    if (retButs > 7)
    {
#if 0
        /// Top shoulder button rises, bottom lowers
        float joy1shoulderbuts[4] = {
            joy1but[4] == GLFW_PRESS ? 1.0f : 0.0f,
            joy1but[5] == GLFW_PRESS ? 1.0f : 0.0f,
            joy1but[6] == GLFW_PRESS ? -1.0f : 0.0f,
            joy1but[7] == GLFW_PRESS ? -1.0f : 0.0f,
        };
        float padLup   = joy1shoulderbuts[1];
        float padLdown = joy1shoulderbuts[3];
        padLup += padLdown;

        GamepadMove += OVR::Vector3f(0,
                                     padLup * padLup * (padLup > 0 ? 1 : -1),
                                     0);
#endif

        // Use left shoulder buttons for head resizing
        if (joy1but[4] == GLFW_PRESS)
        {
            const float incr = pow(2.0f, 1.0f/5.0f);
            m_headSize *= pow(incr, 0.3f);
            m_headSize = std::max(0.001f, m_headSize);
            m_headSize = std::min(1000.0f, m_headSize);
        }
        else if (joy1but[6] == GLFW_PRESS)
        {
            const float incr = pow(2.0f, 1.0f/5.0f);
            m_headSize *= pow(incr, -0.3f);
            m_headSize = std::max(0.001f, m_headSize);
            m_headSize = std::min(1000.0f, m_headSize);
        }
        
        // Use right shoulder buttons for FBO resizing
        if (joy1but[5] == GLFW_PRESS)
        {
            const float curScaleUp = GetBufferScaleUp();
            const float incr = 1.05946309436f;
            float newval = curScaleUp * pow(incr, (float)-1);
            newval = std::max(0.25f, newval);
            newval = std::min(8.0f, newval);
            SetBufferScaleUp(newval);
            ResizeFbo();
        }
        else if (joy1but[7] == GLFW_PRESS)
        {
            const float curScaleUp = GetBufferScaleUp();
            const float incr = 1.05946309436f;
            float newval = curScaleUp * pow(incr, (float)1);
            newval = std::max(0.25f, newval);
            newval = std::min(8.0f, newval);
            SetBufferScaleUp(newval);
            ResizeFbo();
        }
    }
}


/// Handle WASD keys to move camera
void OculusAppSkeleton::HandleKeyboardMovement()
{
    float mag = 1.0f;
    mag *= m_headSize;
    if (m_keyStates[GLFW_KEY_LEFT_SHIFT ] != GLFW_RELEASE)
        mag *= 0.1f;
    if (m_keyStates[GLFW_KEY_LEFT_CONTROL ] != GLFW_RELEASE)
        mag *= 10.0f;

    /// Handle keyboard movement(WASD keys)
    KeyboardMove = OVR::Vector3f(0.0f, 0.0f, 0.0f);
    if (m_keyStates['W'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(0.0f, 0.0f, -mag);
    }
    if (m_keyStates['S'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(0.0f, 0.0f, mag);
    }
    if (m_keyStates['A'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(-mag, 0.0f, 0.0f);
    }
    if (m_keyStates['D'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(mag, 0.0f, 0.0f);
    }
    if (m_keyStates['Q'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(0.0f, mag, 0.0f);
    }
    if (m_keyStates['E'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(0.0f, -mag, 0.0f);
    }
}


/// Handle input's influence on orientation variables.
void OculusAppSkeleton::AccumulateInputs(float dt)
{
    // Handle Sensor motion.
    // We extract Yaw, Pitch, Roll instead of directly using the orientation
    // to allow "additional" yaw manipulation with mouse/controller.
    if (m_ok.SensorActive())
    {
        OVR::Quatf    hmdOrient = m_ok.GetOrientation();
        float    yaw = 0.0f;

        hmdOrient.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &EyePitch, &EyeRoll);

        EyeYaw += (yaw - LastSensorYaw);
        LastSensorYaw = yaw;
    }

    if (!m_ok.SensorActive())
    {
        // Gamepad rotation.
        EyeYaw -= GamepadRotate.x * dt;
        // Allow gamepad to look up/down, but only if there is no Rift sensor.
        EyePitch -= GamepadRotate.y * dt;
        EyePitch -= MouseRotate.y * dt;
        EyeYaw   -= MouseRotate.x * dt;

        const float maxPitch = ((3.1415f/2)*0.98f);
        if (EyePitch > maxPitch)
            EyePitch = maxPitch;
        if (EyePitch < -maxPitch)
            EyePitch = -maxPitch;
    }

    if (HydraMove.LengthSq() > 0)
    {
        float mag = 1.0f;
        mag *= m_headSize;

        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Matrix4f pitchRotate = OVR::Matrix4f::RotationX(EyePitch);
        OVR::Matrix4f forwardTransform =  yawRotate * pitchRotate;
        OVR::Vector3f orientationVector = forwardTransform.Transform(HydraMove);
        orientationVector *= mag * MoveSpeed * dt;
        EyePos += orientationVector;
    }

    if (GamepadMove.LengthSq() > 0)
    {
        float mag = 1.0f;
        mag *= m_headSize;

        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Matrix4f pitchRotate = OVR::Matrix4f::RotationX(EyePitch);
        OVR::Matrix4f forwardTransform =  yawRotate * pitchRotate;
        OVR::Vector3f orientationVector = forwardTransform.Transform(GamepadMove);
        orientationVector *= mag * MoveSpeed * dt;
        EyePos += orientationVector;
    }

    if (MouseMove.LengthSq() > 0)
    {
        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Vector3f orientationVector = yawRotate.Transform(MouseMove);
        orientationVector *= MoveSpeed * dt;
        EyePos += orientationVector;
    }

    if (KeyboardMove.LengthSq() > 0)
    {
        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Matrix4f pitchRotate = OVR::Matrix4f::RotationX(EyePitch);
        OVR::Matrix4f forwardTransform =  yawRotate * pitchRotate;
        OVR::Vector3f orientationVector = forwardTransform.Transform(KeyboardMove);
        orientationVector *= MoveSpeed * dt;
        EyePos += orientationVector;
    }
}

/// From the OVR SDK.
void OculusAppSkeleton::AssembleViewMatrix()
{
    // Rotate and position m_oculusView Camera, using YawPitchRoll in BodyFrame coordinates.
    // 
    OVR::Matrix4f rollPitchYaw = GetRollPitchYaw();
    OVR::Vector3f up      = rollPitchYaw.Transform(UpVector);
    OVR::Vector3f forward = rollPitchYaw.Transform(ForwardVector);

    // Minimal head modelling.
    float headBaseToEyeHeight     = 0.15f;  // Vertical height of eye from base of head
    float headBaseToEyeProtrusion = 0.09f;  // Distance forward of eye from base of head

    headBaseToEyeHeight *= m_headSize;
    headBaseToEyeProtrusion *= m_headSize;

    OVR::Vector3f eyeCenterInHeadFrame(0.0f, headBaseToEyeHeight, -headBaseToEyeProtrusion);
    OVR::Vector3f shiftedEyePos = EyePos + rollPitchYaw.Transform(eyeCenterInHeadFrame);
    shiftedEyePos.y -= eyeCenterInHeadFrame.y; // Bring the head back down to original height

    m_oculusView = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + forward, up); 

    // This is what transformation would be without head modeling.
    // m_oculusView = Matrix4f::LookAtRH(EyePos, EyePos + forward, up);


    /// Set up a third person(or otherwise) view for control window
    {
        OVR::Vector3f txFollowDisp = rollPitchYaw.Transform(FollowCamDisplacement);
        FollowCamPos = EyePos + txFollowDisp;
        m_controlView = OVR::Matrix4f::LookAtRH(FollowCamPos, EyePos, up);
    }
}

void OculusAppSkeleton::mouseDown(int button, int state, int x, int y)
{
    which_button = button;
    oldx = newx;
    oldy = newy;
    if (state == GLFW_RELEASE)
    {
        which_button = -1;
    }

    if (state == GLFW_PRESS)
    {
        if (button == 3)
            PrevShader();
        else if (button == 4)
            NextShader();
    }
}

void OculusAppSkeleton::mouseMove(int x, int y)
{
    const float thresh = 32;

    oldx = newx;
    oldy = newy;
    newx = x;
    newy = y;
    const int mmx = x-oldx;
    const int mmy = y-oldy;
    const float rx = (float)mmx/thresh;
    const float ry = (float)mmy/thresh;
    
    MouseRotate = OVR::Vector3f(0, 0, 0);
    MouseMove   = OVR::Vector3f(0, 0, 0);
    
    if (which_button == GLFW_MOUSE_BUTTON_LEFT)
    {
        MouseRotate = OVR::Vector3f(rx, ry, 0);
    }
    else if (which_button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        MouseMove   = OVR::Vector3f(rx * rx * (rx > 0 ? 1 : -1),
                                    0,
                                    ry * ry * (ry > 0 ? 1 : -1));
    }
}

void OculusAppSkeleton::mouseWheel(int x, int y)
{
    if (m_keyStates[GLFW_KEY_LEFT_CONTROL] != GLFW_RELEASE)
    {
        const float incr = pow(2.0f, 1.0f/5.0f);
        m_headSize *= pow(incr, (float)y);
        m_headSize = std::max(0.001f, m_headSize);
        m_headSize = std::min(1000.0f, m_headSize);
    }
    else if (m_keyStates[GLFW_KEY_LEFT_SHIFT] != GLFW_RELEASE)
    {
#if 0
        const float incr = 0.01f;
        m_bufferGutterPctg += incr * (float)y;
        m_bufferGutterPctg = std::max(0.0f, m_bufferGutterPctg);
        m_bufferGutterPctg = std::min(1.0f, m_bufferGutterPctg);
#else
        const float curScaleUp = GetBufferScaleUp();
        const float incr = 1.05946309436f;
        float newval = curScaleUp * pow(incr, (float)y);
        newval = std::max(0.25f, newval);
        newval = std::min(8.0f, newval);
        SetBufferScaleUp(newval);
        ResizeFbo();
#endif
    }
    else
    {
        // Zoom focus with wheel - scale up FBO and gutter width together
        const float incr = pow(2.0f, 1.0f/12.0f);
        const float boostCoeff = pow(incr, (float)y);
        const float maxPctg = 0.45f;
        m_bufferBoost *= boostCoeff;

        // Calculate new gutter pctg to keep # of MPx throughput constant
        ///@todo Extract a function or 2 for this and Hydra controls
        const float bmin = 1.0f;
        const float bmax = 4.0f;
        m_bufferBoost = std::max(bmin, m_bufferBoost);
        m_bufferBoost = std::min(bmax, m_bufferBoost);
        const float unitFactor = (m_bufferBoost - bmin) / (bmax - bmin);
        m_bufferGutterPctg = maxPctg * unitFactor;
        {
            ResizeFbo();
        }
    }
}

void OculusAppSkeleton::keyboard(int key, int action, int x, int y)
{
    if ((key > -1) && (key <= GLFW_KEY_LAST))
    {
        m_keyStates[key] = action;
    }

    if (action != GLFW_PRESS)
    {
        return;
    }

    switch (key)
    {
    case 'Z':
        m_displaySceneInControl = !m_displaySceneInControl;
        break;

    case GLFW_KEY_SPACE:
        if (m_keyStates[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS)
            PrevShader();
        else
            NextShader();
        break;

    case GLFW_KEY_BACKSPACE:
        PrevShader();
        break;

    default:
        //printf("%d: %c\n", key, (char)key);
        break;
    }

    fflush(stdout);
}

void OculusAppSkeleton::resize(int w, int h)
{
    m_windowWidth = w;
    m_windowHeight = h;
    AppSkeleton::resize(w,h);
}


///////////////////////////////////////////////////////////////////////////////


/// Handle animations, joystick states and viewing matrix
void OculusAppSkeleton::timestep(float dt)
{
    if (m_loadingShader && m_drawnBlackFrame)
    {
        m_loadingShader = m_drawnBlackFrame = false;
        ReloadCurrentShader();
    }

    m_scene.Timestep(dt, m_headSize);

    const float frequency = 5.0f;
    const float amplitude = 0.2f;

    HandleHydra();
    HandleGlfwJoystick();
    HandleKeyboardMovement();
    AccumulateInputs(dt);
    AssembleViewMatrix();
    m_ok.UpdateEyeParams();
}


///////////////////////////////////////////////////////////////////////////////


/// Render avatar of Oculus user
void OculusAppSkeleton::DrawFrustumAvatar(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const
{
    //if (UseFollowCam)
    const GLuint prog = m_avatarProg;
    glUseProgram(prog);
    {
        OVR::Matrix4f rollPitchYaw = GetRollPitchYaw();
        OVR::Matrix4f eyetx = mview
            * OVR::Matrix4f::Translation(EyePos.x, EyePos.y, EyePos.z)
            * rollPitchYaw;

        glUniformMatrix4fv(getUniLoc(prog, "mvmtx"), 1, false, &eyetx.Transposed().M[0][0]);
        glUniformMatrix4fv(getUniLoc(prog, "prmtx"), 1, false, &persp.Transposed().M[0][0]);

        glLineWidth(4.0f);
        DrawOriginLines();
        const float aspect = (float)GetOculusWidth() / (float)GetOculusHeight();
        DrawViewFrustum(aspect);
        glLineWidth(1.0f);
    }
}

void OculusAppSkeleton::DrawScene(bool stereo, OVRkill::DisplayMode mode) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const int fboWidth = m_ok.GetRenderBufferWidth();
    const int fboHeight = m_ok.GetRenderBufferHeight();
    const int halfWidth = fboWidth/2;
    if (stereo)
    {
        const OVR::HMDInfo& hmd = m_ok.GetHMD();
        // Compute Aspect Ratio. Stereo mode cuts width in half.
        float aspectRatio = float(hmd.HResolution * 0.5f) / float(hmd.VResolution);

        // Compute Vertical FOV based on distance.
        float halfScreenDistance = (hmd.VScreenSize / 2);
        float yfov = 2.0f * atan(halfScreenDistance/hmd.EyeToScreenDistance);

        // Post-projection viewport coordinates range from (-1.0, 1.0), with the
        // center of the left viewport falling at (1/4) of horizontal screen size.
        // We need to shift this projection center to match with the lens center.
        // We compute this shift in physical units (meters) to correct
        // for different screen sizes and then rescale to viewport coordinates.
        float viewCenterValue = hmd.HScreenSize * 0.25f;
        float eyeProjectionShift = viewCenterValue - hmd.LensSeparationDistance * 0.5f;
        float projectionCenterOffset = 4.0f * eyeProjectionShift / hmd.HScreenSize;

        // Projection matrix for the "center eye", which the left/right matrices are based on.
        OVR::Matrix4f projCenter = OVR::Matrix4f::PerspectiveRH(yfov, aspectRatio, 0.3f, 1000.0f);
        OVR::Matrix4f projLeft   = OVR::Matrix4f::Translation(projectionCenterOffset, 0, 0) * projCenter;
        OVR::Matrix4f projRight  = OVR::Matrix4f::Translation(-projectionCenterOffset, 0, 0) * projCenter;

        // m_oculusView transformation translation in world units.
        float halfIPD = hmd.InterpupillaryDistance * 0.5f * m_headSize;
        if (m_flattenStereo)
            halfIPD = 0.0f;
        OVR::Matrix4f viewLeft = OVR::Matrix4f::Translation(halfIPD, 0, 0) * m_oculusView;
        OVR::Matrix4f viewRight= OVR::Matrix4f::Translation(-halfIPD, 0, 0) * m_oculusView;

        ///@note Scissoring out the fragments near the periphery of the FOV should (hopefully)
        /// result in higher performance by having to draw fewer pixels.
        /// This is contingent on the driver's implementation performing the scissor test
        /// *before* execution of the fragment shader.
        glEnable(GL_SCISSOR_TEST);
        {
            const GLsizei gw = (int)(m_bufferGutterPctg * (float)halfWidth);
            const GLsizei gh = (int)(m_bufferGutterPctg * (float)fboHeight);
            const GLsizei gl = 2*gw - (int)( m_bufferGutterFocus * (float)gw);//2*gw/3;
            const GLsizei gr = 2*gw - gl;

            ///@todo Center shrunken rects on viewpoint center
            glViewport(0           , 0 , (GLsizei)halfWidth      , (GLsizei)fboHeight     );
            glScissor (gl          , gh, (GLsizei)halfWidth -2*gw, (GLsizei)fboHeight -2*gh);
            m_scene.RenderForOneEye(viewLeft, projLeft, halfWidth, fboHeight, true);

            glViewport(halfWidth   , 0 , (GLsizei)halfWidth      , (GLsizei)fboHeight     );
            glScissor (halfWidth+gr, gh, (GLsizei)halfWidth -2*gw, (GLsizei)fboHeight -2*gh);
            m_scene.RenderForOneEye(viewRight, projRight, halfWidth, fboHeight, false);
        }
        glDisable(GL_SCISSOR_TEST);
    }
    else
    {
        /// Set up our 3D transformation matrices
        /// Remember DX and OpenGL use transposed conventions. And doesn't DX use left-handed coords?
        OVR::Matrix4f mview = m_controlView;
        OVR::Matrix4f persp = OVR::Matrix4f::PerspectiveRH(
            m_viewAngleDeg * (float)M_PI / 180.0f,
            (float)m_windowWidth/(float)m_windowHeight,
            0.004f,
            500.0f);

        const GLsizei gw = (int)(m_bufferGutterPctg * (float)fboWidth);
        const GLsizei gh = (int)(m_bufferGutterPctg * (float)fboHeight);
        glViewport(0 , 0 , (GLsizei)fboWidth     , (GLsizei)fboHeight     );
        glScissor (gw, gh, (GLsizei)fboWidth-2*gw, (GLsizei)fboHeight-2*gh);

        glEnable(GL_SCISSOR_TEST);
        m_scene.RenderForOneEye(mview, persp, fboWidth, fboHeight);
        glDisable(GL_SCISSOR_TEST);

        //DrawFrustumAvatar(mview, persp);
    }
}

/// Set up view matrices, then draw scene
void OculusAppSkeleton::display(bool isControl, OVRkill::DisplayMode mode) const
{
    /// This may save us some frame rate
    if (isControl && !m_displaySceneInControl)
    {
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    glEnable(GL_DEPTH_TEST);

    m_ok.BindRenderBuffer();
    {
        bool useStereo = (mode == OVRkill::Stereo) ||
                         (mode == OVRkill::StereoWithDistortion);
        DrawScene(useStereo, mode);
    }
    m_ok.UnBindRenderBuffer();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    OVRkill::PostProcessType post = OVRkill::PostProcess_None;
    if (mode == OVRkill::StereoWithDistortion)
    {
        post = OVRkill::PostProcess_Distortion;
    }

    m_ok.PresentFbo(post, m_riftDist);
}

// OculusAppSkeleton.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <map>
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#ifdef USE_CUDA
#else
#  include "vector_make_helpers.h"
#endif

#include "OVR.h"
#include "OVR_Shaders.h"
#include "Util/Util_Render_Stereo.h"

#include "AppSkeleton.h"
#include "Scene.h"
#include "OVRkill.h"
#include "Timer.h"

///@brief Encapsulates as much of the VR viewer state as possible,
/// pushing all viewer-independent stuff to Scene.
/// display takes a bool to indicate Oculus window or Control.
class OculusAppSkeleton : public AppSkeleton
{
public:
    OculusAppSkeleton();
    virtual ~OculusAppSkeleton();

    virtual void display(bool isControl=false, OVRkill::DisplayMode mode=OVRkill::SingleEye) const;
    virtual void mouseDown(int button, int state, int x, int y);
    virtual void mouseMove(int x, int y);
    virtual void mouseWheel(int x, int y);
    virtual void keyboard(int key, int action, int x, int y);
    virtual void resize(int w, int h);
    virtual bool initVR(bool fullScreen);
    virtual bool initJoysticks();
    virtual bool initGL(int argc, char **argv);
    virtual void timestep(float dt);

    void SetBufferScaleUp(float s) { m_bufferScaleUp = s; }
    void ResetEyePosition()
    {
        EyePos = OVR::Vector3f(0.0f, m_standingHeight, -5.0f);
        EyeYaw = YawInitial;
        EyePitch = 0;
        EyeRoll = 0;
    }

    virtual void SetupCurrentShader()
    {
        ResetEyePosition();
        GetShaderParams();
    }

    virtual void PrevShader()
    {
        m_loadingShader = true;
        m_scene.DestroyCurrentShader();
        m_scene.PrevShaderName();
    }

    virtual void NextShader()
    {
        m_loadingShader = true;
        m_scene.DestroyCurrentShader();
        m_scene.NextShaderName();
    }

    virtual void ReloadCurrentShader()
    {
        m_scene.RefreshCurrentShader();
        SetupCurrentShader();
    }

    virtual void ResetSceneTimer() { m_scene.ResetTimer(); }
    virtual void GetShaderParams();
    void ResizeFbo();

    int GetOculusWidth() const { return m_ok.GetOculusWidth(); }
    int GetOculusHeight() const { return m_ok.GetOculusHeight(); }
    float GetBufferScaleUp() const { return m_bufferScaleUp; }
    float GetMegaPixelCount() const;
    const std::map<std::string, std::string>& GetVarMap() const { return m_varMap; }

    OVR::Matrix4f GetRollPitchYaw() const {
        return OVR::Matrix4f::RotationY(EyeYaw) *
               OVR::Matrix4f::RotationX(EyePitch) *
               OVR::Matrix4f::RotationZ(EyeRoll);
    }

    bool m_loadingShader;
    bool m_drawnBlackFrame;

protected:
    void HandleHydra();
    void HandleGlfwJoystick();
    void HandleKeyboardMovement();
    void AccumulateInputs(float dt);
    void AssembleViewMatrix();

    void DrawFrustumAvatar(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const;
    void DrawScene(bool stereo, OVRkill::DisplayMode mode=OVRkill::SingleEye) const;

    /// VR view parameters
    const OVR::Vector3f UpVector;
    const OVR::Vector3f ForwardVector;
    const OVR::Vector3f RightVector;
    const float  YawInitial;
    const float  Sensitivity;
    const float  MoveSpeed;
    const float  m_standingHeight;
    const float  m_crouchingHeight;
    OVR::Vector3f EyePos;
    float EyeYaw;
    float EyePitch;
    float EyeRoll;
    float LastSensorYaw;
    OVR::Vector3f FollowCamDisplacement;
    OVR::Vector3f FollowCamPos;
    float m_viewAngleDeg; ///< For the no HMD case

    OVR::Matrix4f  m_oculusView; /// World modelview matrix for Oculus
    OVR::Matrix4f  m_controlView; /// World modelview matrix for Control window

    /// Viewing parameters fed in from joystick and the HMD
    OVR::Vector3f  HydraMove, HydraRotate;
    OVR::Vector3f  GamepadMove, GamepadRotate;
    OVR::Vector3f  MouseMove, MouseRotate;
    OVR::Vector3f  KeyboardMove, KeyboardRotate;

    /// For choosing one connected joystick
    int preferredGamepadID;
    bool swapGamepadRAxes;
    std::vector<unsigned char> lastGamepadButtons;

    /// Mouse motion internal state
    int oldx, oldy, newx, newy;
    int which_button;
    int modifier_mode;
    int m_keyStates[GLFW_KEY_LAST];

    OVRkill m_ok;
    RiftDistortionParams  m_riftDist;
    float m_bufferScaleUp;
    float m_bufferBoost;
    float m_bufferGutterPctg;
    float m_bufferGutterFocus;
    bool  m_flattenStereo;
    float m_headSize;

    Scene   m_scene;

    GLuint m_avatarProg;
    GLuint m_basictexProg;
    bool   m_displaySceneInControl;

    std::map<std::string, std::string>  m_varMap;

private: // Disallow copy ctor and assignment operator
    OculusAppSkeleton(const OculusAppSkeleton&);
    OculusAppSkeleton& operator=(const OculusAppSkeleton&);
};

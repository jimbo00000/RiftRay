// OVRkill.h

#pragma once

#include "OVR.h"
#include "FBO.h"

struct RiftDistortionParams
{
    float lensOff;
    float LensCenterX;
    float LensCenterY;
    float ScreenCenterX;
    float ScreenCenterY;
    float ScaleX;
    float ScaleY;
    float ScaleInX;
    float ScaleInY;
    float DistScale;

    RiftDistortionParams()
        : lensOff(0.287994f - 0.25f) // this value ripped from the TinyRoom demo at runtime
        , LensCenterX(0.25f)
        , LensCenterY(0.50f)
        , ScreenCenterX(0.25f)
        , ScreenCenterY(0.5f)
        , ScaleX(0.145806f)
        , ScaleY(0.233290f)
        , ScaleInX(4.0f)
        , ScaleInY(2.5f)
        , DistScale(1.0f)
    {}
};

///@brief The OVRkill class is instantiated once globally and exists to push as much
/// code as possible out of the app skeleton main source file.
class OVRkill
{
public:
    // Stereo viewing parameters
    enum PostProcessType
    {
        PostProcess_None,
        PostProcess_Distortion
    };

    OVRkill();
    virtual ~OVRkill();

    bool       SensorActive() const { return m_pSensor != NULL; }
    OVR::Quatf GetOrientation() const { return m_pSFusion->GetOrientation(); }
    bool       GetStereoMode() const { return m_SConfig.GetStereoMode() == OVR::Util::Render::Stereo_LeftRight_Multipass; }
    const OVR::HMDInfo& GetHMD() const { return m_HMDInfo; }

    int GetOculusWidth() const { return m_windowWidth; }
    int GetOculusHeight() const { return m_windowHeight; }
    int GetRenderBufferWidth() const { return m_fboWidth; }
    int GetRenderBufferHeight() const { return m_fboHeight; }
    float GetRenderBufferScaleIncrease() { return m_SConfig.GetDistortionScale(); }

    void InitOVR();
    void DestroyOVR();
    void CreateShaders();
    void CreateRenderBuffer(float bufferScaleUp);
    void UpdateEyeParams();
    void BindRenderBuffer() const;
    void UnBindRenderBuffer() const;

    void PresentFbo(
        PostProcessType post,
        const RiftDistortionParams& distParams) const;
    void PresentFbo_NoDistortion() const;
    void PresentFbo_PostProcessDistortion(
        const OVR::Util::Render::StereoEyeParams& eyeParams,
        const RiftDistortionParams& distParams) const;

    enum DisplayMode
    {
        SingleEye,
        Stereo,
        StereoWithDistortion
    };

    void SetDisplayMode(DisplayMode);

protected:
    // OVR hardware
    OVR::Ptr<OVR::DeviceManager>  m_pManager;
    OVR::Ptr<OVR::HMDDevice>      m_pHMD;
    OVR::Ptr<OVR::SensorDevice>   m_pSensor;
    OVR::SensorFusion*            m_pSFusion;
    OVR::HMDInfo                  m_HMDInfo;

    OVR::Util::Render::StereoEyeParams m_LeyeParams;
    OVR::Util::Render::StereoEyeParams m_ReyeParams;
    OVR::Util::Render::StereoConfig    m_SConfig;

    // Render buffer for OVR distortion correction shader
    FBO m_renderBuffer;
    int m_fboWidth;
    int m_fboHeight;

    GLuint m_progRiftDistortion;
    GLuint m_progPresFbo;

    int m_windowWidth;
    int m_windowHeight;

private: // Disallow copy ctor and assignment operator
    OVRkill(const OVRkill&);
    OVRkill& operator=(const OVRkill&);
};

// OVRkill.cpp

#include <GL/glew.h>
#include "OVRkill.h"
#include "OVR_Shaders.h"
#include "GL/ShaderFunctions.h"


/// Once source is obtained from either file or hard-coded map, compile the
/// shader, release the string memory and return the ID.
GLuint makeShaderFromSource(const char* source, const unsigned long Type)
{
    if (source == NULL)
        return 0;
    GLint length = strlen(source);

    GLuint shaderId = glCreateShader(Type);
    glShaderSource(shaderId, 1, &source, &length);
    glCompileShader(shaderId);

    return shaderId;
}

/// Create a shader program from vertex and fragment shader sources.
GLuint BuildShader(const char* pVertSrc, const char* pFragSrc)
{
    GLuint vertSrc = makeShaderFromSource(pVertSrc, GL_VERTEX_SHADER);
    GLuint fragSrc = makeShaderFromSource(pFragSrc, GL_FRAGMENT_SHADER);

    printShaderInfoLog(vertSrc);
    printShaderInfoLog(fragSrc);

    GLuint program = glCreateProgram();

    glCompileShader(vertSrc);
    glCompileShader(fragSrc);

    glAttachShader(program, vertSrc);
    glAttachShader(program, fragSrc);

    // Will be deleted when program is.
    glDeleteShader(vertSrc);
    glDeleteShader(fragSrc);

    glLinkProgram(program);
    printProgramInfoLog(program);

    glUseProgram(0);
    return program;
}


///@note Remember that initialization here depends on GL context state.
/// Shaders and buffers are created later.
OVRkill::OVRkill()
: m_pManager(NULL)
, m_pHMD(NULL)
, m_pSensor(NULL)
, m_pSFusion(NULL)
, m_HMDInfo()
, m_SConfig()
, m_fboWidth(0)
, m_fboHeight(0)
, m_progRiftDistortion(0)
, m_progPresFbo(0)
, m_windowWidth(0)
, m_windowHeight(0)
{
}

OVRkill::~OVRkill()
{
    //if (m_pSFusion)
    //    delete m_pSFusion;
    DestroyOVR();
}

void OVRkill::DestroyOVR()
{
    // Clear these before calling Destroy.
    m_pSensor.Clear();
    m_pManager.Clear();
    m_pHMD.Clear();
    // No OVR functions involving memory are allowed after this.
    OVR::System::Destroy();
}

/// We need an active GL context for this
void OVRkill::CreateShaders()
{
    m_progPresFbo        = BuildShader(PresentFboVertSrc         , PresentFboFragSrc);
    m_progRiftDistortion = BuildShader(PostProcessVertexShaderSrc, PostProcessFragShaderSrc);
}

/// We need an active GL context for this
void OVRkill::CreateRenderBuffer(float bufferScaleUp)
{
    deallocateFBO(m_renderBuffer);

    m_fboWidth = (int)((bufferScaleUp) * (float)m_windowWidth );
    m_fboHeight = (int)((bufferScaleUp) * (float)m_windowHeight );
    allocateFBO(m_renderBuffer, m_fboWidth, m_fboHeight);
}

void OVRkill::BindRenderBuffer() const
{
    bindFBO(m_renderBuffer);
}

void OVRkill::UnBindRenderBuffer() const
{
    unbindFBO();
}

void OVRkill::PresentFbo_NoDistortion() const
{
    glUseProgram(m_progPresFbo);
    {
        OVR::Matrix4f ortho = OVR::Matrix4f::Ortho2D((float)m_fboWidth, (float)m_fboHeight);
        glUniformMatrix4fv(getUniLoc(m_progPresFbo, "prmtx"), 1, false, &ortho.Transposed().M[0][0]);

        const float verts[] = {
            0                ,  0,
            (float)m_fboWidth,  0,
            (float)m_fboWidth, (float)m_fboHeight,
            0                , (float)m_fboHeight,
        };
        const float texs[] = {
            0,1,
            1,1,
            1,0,
            0,0,
        };
        const unsigned int tris[] = {
            0,1,2, 0,3,2, // ccw
        };

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        int posAttrib =  glGetAttribLocation(m_progPresFbo, "vPosition");
        int texAttrib =  glGetAttribLocation(m_progPresFbo, "vTex");
        
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, texs);
        glEnableVertexAttribArray(posAttrib);
        glEnableVertexAttribArray(texAttrib);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_renderBuffer.tex);
        glUniform1i(getUniLoc(m_progPresFbo, "fboTex"), 0);

        glDrawElements(GL_TRIANGLES,
                       6,
                       GL_UNSIGNED_INT,
                       &tris[0]);

        glDisableVertexAttribArray(posAttrib);
        glDisableVertexAttribArray(texAttrib);
    }
    glUseProgram(0);
}

void OVRkill::PresentFbo_PostProcessDistortion(
    const OVR::Util::Render::StereoEyeParams& eyeParams,
    const RiftDistortionParams& distParams) const
{
    const OVR::Util::Render::DistortionConfig*  pDistortion = eyeParams.pDistortion;
    if (pDistortion == NULL)
        return;

    glUseProgram(m_progRiftDistortion);
    {
        // Set uniforms for distortion shader
        OVR::Matrix4f ident;
        glUniformMatrix4fv(getUniLoc(m_progRiftDistortion, "View"), 1, false, &ident.Transposed().M[0][0]);
        glUniformMatrix4fv(getUniLoc(m_progRiftDistortion, "Texm"), 1, false, &ident.Transposed().M[0][0]);

        //"uniform vec2 LensCenter;\n"
        //"uniform vec2 ScreenCenter;\n"
        //"uniform vec2 Scale;\n"
        //"uniform vec2 ScaleIn;\n"
        //"uniform vec4 HmdWarpParam;\n"

        // The left screen is centered at (0.25, 0.5)
        glUniform2f(getUniLoc(m_progRiftDistortion, "LensCenter"),
            distParams.LensCenterX + distParams.lensOff, distParams.LensCenterY);

        glUniform2f(getUniLoc(m_progRiftDistortion, "ScreenCenter"),
            distParams.ScreenCenterX, distParams.ScreenCenterY);

        // The right screen is centered at (0.75, 0.5)
        if (eyeParams.Eye == OVR::Util::Render::StereoEye_Right)
        {
            glUniform2f(getUniLoc(m_progRiftDistortion, "LensCenter"),
                1.0f - (distParams.LensCenterX + distParams.lensOff), distParams.LensCenterY);

            glUniform2f(getUniLoc(m_progRiftDistortion, "ScreenCenter"),
                1.0f - distParams.ScreenCenterX, distParams.ScreenCenterY);
        }
        
        glUniform2f(getUniLoc(m_progRiftDistortion, "Scale"),
            distParams.ScaleX,  distParams.ScaleY);

        glUniform2f(getUniLoc(m_progRiftDistortion, "ScaleIn"),
            distParams.ScaleInX, distParams.ScaleInY);

        glUniform4f(getUniLoc(m_progRiftDistortion, "HmdWarpParam"),
            distParams.DistScale * pDistortion->K[0],
            distParams.DistScale * pDistortion->K[1],
            distParams.DistScale * pDistortion->K[2],
            distParams.DistScale * pDistortion->K[3]
        );

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_renderBuffer.tex);
        glUniform1i(getUniLoc(m_progRiftDistortion, "Texture0"), 0);

        float verts[] = { // Left eye coords
            -1.0f, -1.0f,
             0.0f, -1.0f,
             0.0f,  1.0f,
            -1.0f,  1.0f,
        };
        float texs[] = {
            0.0f, 1.0f,
            0.5f, 1.0f,
            0.5f, 0.0f,
            0.0f, 0.0f,
        };

        // Adjust coords for right eye
        if (eyeParams.Eye == OVR::Util::Render::StereoEye_Right)
        {
            verts[2*0  ] += 1.0f;
            verts[2*1  ] += 1.0f;
            verts[2*2  ] += 1.0f;
            verts[2*3  ] += 1.0f;
            
            texs[2*0] += 0.5f;
            texs[2*1] += 0.5f;
            texs[2*2] += 0.5f;
            texs[2*3] += 0.5f;
        }

        const unsigned int tris[] = {
            0,1,2,  0,2,3, // ccw
        };

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        int posAttrib =  glGetAttribLocation(m_progPresFbo, "vPosition");
        int texAttrib =  glGetAttribLocation(m_progPresFbo, "vTex");
        
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, texs);
        glEnableVertexAttribArray(posAttrib);
        glEnableVertexAttribArray(texAttrib);

        glDrawElements(GL_TRIANGLES,
                       6,
                       GL_UNSIGNED_INT,
                       &tris[0]);

        glDisableVertexAttribArray(posAttrib);
        glDisableVertexAttribArray(texAttrib);
    }
    glUseProgram(0);
}

void OVRkill::PresentFbo(PostProcessType post, const RiftDistortionParams& distParams) const
{
    if (post == PostProcess_Distortion)
    {
        PresentFbo_PostProcessDistortion(m_LeyeParams, distParams);
        PresentFbo_PostProcessDistortion(m_ReyeParams, distParams);
    }
    else
    {
        PresentFbo_NoDistortion();
    }
}

void OVRkill::UpdateEyeParams()
{
    m_LeyeParams = m_SConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
    m_ReyeParams = m_SConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
}

/// Call OVR setup routines to get HMD info from the first available HMD.
void OVRkill::InitOVR()
{
    OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));
    m_pSFusion = new OVR::SensorFusion;
    m_pManager = *OVR::DeviceManager::Create();
    m_pHMD  = *m_pManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
    if (m_pHMD == NULL)
    {
        // These default values were copied from the Rift DK1 and will be
        // used when no Rift is present so output looks sane.
        OVR::HMDInfo& hmd = m_HMDInfo;
        hmd.DesktopX = 0;
        hmd.DesktopY = 0;
        hmd.HResolution = 1280;
        hmd.VResolution = 800;

        hmd.HScreenSize = 0.14975999f;
        hmd.VScreenSize = 0.093599997f;
        hmd.VScreenCenter = 0.046799999f;

        hmd.DistortionK[0] = 1.0f;
        hmd.DistortionK[1] = 0.5f;
        hmd.DistortionK[2] = 0.25f;
        hmd.DistortionK[3] = 0.0f;

        hmd.EyeToScreenDistance = 0.041000001f;
        hmd.InterpupillaryDistance = 0.064f;
        hmd.LensSeparationDistance = 0.063500002f;
    }
    else
    {
        m_pSensor = *m_pHMD->GetSensor();

        // This will initialize HMDInfo with information about configured IPD,
        // screen size and other variables needed for correct projection.
        // We pass HMD DisplayDeviceName into the renderer to select the
        // correct monitor in full-screen mode.
        if (m_pHMD->GetDeviceInfo(&m_HMDInfo))
        {
            m_SConfig.SetHMDInfo(m_HMDInfo);
        }
        if (m_pSensor)
        {
            // We need to attach sensor to SensorFusion object for it to receive 
            // body frame messages and update orientation. SFusion.GetOrientation() 
            // is used in OnIdle() to orient the view.
            m_pSFusion->AttachToSensor(m_pSensor);
            //SFusion.SetDelegateMessageHandler(this);
            //SFusion.SetPredictionEnabled(true);
        }
    }

    if (m_HMDInfo.HResolution > 0)
    {
        m_windowWidth  = m_HMDInfo.HResolution;
        m_windowHeight = m_HMDInfo.VResolution;
    }

    // *** Configure Stereo settings.
    m_SConfig.SetFullViewport(OVR::Util::Render::Viewport(0,0, m_windowWidth, m_windowHeight));
    m_SConfig.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);

    // Configure proper Distortion Fit.
    // For 7" screen, fit to touch left side of the view, leaving a bit of invisible
    // screen on the top (saves on rendering cost).
    // For smaller screens (5.5"), fit to the top.
    if (m_HMDInfo.HScreenSize > 0.0f)
    {
        if (m_HMDInfo.HScreenSize > 0.140f) // 7"
            m_SConfig.SetDistortionFitPointVP(-1.0f, 0.0f);
        else
            m_SConfig.SetDistortionFitPointVP(0.0f, 1.0f);
    }
    m_SConfig.Set2DAreaFov(OVR::DegreeToRad(85.0f));

    // For OVR distortion correction shader
    const float renderBufferScaleIncrease = m_SConfig.GetDistortionScale();
    m_fboWidth = (int)((renderBufferScaleIncrease) * (float)m_windowWidth );
    m_fboHeight = (int)((renderBufferScaleIncrease) * (float)m_windowHeight );
}


void OVRkill::SetDisplayMode(DisplayMode mode)
{
    switch(mode)
    {
    default:
    case SingleEye:
        m_SConfig.SetStereoMode(OVR::Util::Render::Stereo_None);
        break;

    case Stereo:
        m_SConfig.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
        break;

    case StereoWithDistortion:
        m_SConfig.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
        break;
    }
}

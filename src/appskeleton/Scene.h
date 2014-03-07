// Scene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

#include <vector>
#include <string>

#ifdef USE_CUDA
#  include <vector_functions.h> // make_int2, etc.
#  include "cutil_math.h"
#else
///@todo Define vector types in the absence of CUDA
#  include "vectortypes.h"
#  include "vector_make_helpers.h"
#  include "VectorMath.h"
#endif

#include "OVRkill.h"
#include "Timer.h"
#include "FlyingMouse.h"
#include "VirtualTrackball.h"

///@brief The Scene class renders everything in the VR world that will be the same
/// in the Oculus and Control windows. The RenderForOneEye function is the display entry point.
class Scene
{
public:
    Scene();
    virtual ~Scene();

    void initGL();
    void Timestep(float dt, float headSize);
    void RenderForOneEye(
        const OVR::Matrix4f& mview,
        const OVR::Matrix4f& persp,
        int pixelWidth,
        int pixelHeight,
        bool isLeft=true) const;

    void ReloadShaders();
    void DestroyCurrentShader();
    void RefreshCurrentShader();
    void PrevShaderName();
    void NextShaderName();
    void LoadTextures(const std::vector<std::string>&);
    void ResetTimer() { m_globalTime.reset(); }
    const std::string& GetCurrentShaderName() const { return m_shaderNames[m_currentShaderIdx]; }

#ifdef USE_HYDRA
    const FlyingMouse& GetFlyingMouseState() const { return g_fm; }
#endif

protected:
    void AssembleRwwttShaderByName(const char* pFilename);
    void DrawColorCube() const;
    void DrawGrid() const;
    void DrawOrigin() const;
    void DrawScene(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const;

protected:
    void _DrawBouncingCubes(const OVR::Matrix4f& mview) const;
    void _DrawSceneWireFrame(const OVR::Matrix4f& mview) const;
    void _DrawScenePlanes(const OVR::Matrix4f& mview) const;


    GLuint m_progBasic;
    GLuint m_progPlane;
    GLuint m_progRwwtt;
    Timer  m_globalTime;


    std::vector<std::string>   m_shaderNames;
    int m_currentShaderIdx;

#ifdef USE_HYDRA
    FlyingMouse    g_fm;
    float3         m_hydraOffset;
    VirtualTrackball m_vtb;
#endif //def USE_HYDRA

public:
    /// Scene animation state
    float m_phaseVal;
    float m_cubeScale;
    float m_amplitude;
    float m_eyeballCenterTweak;
    float m_triggerVal;
    std::string m_curShaderName;
    GLuint m_texChan0;
    GLuint m_texChan1;
    GLuint m_texChan2;
    GLuint m_texChan3;

    uint3  m_texDim0;
    uint3  m_texDim1;
    uint3  m_texDim2;
    uint3  m_texDim3;

private: // Disallow copy ctor and assignment operator
    Scene(const Scene&);
    Scene& operator=(const Scene&);
};

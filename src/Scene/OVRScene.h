// OVRScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <OVR_CAPI.h>

#include "IScene.h"
#include "ShaderWithVariables.h"

///@brief Render the OVR positional tracking frustum
class OVRScene : public IScene
{
public:
    OVRScene();
    virtual ~OVRScene();

    virtual void initGL();
    virtual void timestep(float dt);
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    // Retain pointers to scene orientation parameters
    virtual void SetHmdPointer(ovrHmd pHmd) { m_pHmd = pHmd; }
    virtual void SetChassisPosPointer(ovrVector3f* pPos) { m_pPos = pPos; }
    virtual void SetChassisYawPointer(float* pYaw) { m_pYaw = pYaw; }

protected:
    void _InitFrustumAttributes();
    void _DrawFrustum() const;
    void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection) const;

    ShaderWithVariables m_basic;
    ovrHmd m_pHmd;
    ovrVector3f* m_pPos;
    float* m_pYaw;
    std::vector<glm::vec3> m_frustumVerts;
    float m_distanceToFrustum;
    glm::vec2 m_tanFromCameraCenterline;
    glm::vec2 m_tanHalfFov;

private: // Disallow copy ctor and assignment operator
    OVRScene(const OVRScene&);
    OVRScene& operator=(const OVRScene&);
};

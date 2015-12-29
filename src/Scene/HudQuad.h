// HudQuad.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <OVR.h>
#include <Kernel/OVR_Types.h> // Pull in OVR_OS_* defines 
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "FBO.h"

///@brief A flat quad displayed in-world passed as a compositor layer to OVR SDK.
class HudQuad
{
public:
    HudQuad();
    virtual ~HudQuad();

    virtual void initGL(ovrHmd hmd, ovrSizei sz);
    virtual void exitGL(ovrHmd hmd);
    virtual void DrawToQuad();
    virtual void SetHoldingFlag(ovrPosef pose, bool f);
    virtual ovrPosef GetPose() const { return m_layerQuad.QuadPoseCenter; }
    virtual void SetHmdEyeRay(ovrPosef pose);

    virtual bool GetPaneRayIntersectionCoordinates(
        const glm::mat4& quadPoseMatrix, ///< [in] Quad's pose in world space
        glm::vec3 origin3, ///< [in] Ray origin
        glm::vec3 dir3, ///< [in] Ray direction(normalized)
        glm::vec2& planePtOut, ///< [out] Intersection point in XY plane coordinates
        float& tParamOut); ///< [out] t parameter of ray intersection (ro + t*dt)

    ovrLayerQuad m_layerQuad;
    ovrSwapTextureSet* m_pQuadTex;
    bool m_showQuadInWorld;

protected:
    void _PrepareToDrawToQuad() const;

    FBO m_fbo;
    glm::vec3 m_quadLocation;

    // Movement state
    bool m_holding;
    glm::vec3 m_planePositionAtGrab;
    glm::vec3 m_hitPtPositionAtGrab;
    float m_hitPtTParam;

private: // Disallow copy ctor and assignment operator
    HudQuad(const HudQuad&);
    HudQuad& operator=(const HudQuad&);
};

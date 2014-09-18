// RaymarchShaderScene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

#include <glm/glm.hpp>

#include "IScene.h"
#include "ShaderWithVariables.h"
#include "VirtualTrackball.h"
#include "FlyingMouse.h"

///@brief 
class RaymarchShaderScene : public IScene
{
public:
    RaymarchShaderScene();
    virtual ~RaymarchShaderScene();

    virtual void initGL();
    virtual void timestep(float dt);
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    virtual void SetFlyingMousePointer(FlyingMouse* pFM) { m_pFm = pFM; }
    virtual Transformation* GetTransformationPointer() { return &m_tx; }
    virtual void ResetTransformation()
    {
        m_tx.ResetOrientation();
        m_tx.ResetPosition();
        m_tx.ResetScale();
    }

protected:
    void _InitShaderRectAttributes();
    virtual void _DrawScreenQuad() const;
    void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object) const;

    ShaderWithVariables m_raymarch;
    Transformation m_tx;
    FlyingMouse* m_pFm;

private: // Disallow copy ctor and assignment operator
    RaymarchShaderScene(const RaymarchShaderScene&);
    RaymarchShaderScene& operator=(const RaymarchShaderScene&);
};

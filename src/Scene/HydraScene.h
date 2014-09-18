// HydraScene.h

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
#include "FlyingMouse.h"
#include "ShaderWithVariables.h"

///@brief 
class HydraScene : public IScene
{
public:
    HydraScene();
    virtual ~HydraScene();

    virtual void initGL();
    virtual void timestep(float) {}
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    virtual void SetFlyingMousePointer(FlyingMouse* pFM) { m_pFm = pFM; }

protected:
    void _InitOriginAttributes();
    void _InitHydraModelAttributes();
    void _DrawOrigin(int verts) const;
    void _DrawHydraModel() const;
    void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection) const;

    ShaderWithVariables m_basic;
    ShaderWithVariables m_hydra;
    int m_numPts;
    int m_numTris;
    FlyingMouse* m_pFm;

private: // Disallow copy ctor and assignment operator
    HydraScene(const HydraScene&);
    HydraScene& operator=(const HydraScene&);
};

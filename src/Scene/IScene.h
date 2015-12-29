// IScene.h

#pragma once

///@brief Interface to a 3D Scene
class IScene
{
public:
    IScene()
      : m_bDraw(true)
      , m_bChassisLocalSpace(false)
      {}

    virtual void initGL() = 0;
    virtual void exitGL() = 0;

    virtual void timestep(double absTime, double dt) = 0;

    virtual void RenderPrePass() const {}

    virtual void RenderForOneEye(
        const float* pMview,
        const float* pPersp) const = 0;

    virtual bool RayIntersects(
        const float*, // pRayOrigin [in]
        const float*, // pRayDirection [in]
        float*, // pTParameter [inout]
        float*, // pHitLocation [inout]
        float* // pHitNormal [inout]
        ) const { return false; }

    bool m_bDraw;
    bool m_bChassisLocalSpace;
};

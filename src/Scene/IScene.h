// IScene.h

#pragma once

///@brief Interface to a 3D Scene
class IScene
{
public:
    IScene() : m_bDraw(true) {}

    virtual void initGL() = 0;

    virtual void timestep(float dt) = 0;

    virtual void RenderForOneEye(
        const float* pMview,
        const float* pPersp) const = 0;

    bool m_bDraw;
};

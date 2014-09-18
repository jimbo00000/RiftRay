// Scene.h

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

///@brief The Scene class renders everything in the VR world that will be the same
/// in the Oculus and Control windows. The RenderForOneEye function is the display entry point.
class Scene : public IScene
{
public:
    Scene();
    virtual ~Scene();

    virtual void initGL();
    virtual void timestep(float) {}
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

protected:
    void DrawColorCube() const;
    void DrawGrid() const;
    void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object) const;

protected:
    void _InitPlaneAttributes();

    void _DrawScenePlanes(const glm::mat4& modelview) const;

    ShaderWithVariables m_plane;

private: // Disallow copy ctor and assignment operator
    Scene(const Scene&);
    Scene& operator=(const Scene&);
};

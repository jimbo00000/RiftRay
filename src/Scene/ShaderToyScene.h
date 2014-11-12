// ShaderToyScene.h

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

#include "IScene.h"
#include "ShaderToy.h"
#include "ShaderWithVariables.h"
#include "Timer.h"
#include "FBO.h"

struct textureChannel {
    GLuint texID;
    unsigned int w;
    unsigned int h;
};

///@brief 
class ShaderToyScene : public IScene
{
public:
    ShaderToyScene();
    virtual ~ShaderToyScene();

    virtual void initGL();
    virtual void timestep(float) {}
    virtual void RenderForOneEye(const float* pMview, const float* pPersp) const;

    void SetTextureLibraryPointer(std::map<std::string, textureChannel>* pTL) { m_pTexLibrary = pTL; }
    void SetShaderToy(ShaderToy* pST) { m_currentShaderToy = pST; }
    void ResetTimer() { m_globalTime.reset(); }

    ShaderToy* GetShaderToy() const { return m_currentShaderToy; }

protected:
    void _InitShaderRectAttributes();
    void _DrawScreenQuad() const;
    void _SetTextureUniforms(const ShaderToy* pST) const;
    void DrawScene(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object) const;

    ShaderWithVariables m_quadVao;
    Timer m_globalTime;
    std::map<std::string, textureChannel>* m_pTexLibrary;
    ShaderToy* m_currentShaderToy;

private: // Disallow copy ctor and assignment operator
    ShaderToyScene(const ShaderToyScene&);
    ShaderToyScene& operator=(const ShaderToyScene&);
};

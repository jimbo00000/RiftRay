// ShaderToy.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <map>

class ShaderToy
{
public:
    ShaderToy(const std::string& sourceFile);
    virtual ~ShaderToy();

    virtual void CompileShader();

    GLuint prog() const { return m_prog; }
    const std::string GetSourceFile() const { return m_sourceFile; }
    const std::string GetTextureFilenameAtChannel(int idx) const;
    const std::string GetStringByName(const char* key) const;
    glm::vec3 GetHeadPos() const;
    float GetHeadSize() const;

    static std::string s_shaderDir;

protected:
    virtual void _ParseVariableMap();

    std::string m_sourceFile;
    GLuint m_prog;
    std::map<std::string, std::string> m_varMap;

private: // Disallow default, copy ctor and assignment operator
    ShaderToy();
    ShaderToy(const ShaderToy&);
    ShaderToy& operator=(const ShaderToy&);
};

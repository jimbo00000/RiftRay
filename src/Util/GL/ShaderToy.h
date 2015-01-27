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

#include "Timer.h"

struct shaderVariable {
    enum variableType {
        None=0,
        Scalar,
        Direction,
        Color,
        Unknown,
    };
    std::string name;
    GLint uniLoc;
    glm::vec4 value;
    glm::vec4 minVal;
    glm::vec4 maxVal;
    float incr;
    int width;
    variableType varType;

    shaderVariable()
        : name()
        , uniLoc(-1)
        , value(0.f)
        , minVal(0.f)
        , maxVal(0.f)
        , incr(1.f)
        , width(1)
        , varType(None)
    {
    }
};

class ShaderToy
{
public:
    ShaderToy(const std::string& sourceFile);
    virtual ~ShaderToy();

    virtual void CompileShader();

    void ResetTimer() { m_globalTime.reset(); }

    GLuint prog() const { return m_prog; }
    const std::string GetSourceFile() const { return m_sourceFile; }
    const std::string GetTextureFilenameAtChannel(int idx) const;
    const std::string GetStringByName(const char* key) const;
    glm::vec3 GetHeadPos() const;
    float GetHeadSize() const;
    float GlobalTime() const { return static_cast<float>(m_globalTime.seconds()); }

    static std::string s_shaderDir;

protected:
    virtual void _ParseVariableLine(const std::string&);
    virtual void _ParseVariableMap();

    std::string m_sourceFile;
    GLuint m_prog;
    std::map<std::string, std::string> m_varMap;
    Timer m_globalTime;

public:
    std::map<std::string, shaderVariable> m_tweakVars;

private: // Disallow default, copy ctor and assignment operator
    ShaderToy();
    ShaderToy(const ShaderToy&);
    ShaderToy& operator=(const ShaderToy&);
};

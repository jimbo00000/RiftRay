// ShaderWithVariables.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <GL/glew.h>

#include <map>
#include <string>

///@brief 
class ShaderWithVariables
{
public:
    ShaderWithVariables();
    virtual ~ShaderWithVariables();

    virtual void initProgram(const char* shadername);
    virtual void AddVbo(const std::string name, GLuint vbo) { m_vbos[name] = vbo; }
    virtual void destroy();

    virtual GLuint prog() const { return m_program; }
    virtual void bindVAO() const { glBindVertexArray(m_vao); }
    virtual GLint GetAttrLoc(const std::string name) const;
    virtual GLint GetUniLoc(const std::string name) const;
    virtual GLuint GetVboLoc(const std::string name) const;

protected:
    virtual void findVariables(const char* vertsrc);

    GLuint m_program;
    GLuint m_vao;
    std::map<std::string, GLint> m_attrs;
    std::map<std::string, GLint> m_unis;
    std::map<std::string, GLuint> m_vbos;

private: // Disallow copy ctor and assignment operator
    ShaderWithVariables(const ShaderWithVariables&);
    ShaderWithVariables& operator=(const ShaderWithVariables&);
};

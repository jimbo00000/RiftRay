// ShaderWithVariables.cpp

#include "ShaderWithVariables.h"
#include "ShaderFunctions.h"
#include "StringFunctions.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

ShaderWithVariables::ShaderWithVariables()
: m_program(0)
, m_vao(0)
, m_attrs()
, m_unis()
, m_vbos()
{
}

ShaderWithVariables::~ShaderWithVariables()
{
    destroy();
}

void ShaderWithVariables::destroy()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
        m_program = 0;
    }

    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    for (std::map<std::string, GLuint>::iterator it = m_vbos.begin();
        it != m_vbos.end();
        ++it)
    {
        GLuint vbo = it->second;
        glDeleteBuffers(1, &vbo);
    }

    m_attrs.clear();
    m_unis.clear();
    m_vbos.clear();
}

void ShaderWithVariables::initProgram(const char* shadername)
{
    glGenVertexArrays(1, &m_vao);

    std::cout
        << "Shader ["
        << shadername
        << "] "
        ;

    std::string vs = shadername;
    std::string fs = shadername;
    vs += ".vert";
    fs += ".frag";

    m_program = makeShaderFromSource(vs.c_str(), fs.c_str());
    if (m_program == 0)
        return;

    std::cout << "  vars: ";
    const std::string vsrc = GetShaderSource(vs.c_str());
    findVariables(vsrc.c_str());
    const std::string fsrc = GetShaderSource(fs.c_str());
    findVariables(fsrc.c_str());

    std::cout
        << m_unis.size() << " unis, "
        << m_attrs.size() << " attrs."
        << std::endl;
}

void ShaderWithVariables::findVariables(const char* vertsrc)
{
    ///@todo handle all kinds of line breaks?
    std::vector<std::string> vtoks = split(vertsrc, '\n');

    for (std::vector<std::string>::const_iterator it = vtoks.begin();
        it != vtoks.end();
        ++it)
    {
        const std::string& line = *it;
        ///@todo Handle tabs, etc.
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.size() < 3)
            continue;

        // We are assuming this will strip off the trailing semicolon.
        std::string var = tokens[2];
        var = var.substr(0, var.length()-1);

        if (!tokens[0].compare("uniform"))
        {
            m_unis[var] = glGetUniformLocation(m_program, var.c_str());
        }
        else if (!tokens[0].compare("in"))
        {
            m_attrs[var] = glGetAttribLocation(m_program, var.c_str());
        }
        else if (!tokens[0].compare("attribute")) // deprecated keyword
        {
            m_attrs[var] = glGetAttribLocation(m_program, var.c_str());
        }
    }
}

GLint ShaderWithVariables::GetAttrLoc(const std::string name) const
{
    std::map<std::string, GLint>::const_iterator it = m_attrs.find(name);
    if (it == m_attrs.end()) // key not found
        return -1;
    return it->second;
}

GLint ShaderWithVariables::GetUniLoc(const std::string name) const
{
    std::map<std::string, GLint>::const_iterator it = m_unis.find(name);
    if (it == m_unis.end()) // key not found
        return -1; // -1 values are ignored silently by GL
    return it->second;
}

GLuint ShaderWithVariables::GetVboLoc(const std::string name) const
{
    std::map<std::string, GLuint>::const_iterator it = m_vbos.find(name);
    if (it == m_vbos.end()) // key not found
        return 0; // -1 values are ignored silently by GL
    return it->second;
}

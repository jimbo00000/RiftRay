// ShaderToy.cpp

#include "GL/glew.h"

#include "ShaderToy.h"
#include "ShaderFunctions.h"
#include "StringFunctions.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

std::string ShaderToy::s_shaderDir = "../shaders/";

ShaderToy::ShaderToy(const std::string& sourceFile)
: m_sourceFile(sourceFile)
, m_prog(0)
, m_progFulldome(0)
, m_varMap()
, m_globalTime()
, m_tweakVars()
{
}

ShaderToy::~ShaderToy()
{
    glDeleteProgram(m_prog);
    glDeleteProgram(m_progFulldome);
}

GLuint ShaderToy::_GetVsSourceId()
{
    const std::string vs("rwwtt.vert");
    return loadShaderFile(vs.c_str(), GL_VERTEX_SHADER);
}

GLuint ShaderToy::_GetFsSourceId(bool fulldome)
{
    const std::string src0 = GetShaderSourceFromTable("rwwtt_header.glsl");
    const std::string src1 = GetShaderSourceFromFile(m_sourceFile.c_str(), s_shaderDir);
    const std::string src2 = fulldome ? "#define USE_FULLDOME_PROJECTION\n" : "";
    const std::string src3 = GetShaderSourceFromTable("rwwtt_footer.glsl");

    const GLuint shaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* pSrcs[4] = {
        src0.c_str(),
        src1.c_str(),
        src2.c_str(),
        src3.c_str(),
    };
    glShaderSource(shaderId, 4, pSrcs, NULL);
    glCompileShader(shaderId);
    return shaderId;
}

GLuint ShaderToy::_MakeProgram(bool fulldome)
{
    if (m_sourceFile.empty())
        return 0;

    const GLuint vertSrc = _GetVsSourceId();
    printShaderInfoLog(vertSrc);

    const GLuint fragSrc =_GetFsSourceId(fulldome);
    printShaderInfoLog(fragSrc);

    const GLuint program = glCreateProgram();

    glCompileShader(vertSrc);
    glCompileShader(fragSrc);

    glAttachShader(program, vertSrc);
    glAttachShader(program, fragSrc);

    // Will be deleted when program is.
    glDeleteShader(vertSrc);
    glDeleteShader(fragSrc);

    glLinkProgram(program);
    printProgramInfoLog(program);

    glUseProgram(0);
    return program;
}

void ShaderToy::CompileShader()
{
    if (m_sourceFile.empty())
        return;
    std::cout
        << std::endl
        << m_sourceFile;

    m_prog = _MakeProgram(false);
    m_progFulldome = _MakeProgram(true);

    _ParseVariableMap();
}

void ShaderToy::_ParseVariableLine(const std::string& vardecl)
{
    if (vardecl.empty())
        return;

    const std::vector<std::string> tokens = split(vardecl, ' ');
    if (tokens.size() < 2)
        return;

    const std::string& type = tokens[0];
    const std::string& name = tokens[1];
    if (!type.compare("vec3"))
    {
        if (tokens.size() < 5)
            return;

        const glm::vec4 initialVal(
            atof(tokens[2].c_str()),
            atof(tokens[3].c_str()),
            atof(tokens[4].c_str()),
            0.f);

        shaderVariable var;
        var.value = initialVal;
        var.width = 3;

        if (tokens.size() > 5)
        {
            if (!tokens[5].compare("dir"))
            {
                var.varType = shaderVariable::Direction;
                //var.value = glm::vec4(glm::normalize(glm::vec3(var.value)), var.value.w);
            }
            else if (!tokens[5].compare("color"))
            {
                var.varType = shaderVariable::Color;
            }
        }
        m_tweakVars[name] = var;
    }
    else if (!type.compare("float"))
    {
        if (tokens.size() < 3)
            return;

        shaderVariable var;
        var.value = glm::vec4(atof(tokens[2].c_str()));
        if (tokens.size() >= 4)
            var.minVal = glm::vec4(atof(tokens[3].c_str()));
        if (tokens.size() >= 5)
            var.maxVal = glm::vec4(atof(tokens[4].c_str()));
        if (tokens.size() >= 6)
            var.incr = atof(tokens[5].c_str());
        var.width = 1;

        m_tweakVars[name] = var;
    }
    else
    {
        // Push {name,value} pair to hash map
        if (tokens.size() >= 2)
        {
            const std::string& name = tokens[0];
            const std::string value = vardecl.substr(name.length()+1);
            m_varMap[name] = value;
        }
    }
}

void ShaderToy::_ParseVariableMap()
{
    //const std::string src = GetShaderSourceFromFile(m_sourceFile.c_str(), s_shaderDir);
    const std::string toy = s_shaderDir + m_sourceFile;

    std::ifstream file;
    file.open(toy.c_str(), std::ios::in);
    if (!file.is_open())
        return;

    std::string str;
    // Look through lines for variable decls
    const std::string needle = "@var "; //< Include the trailing space
    while (std::getline(file,str))
    {
        std::size_t found = str.find(needle);
        if (found == std::string::npos)
            continue;

        const std::string stripped = trim(str);
        // Keep only the part after the magic decl string
        const std::string vardecl = stripped.substr(found + needle.length());
        _ParseVariableLine(vardecl);
    }
    file.close();
}

const std::string ShaderToy::GetTextureFilenameAtChannel(int idx) const
{
    std::ostringstream oss;
    oss << "tex"
        << idx;

    std::map<std::string, std::string>::const_iterator it = m_varMap.find(oss.str());
    if (it == m_varMap.end()) // key not found
        return "";

    return it->second;
}

const std::string ShaderToy::GetStringByName(const char* key) const
{
    const std::map<std::string, std::string>::const_iterator it = m_varMap.find(key);
    if (it == m_varMap.end()) // key not found
        return "";

    return it->second;
}

glm::vec3 ShaderToy::GetHeadPos() const
{
    glm::vec3 val(0.0f);
    std::map<std::string, std::string>::const_iterator it = m_varMap.find("eyePos");
    if (it == m_varMap.end()) // key not found
        return val;

    const std::string s = it->second;
    const std::vector<std::string> tokens = split(s, ' ');
    if (tokens.size() < 3)
        return val;
    val.x = static_cast<float>(strtod(tokens[0].c_str(), NULL));
    val.y = static_cast<float>(strtod(tokens[1].c_str(), NULL));
    val.z = static_cast<float>(strtod(tokens[2].c_str(), NULL));
    return val;
}

float ShaderToy::GetHeadSize() const
{
    std::map<std::string, std::string>::const_iterator it = m_varMap.find("headSize");
    if (it == m_varMap.end()) // key not found
        return 1.0f;

    const std::string s = it->second;
    const float v = static_cast<float>(strtod(s.c_str(), NULL));
    return v;
}

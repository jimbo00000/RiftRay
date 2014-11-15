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
, m_varMap()
{
}

ShaderToy::~ShaderToy()
{
    glDeleteProgram(m_prog);
}

void ShaderToy::CompileShader()
{
    if (m_sourceFile.empty())
        return;

    std::string vs("rwwtt");
    std::string fs(m_sourceFile);
    vs += ".vert";
    fs += ".frag";

    std::cout
        << std::endl
        << m_sourceFile
        ;

    const GLuint vertSrc = loadShaderFile(vs.c_str(), GL_VERTEX_SHADER);
    const std::string src1 = GetShaderSourceFromTable("rwwtt_header.glsl");
    const std::string src2 = GetShaderSourceFromFile(m_sourceFile.c_str(), s_shaderDir);
    const std::string src3 = GetShaderSourceFromTable("rwwtt_footer.glsl");

    GLuint fragSrc = 0;
    GLuint shaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* pSrcs[3] = {
        src1.c_str(),
        src2.c_str(),
        src3.c_str(),
    };
    glShaderSource(shaderId, 3, pSrcs, NULL);
    glCompileShader(shaderId);
    fragSrc = shaderId;

    printShaderInfoLog(vertSrc);
    printShaderInfoLog(fragSrc);

    GLuint program = glCreateProgram();

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
    m_prog = program;

    _ParseVariableMap();
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
    std::vector <std::string> lines;
    // Look through lines for variable decls
    const std::string needle = "@var "; //< Include the trailing space
    while (std::getline(file,str))
    {
        std::size_t found = str.find(needle);
        if (found == std::string::npos)
            continue;

        const std::string stripped = trim(str);
        // Keep only the part after the magic decl string
        std::string vardecl = stripped.substr(found + needle.length());
        lines.push_back(vardecl);

        // Push {name,value} pair to hash map
        std::vector<std::string> tokens = split(vardecl, ' ');
        if (tokens.size() >= 2)
        {
            const std::string& name = tokens[0];
            const std::string value = vardecl.substr(name.length()+1);
            m_varMap[name] = value;
        }
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

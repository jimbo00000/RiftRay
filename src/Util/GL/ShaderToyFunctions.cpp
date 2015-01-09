// ShaderToyFunctions.cpp

#include "ShaderToyFunctions.h"
#include "ShaderToy.h"
#include "StringFunctions.h"
#include "DirectoryFunctions.h"
#include "TextureFunctions.h"

#include <string>
#include <vector>
#include <sstream>

// Assume program is bound
void SetTweakUniforms(
    const ShaderToy* pST,
    const GLuint prog)
{
    ///@todo Different type widths(float, vec2, vec3...)
    const std::map<std::string, glm::vec4>& tweakVars = pST->m_tweakVars;
    for (std::map<std::string, glm::vec4>::const_iterator it = tweakVars.begin();
        it != tweakVars.end();
        ++it)
    {
        const std::string& name = it->first;
        const glm::vec4& tv = it->second;

        const GLint uloc = glGetUniformLocation(prog, name.c_str());
        glUniform3f(uloc, tv.x, tv.y, tv.z);
    }
}

// Assume program is bound
void SetTextureUniforms(
    const ShaderToy* pST,
    const std::map<std::string, textureChannel>* pTexLib)
{
    if (pST == NULL)
        return;
    if (pTexLib == NULL)
        return;

    for (int i=0; i<4; ++i)
    {
        std::ostringstream oss;
        oss << "iChannel"
            << i;
        const GLint u_samp = glGetUniformLocation(pST->prog(), oss.str().c_str());
        const std::string texname = pST->GetTextureFilenameAtChannel(i);
        const std::map<std::string, textureChannel>::const_iterator it = pTexLib->find(texname);
        if (it != pTexLib->end()) // key not found
        {
            const textureChannel& t = it->second;
            if ((u_samp != -1) && (t.texID > 0))
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, t.texID);
                glUniform1i(u_samp, i);
            }
        }
    }
}


void LoadShaderToyTexturesFromDirectory(
    std::map<std::string, textureChannel>& texLib,
    const std::string& texdir)
{
    const std::vector<std::string> texturenames = GetListOfFilesFromDirectory(texdir);
    for (std::vector<std::string>::const_iterator it = texturenames.begin();
        it != texturenames.end();
        ++it)
    {
        const std::string& s = *it;
        const std::string fullName = texdir + s;

        GLuint texId = 0;
        GLuint width = 0;
        GLuint height = 0;
        ///@todo Case insensitivity?
        if (hasEnding(fullName, ".jpg"))
            texId = LoadTextureFromJpg(fullName.c_str(), &width, &height);
        else if (hasEnding(fullName, ".png"))
            texId = LoadTextureFromPng(fullName.c_str(), &width, &height);

        textureChannel tc = {texId, width, height};
        texLib[s] = tc;
    }
}

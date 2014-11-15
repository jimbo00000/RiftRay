// ShaderToyFunctions.cpp

#include "ShaderToyFunctions.h"
#include "StringFunctions.h"
#include "DirectoryFunctions.h"
#include "TextureFunctions.h"
#include <vector>

void SetTextureUniforms(const ShaderToy* pST)
{
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

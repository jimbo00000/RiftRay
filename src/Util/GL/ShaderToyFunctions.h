// ShaderToyFunctions.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <map>
#include <string>
#include <GL/glew.h>

struct textureChannel {
    GLuint texID;
    unsigned int w;
    unsigned int h;
};

class ShaderToy;

void SetTweakUniforms(
    const ShaderToy* pST,
    const GLuint prog);

void SetTextureUniforms(
    const ShaderToy* pST,
    const std::map<std::string, textureChannel>* pTexLib);

void LoadShaderToyTexturesFromDirectory(
    std::map<std::string, textureChannel>& texLib,
    const std::string& texdir);

// ShaderToyFunctions.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <map>
#include <GL/glew.h>

struct textureChannel {
    GLuint texID;
    unsigned int w;
    unsigned int h;
};

class ShaderToy;

void SetTextureUniforms(const ShaderToy* pST);
void LoadShaderToyTexturesFromDirectory(
    std::map<std::string, textureChannel>& texLib,
    const std::string& texdir);

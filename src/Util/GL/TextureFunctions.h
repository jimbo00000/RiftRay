// TextureFunctions.h

#pragma once

GLuint LoadTextureFromPng(
    const char* pFilename,
    unsigned int* width,
    unsigned int* height,
    bool flipY=false);

GLuint LoadTextureFromJpg(
    const char* pFilename,
    unsigned int* width,
    unsigned int* height);

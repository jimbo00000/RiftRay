// TextureFunctions.h

#pragma once

GLuint LoadTextureFromPng(
    const char* pFilename,
    unsigned int* width,
    unsigned int* height);

GLuint LoadTextureFromJpg(
    const char* pFilename,
    unsigned int* width,
    unsigned int* height);

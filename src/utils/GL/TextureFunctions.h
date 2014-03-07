// TextureFunctions.h

#pragma once

GLuint LoadTextureFromRaw(
    const char* filename,
    unsigned int dim,
    unsigned int channels=4);

GLuint LoadTextureFromPxraw(
    const char* pFilename);

void SaveToPxRaw(
    const char* pFilename,
    const char* pixels,
    unsigned int width,
    unsigned int height, unsigned int depth);

GLuint LoadTextureFromPng(
    const char* pFilename,
    unsigned int* width,
    unsigned int* height);

GLuint LoadTextureFromJpg(
    const char* pFilename,
    unsigned int* width,
    unsigned int* height);

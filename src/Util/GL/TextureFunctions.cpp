// TextureFunctions.cpp

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include "Logger.h"

#include "lodepng.h"
#include "jpgd.h"

/// Pass in a vector of char data, get back a GL texture.
/// Works for 3 and 4 channel data.
///@note Be sure the OpenGL context is created when calling
GLuint GenTextureFromImageData(
    const std::vector<unsigned char>& data,
    unsigned int dim,
    unsigned int channels)
{
    if ((channels != 3) && (channels != 4))
    {
        LOG_INFO("Unsupported number of channels: %d", channels);
        printf("Unsupported number of channels: %d", channels);
        return 0;
    }

    GLuint tex = 0;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        channels == 4 ? GL_RGBA : GL_RGB,
        dim,
        dim,
        0,
        channels == 4 ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE,
        &data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

/// http://lodev.org/lodepng/example_decode.cpp
GLuint LoadTextureFromPng(
    const char* pFilename,
    unsigned int* pWid,
    unsigned int* pHei)
{
    std::vector<unsigned char> image;
    unsigned width, height;

    //decode
    unsigned error = lodepng::decode(image, width, height, pFilename);

    //if there's an error, display it
    if(error)
    {
        std::cout
            << "decoder error "
            << error
            << ": "
            << lodepng_error_text(error)
            << std::endl;
        return 0;
    }
    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

    *pWid = width;
    *pHei = height;
    return GenTextureFromImageData(image, width, 4);
}

GLuint LoadTextureFromJpg(
    const char* pFilename,
    unsigned int* pWid,
    unsigned int* pHei)
{
    int width = 0;
    int height = 0;
    int comps = 0;
    unsigned char* pRes = jpgd::decompress_jpeg_image_from_file(
        pFilename,
        &width,
        &height, 
        &comps,
        3); // components

    const int sz = width*height*comps;
    std::vector<unsigned char> image;
    image.resize(sz);
    memcpy(&image[0], pRes, sz);
    //the pixels are now in the vector "image", comps bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

    *pWid = width;
    *pHei = height;
    return GenTextureFromImageData(image, width, comps);
}

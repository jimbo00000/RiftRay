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


// http://www.nullterminator.net/gltexture.html
/// @note Texture file must be a square image.
GLuint LoadTextureFromRaw(
    const char* filename,
    unsigned int dim,
    unsigned int channels)
{
    std::vector<unsigned char> data;

    ///@todo Maybe do something about this unholy mix of string and IO libraries...
    const char* pFullFilename = (new std::string("../data/"))->append(filename).c_str();
    
    std::ifstream in(pFullFilename, std::ifstream::binary);
    if (!in.is_open())
    {
        LOG_INFO("LoadTextureFromRaw: file %s could not be opened.", pFullFilename);
        return 0;
    }

    unsigned int size = dim*dim*channels;
    data.resize(size);

    in.read((char*)&data[0], size);

    in.close();

    LOG_INFO("LoadTextureFromRaw: file %s loaded successfully.", pFullFilename);

    return GenTextureFromImageData(data, dim, channels);
}


GLuint LoadTextureFromPxraw(
    const char* pFilename)
{
    if (pFilename == NULL)
        return 0;

    std::ifstream in(pFilename, std::ifstream::binary);
    if (!in.is_open())
    {
        fprintf(stderr, "File %s could not be opened.\n", pFilename);
        return 0;
    }

    int width = 0;
    int height = 0;
    int depth = 4;
    
    in.read((char*)&width, sizeof(int));
    in.read((char*)&height, sizeof(int));
    
    const int sz = width * height * depth;
    if (sz ==  0)
        return 0;

    std::vector<unsigned char> pixels;
    pixels.resize(sz);
    in.read((char*)&pixels[0], sz);

    in.close();


    /// Generate texture from image data
    GLuint tex = 0;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(
        GL_TEXTURE_2D,
        depth,
        width,
        height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &pixels[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

void SaveToPxRaw(
    const char* pFilename,
    const char* pixels,
    unsigned int width,
    unsigned int height,
    unsigned int depth)
{
    if (pFilename == NULL)
        return;
    if (width == 0)
        return;
    if (height == 0)
        return;
    if (depth == 0)
        return;

    std::ofstream out(pFilename, std::ifstream::binary);
    if (!out.is_open())
    {
        fprintf(stderr, "File %s could not be opened.\n", pFilename);
        return;
    }
    
    out.write((const char*)&width, sizeof(int));
    out.write((const char*)&height, sizeof(int));
    out.write((const char*)&depth, sizeof(int));
    /// Hope we don't segfault!
    const int sz = width * height * depth;
    out.write((const char*)pixels, sz);

    out.close();

    std::cout
        << "SaveToPxRaw: wrote "
        << " w=" << width
        << " h=" << height
        << " d=" << depth
        << std::endl;
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

#if 0
    std::string rawPxFilename(pFilename);
    rawPxFilename += ".pxraw";
    SaveToPxRaw(
        rawPxFilename.c_str(),
        (const char*)&image[0],
        width,
        height,
        4);
#endif

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

#if 0
    std::string rawPxFilename(pFilename);
    rawPxFilename += ".pxraw";
    SaveToPxRaw(
        rawPxFilename.c_str(),
        (const char*)&image[0],
        width,
        height,
        3);
#endif

    *pWid = width;
    *pHei = height;
    return GenTextureFromImageData(image, width, comps);
}

// ShaderFunctions.h

#pragma once

#include "AppDirectories.h"
#include <string>

GLint getUniLoc(const GLuint program, const GLchar *name);
void  printShaderInfoLog(GLuint obj);
void  printProgramInfoLog(GLuint obj);

const std::string GetShaderSourceFromFile(
    const char* filename,
    const std::string path = std::string(HOME_DATA_DIR "hardcoded_shaders/"));
const GLchar* GetShaderSourceFromTable(const char* filename);
const std::string GetShaderSource(const char* filename);
GLuint loadShaderFile(const char* filename, const unsigned long Type);

///@todo Audit these terrible variable names
GLuint makeShaderFromSource(
    const char* vertSrc,
    const char* fragSrc,
    const char* geomSrc=NULL);
GLuint makeShaderByName(const char* name);

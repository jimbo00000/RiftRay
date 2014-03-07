// GLUtils.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <GL/glew.h>
#include <stdio.h>

#include "GLUtils.h"

/// Error printing function called by CHECK_GL_ERROR_MACRO(),
/// which is only defined when _DEBUG is defined.
void CheckErrorGL(const char* file, const int line)
{
    GLenum errCode;
    const GLubyte *errString;

    if ((errCode = glGetError()) != GL_NO_ERROR)
    {
        fprintf(stderr, "GL Error in file '%s' in line %d :\n", file, line);
        errString = gluErrorString(errCode);
        fprintf(stderr, "%s\n", errString);
    }
}

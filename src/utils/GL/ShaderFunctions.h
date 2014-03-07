// ShaderFunctions.h

#ifndef _SHADER_FUNCTIONS_H_
#define _SHADER_FUNCTIONS_H_

GLint getUniLoc(const GLuint program, const GLchar *name);
void  printShaderInfoLog(GLuint obj);
void  printProgramInfoLog(GLuint obj);

const GLchar* GetShaderSourceFromFile(const char* filename);
const GLchar* GetShaderSource(const char* filename);
GLuint loadShaderFile(const char* filename, const unsigned long Type);
GLuint makeShaderByName(const char* name);

#endif //_SHADER_FUNCTIONS_H_

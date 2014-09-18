// GLUtils.h

#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

/// Error printing function called by CHECK_GL_ERROR_MACRO(),
/// which is only defined when _DEBUG is defined.
void CheckErrorGL(const char* file, const int line);

/// Debug macro - preprocessed to a no-op in release mode.
#ifdef _DEBUG
#define CHECK_GL_ERROR_MACRO() CheckErrorGL(__FILE__, __LINE__);
#else
#define CHECK_GL_ERROR_MACRO()
#endif


#endif // _GL_UTILS_H_
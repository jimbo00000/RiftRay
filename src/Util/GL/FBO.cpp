// FBO.cpp

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <GL/glew.h>
#include "FBO.h"

// FrameBuffer Object
// Allows rendering to texture.
void allocateFBO(FBO& f, int w, int h)
{
    // Delete old textures if they exist
    deallocateFBO(f);

    f.w = w;
    f.h = h;

    glGenFramebuffersEXT(1, &f.id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, f.id);
    
#if 0
    // Depth buffer render target
	glGenRenderbuffersEXT(1, &f.depth);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, f.depth);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                 GL_DEPTH_ATTACHMENT_EXT,
                                 GL_RENDERBUFFER_EXT,
                                 f.depth);
#else
    // Depth buffer texture target
    glGenTextures( 1, &f.depth );
    glBindTexture( GL_TEXTURE_2D, f.depth );
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY );
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     w, h, 0,
                     GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, f.depth, 0);
#endif


    // Texture render target
    glGenTextures(1, &f.tex);
    glBindTexture(GL_TEXTURE_2D, f.tex);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                              GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D,
                              f.tex, 0);

    // Check status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        //printf("BufferStructs.cpp: Framebuffer is incomplete with status %d\n", status);
        //assert(false);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void deallocateFBO(FBO& f)
{
    glDeleteFramebuffersEXT(1, &f.id), f.id = 0;
    glDeleteTextures(1, &f.tex), f.tex = 0;
#if 0
    glDeleteRenderbuffersEXT(1, &f.depth), f.depth = 0;
#else
    glDeleteTextures(1, &f.depth), f.depth = 0;
#endif
}

// Set viewport here, then restore it in unbind
void bindFBO(const FBO& f, float fboScale)
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, f.id);
    glPushAttrib(GL_VIEWPORT_BIT);

    // Add 1 to the viewport sizes here to mitigate the edge effects on the render buffer -
    // this way we render all the way out to the borders rather than leaving an unsightly gap.
    glViewport(
        0, 0,
        static_cast<int>(f.w * fboScale) + 1,
        static_cast<int>(f.h * fboScale) + 1);
}

void unbindFBO()
{
    glPopAttrib(); // GL_VIEWPORT_BIT - if this is not misused!
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
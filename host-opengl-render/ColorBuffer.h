#ifndef _LIBRENDER_COLORBUFFER_H
#define _LIBRENDER_COLORBUFFER_H

#include "egl.h"
#include "eglext.h"
#include "gl.h"
#include "smart_ptr.h"

class ColorBuffer
{
public:
    static ColorBuffer *create(int p_width, int p_height,
                               GLenum p_internalFormat);
    ~ColorBuffer();

    GLuint getGLTextureName() const { return m_tex; }
    GLuint getWidth() const { return m_width; }
    GLuint getHeight() const { return m_height; }
    GLenum getFormat() const { return m_internalFormat; }

    void subUpdate(int x, int y, int width, int height, GLenum p_format, GLenum p_type, void *pixels);
    bool post();
    bool bindToTexture();
    bool bindToRenderbuffer();
    bool blitFromCurrentReadBuffer(EGLImageKHR m_blitEGLImage);
    void readback(unsigned char* img);

private:
    ColorBuffer();
    void drawTexQuad();
    bool bind_fbo();  // binds a fbo which have this texture as render target

private:
    GLuint m_tex;
    GLuint m_blitTex;
    EGLImageKHR m_eglImage;
    GLuint m_width;
    GLuint m_height;
    GLuint m_fbo;
    GLenum m_internalFormat;
};

typedef emugl::SmartPtr<ColorBuffer> ColorBufferPtr;

#endif

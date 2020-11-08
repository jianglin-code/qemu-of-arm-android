#include <stdio.h>
#include "ColorBuffer.h"
#include "FrameBuffer.h"
#include "EGLDispatch.h"
#include "GLDispatch.h"
#include "ThreadInfo.h"
#include "GLutils.h"
#include "GL2Dispatch.h"
#include "TimeUtils.h"
#include "GLErrorLog.h"

ColorBuffer *ColorBuffer::create(int p_width, int p_height,
                                 GLenum p_internalFormat)
{
    FrameBuffer *fb = FrameBuffer::getFB();

    GLenum texInternalFormat = 0;

    switch(p_internalFormat) {
        case GL_RGB:
        case GL_RGB565_OES:
            texInternalFormat = GL_RGB;
            break;

        case GL_RGBA:
        case GL_RGB5_A1_OES:
        case GL_RGBA4_OES:
            texInternalFormat = GL_RGBA;
            break;

        default:
            return NULL;
            break;
    }

    if (!fb->bind_locked()) {
        return NULL;
    }

    ColorBuffer *cb = new ColorBuffer();

    cb->m_width = p_width;
    cb->m_height = p_height;
    cb->m_internalFormat = texInternalFormat; 

    s_gl.glGenTextures(1, &cb->m_tex);
    s_gl.glBindTexture(GL_TEXTURE_2D, cb->m_tex);
    s_gl.glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat, p_width, p_height, 0, texInternalFormat, GL_UNSIGNED_BYTE, NULL);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    s_gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    s_gl.glGenTextures(1,&cb->m_blitTex);
    s_gl.glBindTexture(GL_TEXTURE_2D, cb->m_blitTex);
    s_gl.glTexImage2D(GL_TEXTURE_2D, 0, texInternalFormat, p_width, p_height, 0, texInternalFormat, GL_UNSIGNED_BYTE, NULL);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    s_gl.glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    if (fb->getCaps().has_eglimage_texture_2d) {
        cb->m_eglImage = s_egl.eglCreateImageKHR(
                fb->getDisplay(),
                s_egl.eglGetCurrentContext(),
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)SafePointerFromUInt(cb->m_tex),
                0);
    }

    if (NULL == cb->m_eglImage)
    {
        printf("eglCreateImageKHR filed \n");
    }

    fb->unbind_locked();
    return cb;
}

ColorBuffer::ColorBuffer() :
    m_tex(0),
    m_blitTex(0),
    m_eglImage(NULL),
    m_fbo(0),
    m_internalFormat(0)
{
}

ColorBuffer::~ColorBuffer()
{
    FrameBuffer *fb = FrameBuffer::getFB();
    fb->bind_locked();

    if (m_eglImage) {
        s_egl.eglDestroyImageKHR(fb->getDisplay(), m_eglImage);
    }

    if (m_fbo) {
        s_gl.glDeleteFramebuffersOES(1, &m_fbo);
    }

    GLuint tex[1] = {m_blitTex};
    s_gl.glDeleteTextures(1, tex);

    m_tex=0;
    m_blitTex=0;

    fb->unbind_locked();
}

void ColorBuffer::subUpdate(int x, int y, int width, int height, GLenum p_format, GLenum p_type, void *pixels)
{
    FrameBuffer *fb = FrameBuffer::getFB();
    if (!fb->bind_locked()) return;
    s_gl.glBindTexture(GL_TEXTURE_2D, m_tex);
    s_gl.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    s_gl.glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
                         width, height, p_format, p_type, pixels);
    fb->unbind_locked();
}

bool ColorBuffer::blitFromCurrentReadBuffer(EGLImageKHR blitEGLImage)
{
    RenderThreadInfo *tInfo = RenderThreadInfo::get();
    if (!tInfo->currContext.Ptr()) {
        return false;
    }

    FrameBuffer *fb = FrameBuffer::getFB();
    if (fb->bind_locked()) {

        if (bind_fbo()) {

            GLint vport[4] = {0};
            s_gl.glGetIntegerv(GL_VIEWPORT, vport);
            s_gl.glViewport(0, 0, m_width, m_height);

            s_gl.glBindTexture(GL_TEXTURE_2D, m_blitTex);
            s_gl.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, blitEGLImage);
            s_gl.glEnable(GL_TEXTURE_2D);
            s_gl.glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            drawTexQuad(); 

            s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);

            s_gl.glViewport(vport[0], vport[1], vport[2], vport[3]);
        }

        fb->unbind_locked();
    }

    return true;
}

bool ColorBuffer::bindToTexture()
{
    if (m_eglImage) {
        RenderThreadInfo *tInfo = RenderThreadInfo::get();
        if (tInfo->currContext.Ptr()) {
#ifdef WITH_GLES2
            if (tInfo->currContext->isGL2()) {
                s_gl2.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_eglImage);
            }
            else {
                s_gl.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_eglImage);
            }
#else
            s_gl.glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, m_eglImage);
#endif
            return true;
        }
    }
    return false;
}

bool ColorBuffer::bindToRenderbuffer()
{
    if (m_eglImage) {
        RenderThreadInfo *tInfo = RenderThreadInfo::get();
        if (tInfo->currContext.Ptr()) {
#ifdef WITH_GLES2
            if (tInfo->currContext->isGL2()) {
                s_gl2.glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER_OES, m_eglImage);
            }
            else {
                s_gl.glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER_OES, m_eglImage);
            }
#else
            s_gl.glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER_OES, m_eglImage);
#endif
            return true;
        }
    }
    return false;
}

bool ColorBuffer::bind_fbo()
{
    if (m_fbo) {
        // fbo already exist - just bind
        s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_fbo);
        return true;
    }

    s_gl.glGenFramebuffersOES(1, &m_fbo);
    s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_fbo);
    s_gl.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES,
                                   GL_COLOR_ATTACHMENT0_OES,
                                   GL_TEXTURE_2D, m_tex, 0);
    GLenum status = s_gl.glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
    if (status != GL_FRAMEBUFFER_COMPLETE_OES) {
        ERR("ColorBuffer::bind_fbo: FBO not complete: %#x\n", status);
        s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
        s_gl.glDeleteFramebuffersOES(1, &m_fbo);
        m_fbo = 0;
        return false;
    }

    return true;
}

bool ColorBuffer::post()
{
    s_gl.glBindTexture(GL_TEXTURE_2D, m_tex);
    s_gl.glEnable(GL_TEXTURE_2D);
    s_gl.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    drawTexQuad();

    return true;
}

void ColorBuffer::drawTexQuad()
{
    GLfloat verts[] = {  -1.0f, -1.0f, 0.0f,
                         -1.0f, +1.0f, 0.0f,
                         +1.0f, -1.0f, 0.0f,
                         +1.0f, +1.0f, 0.0f };

    GLfloat tcoords[] = { 0.0f,           1,
                          0.0f,           0.0f,
                          1, 1,
                          1, 0.0f };

    s_gl.glClientActiveTexture(GL_TEXTURE0);
    s_gl.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    s_gl.glTexCoordPointer(2, GL_FLOAT, 0, tcoords);

    s_gl.glEnableClientState(GL_VERTEX_ARRAY);
    s_gl.glVertexPointer(3, GL_FLOAT, 0, verts);
    s_gl.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ColorBuffer::readback(unsigned char* img)
{
    FrameBuffer *fb = FrameBuffer::getFB();
    if (fb->bind_locked()) {
        if (bind_fbo()) {
            s_gl.glReadPixels(0, 0, m_width, m_height,
                    m_internalFormat, GL_UNSIGNED_BYTE, img);

            s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
        }
        fb->unbind_locked();
    }
}

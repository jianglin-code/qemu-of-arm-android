/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "WindowSurface.h"
#include "FBConfig.h"
#include "FrameBuffer.h"
#include "glext.h"
#include "EGLDispatch.h"
#include "GLDispatch.h"
#include "GL2Dispatch.h"
#include <stdio.h>
#include <string.h>
#include "TimeUtils.h"
#include "GLErrorLog.h"
#include "GLutils.h"
#include "ThreadInfo.h"

WindowSurface::WindowSurface() :
    m_eglSurface(NULL),
    m_attachedColorBuffer(NULL),
    m_readContext(NULL),
    m_drawContext(NULL),
    m_width(0),
    m_height(0),
    m_pbufWidth(0),
    m_pbufHeight(0),
    m_fbo(0),
    m_blitTex(0),
    m_blitEGLImage(NULL)
{
}

WindowSurface::~WindowSurface()
{
    s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
    m_blitEGLImage = 0;
    m_blitTex = 0;

    s_egl.eglDestroySurface(FrameBuffer::getFB()->getDisplay(), m_eglSurface);
}

WindowSurface *WindowSurface::create(int p_config, int p_width, int p_height)
{
    const FBConfig *fbconf = FBConfig::get(p_config);
    if (!fbconf) {
        return NULL;
    }

    // allocate space for the WindowSurface object
    WindowSurface *win = new WindowSurface();
    if (!win) {
        return NULL;
    }
    win->m_fbconf = fbconf;

    //
    // Create a pbuffer to be used as the egl surface
    // for that window.
    //
    if (!win->resizePbuffer(p_width, p_height)) {
        delete win;
        return NULL;
    }

    win->m_width = p_width;
    win->m_height = p_height;
    return win;
}

//
// flushColorBuffer - The function makes sure that the
//    previous attached color buffer is updated, if copy or blit should be done
//    in order to update it - it is being done here.
//
bool WindowSurface::flushColorBuffer()
{
    if (m_attachedColorBuffer.Ptr() != NULL) {
        return blitToColorBuffer();
    }
    return true;
}

//
// setColorBuffer - this function is called when a new color buffer needs to
//    be attached to the surface. The function doesn't make sure that the
//    previous attached color buffer is updated, this is done by flushColorBuffer
//
void WindowSurface::setColorBuffer(ColorBufferPtr p_colorBuffer)
{
    m_attachedColorBuffer = p_colorBuffer;

    //
    // resize the window if the attached color buffer is of different
    // size
    //
    unsigned int cbWidth = m_attachedColorBuffer->getWidth();
    unsigned int cbHeight = m_attachedColorBuffer->getHeight();

    if (cbWidth != m_width || cbHeight != m_height) {

        if (m_pbufWidth && m_pbufHeight) {
            // if we use pbuffer, need to resize it
            resizePbuffer(cbWidth, cbHeight);
        }

        m_width = cbWidth;
        m_height = cbHeight;
    }
}

//
// This function is called after the context and eglSurface is already
// bound in the current thread (eglMakeCurrent has been called).
// This function should take actions required on the other surface objects
// when being bind/unbound
//
void WindowSurface::bind(RenderContextPtr p_ctx, SurfaceBindType p_bindType)
{
    if (p_bindType == SURFACE_BIND_READ) {
        m_readContext = p_ctx;
    }
    else if (p_bindType == SURFACE_BIND_DRAW) {
        m_drawContext = p_ctx;
    }
    else if (p_bindType == SURFACE_BIND_READDRAW) {
        m_readContext = p_ctx;
        m_drawContext = p_ctx;
    }
    else {
        return;  // bad param
    }

    bind_fbo();
}

bool WindowSurface::blitToColorBuffer()
{
    if ( 0==m_width || 0==m_height || 0 == m_blitEGLImage) return false;

    // if (m_attachedColorBuffer->getWidth() != m_width ||
    //     m_attachedColorBuffer->getHeight() != m_height) {
    //     // XXX: should never happen - how this needs to be handled?
    //     fprintf(stderr, "Dimensions do not match\n");
    //     return false;
    // }

    //
    // Make the surface current
    //
    // EGLContext prevContext = s_egl.eglGetCurrentContext();
    // EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
    // EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);
    // FrameBuffer *fb = FrameBuffer::getFB();
    // if (!m_drawContext.Ptr()) {
    //     fprintf(stderr, "Draw context is NULL\n");
    //     return false;
    // }

    // if (!s_egl.eglMakeCurrent(fb->getDisplay(), m_eglSurface,  m_eglSurface, m_drawContext->getEGLContext())) {
    //     fprintf(stderr, "Error making draw context current\n");
    //     return false;
    // }

    //long long t0 = GetCurrentTimeMS();
    unbind_fbo();
    m_attachedColorBuffer->blitFromCurrentReadBuffer(m_blitEGLImage);
    bind_fbo();
    //long long t1 = GetCurrentTimeMS();
    //float temp = (float)(t1-t0)/1000;
    //printf("blitToColorBuffer:%lld ,%lld ,%5.5f\n",t0,t1,temp);

    // restore current context/surface
    //s_egl.eglMakeCurrent(fb->getDisplay(), prevDrawSurf, prevReadSurf, prevContext);
    return true;
}

bool WindowSurface::resizePbuffer(unsigned int p_width, unsigned int p_height)
{
    if (m_eglSurface &&
        m_pbufWidth == p_width &&
        m_pbufHeight == p_height) {
        // no need to resize
        return true;
    }

    FrameBuffer *fb = FrameBuffer::getFB();

    EGLContext prevContext = s_egl.eglGetCurrentContext();
    EGLSurface prevReadSurf = s_egl.eglGetCurrentSurface(EGL_READ);
    EGLSurface prevDrawSurf = s_egl.eglGetCurrentSurface(EGL_DRAW);
    EGLSurface prevPbuf = m_eglSurface;
    bool needRebindContext = m_eglSurface &&
                             (prevReadSurf == m_eglSurface ||
                              prevDrawSurf == m_eglSurface);

    if (needRebindContext) {
        s_egl.eglMakeCurrent(fb->getDisplay(), EGL_NO_SURFACE,
                              EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    //
    // Destroy previous surface
    //
    if (m_eglSurface) {
        s_egl.eglDestroySurface(fb->getDisplay(), m_eglSurface);
        m_eglSurface = NULL;
    }

    //
    // Create pbuffer surface.
    //
    EGLint pbufAttribs[5];
    pbufAttribs[0] = EGL_WIDTH;
    pbufAttribs[1] = p_width;
    pbufAttribs[2] = EGL_HEIGHT;
    pbufAttribs[3] = p_height;
    pbufAttribs[4] = EGL_NONE;

    m_eglSurface = s_egl.eglCreatePbufferSurface(fb->getDisplay(),m_fbconf->getEGLConfig(),pbufAttribs);

    if (m_eglSurface == EGL_NO_SURFACE) {
        fprintf(stderr, "Renderer error: failed to create/resize pbuffer!!\n");
        return false;
    }

    m_pbufWidth = p_width;
    m_pbufHeight = p_height;

    if (needRebindContext) {
        s_egl.eglMakeCurrent(fb->getDisplay(), 
                     (prevDrawSurf==prevPbuf) ? m_eglSurface : prevDrawSurf,
                     (prevReadSurf==prevPbuf) ? m_eglSurface : prevReadSurf,
                     prevContext);
    }

    return true;
}

void WindowSurface::unbind_fbo()
{
    RenderThreadInfo *tInfo = RenderThreadInfo::get();
    if (!tInfo->currContext.Ptr()) {
        return ;
    }
    bool isGl2 = tInfo->currContext->isGL2();


    if(isGl2)
        s_gl2.glBindFramebuffer(GL_FRAMEBUFFER, 0);
    else
        s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
}

void WindowSurface::bind_fbo()
{
    RenderThreadInfo *tInfo = RenderThreadInfo::get();
    if (!tInfo->currContext.Ptr()) {
        return ;
    }
    bool isGl2 = tInfo->currContext->isGL2();

    if (m_fbo) {
        if(isGl2)
            s_gl2.glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        else
            s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_fbo);
        
        return ;
    }

    if(isGl2)
        create_fbo_api2();
    else
        create_fbo_api1();
}

void WindowSurface::create_fbo_api1()
{
    if(0 != m_fbo){
        s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
        s_gl.glDeleteFramebuffersOES(1, &m_fbo);
        m_fbo = 0;

        s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
        m_blitEGLImage =0;
        m_blitTex=0;
        return ;
    }

    if(NULL == m_attachedColorBuffer.constPtr()) 
        return ;

    GLint currTexBind;
    s_gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &currTexBind);
    s_gl.glGenTextures(1, &m_blitTex);
    s_gl.glBindTexture(GL_TEXTURE_2D, m_blitTex);
    s_gl.glTexImage2D(GL_TEXTURE_2D, 0, m_attachedColorBuffer->getFormat(), m_attachedColorBuffer->getWidth(), m_attachedColorBuffer->getHeight(), 0, m_attachedColorBuffer->getFormat(), GL_UNSIGNED_BYTE, NULL);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    s_gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    s_gl.glBindTexture(GL_TEXTURE_2D, currTexBind);

    m_blitEGLImage = s_egl.eglCreateImageKHR(FrameBuffer::getFB()->getDisplay(),
                s_egl.eglGetCurrentContext(),
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)SafePointerFromUInt(m_blitTex),
                0);

    if(NULL == m_blitEGLImage){
        GLuint tex[1] = {m_blitTex};
        s_gl.glDeleteTextures(1, tex);
        m_blitTex = 0;

        printf("WindowSurface::create_fboApi_1 eglCreateImageKHR failed \n");
        return;
    }

    s_gl.glGenFramebuffersOES(1, &m_fbo);
    if(0 == m_fbo){
        s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
        m_blitEGLImage = 0;
        m_blitTex = 0;
        return ;
    }

    s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_fbo);
    s_gl.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES,
                                   GL_COLOR_ATTACHMENT0_OES,
                                   GL_TEXTURE_2D, m_blitTex, 0);
    GLenum status = s_gl.glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
    if (status != GL_FRAMEBUFFER_COMPLETE_OES) {
        s_gl.glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
        s_gl.glDeleteFramebuffersOES(1, &m_fbo);
        m_fbo = 0;

        s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
        m_blitEGLImage = 0;
        m_blitTex = 0;

        printf("WindowSurface::create_fboApi_1: FBO not complete: %#x\n", status);
    }
}

void WindowSurface::create_fbo_api2()
{
    if(0 != m_fbo){
        s_gl2.glBindFramebuffer(GL_FRAMEBUFFER, 0);
        s_gl2.glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;

        s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
        m_blitEGLImage =0;
        m_blitTex=0;
        return ;
    }

    if(NULL == m_attachedColorBuffer.constPtr()) 
        return ;

    GLint currTexBind;
    s_gl2.glGetIntegerv(GL_TEXTURE_BINDING_2D, &currTexBind);
    s_gl2.glGenTextures(1, &m_blitTex);
    s_gl2.glBindTexture(GL_TEXTURE_2D, m_blitTex);
    s_gl2.glTexImage2D(GL_TEXTURE_2D, 0, m_attachedColorBuffer->getFormat(), m_attachedColorBuffer->getWidth(), m_attachedColorBuffer->getHeight(), 0, m_attachedColorBuffer->getFormat(), GL_UNSIGNED_BYTE, NULL);
    s_gl2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    s_gl2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    s_gl2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    s_gl2.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    s_gl2.glBindTexture(GL_TEXTURE_2D, currTexBind);

    m_blitEGLImage = s_egl.eglCreateImageKHR(FrameBuffer::getFB()->getDisplay(),
                s_egl.eglGetCurrentContext(),
                EGL_GL_TEXTURE_2D_KHR,
                (EGLClientBuffer)SafePointerFromUInt(m_blitTex),
                0);

    if(NULL == m_blitEGLImage){
        GLuint tex[1] = {m_blitTex};
        s_gl2.glDeleteTextures(1, tex);
        m_blitTex = 0;

        printf("WindowSurface::create_fboApi_2 eglCreateImageKHR failed \n");
        return;
    }

    s_gl2.glGenFramebuffers(1, &m_fbo);
    if(0 == m_fbo){
        s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
        m_blitEGLImage = 0;
        m_blitTex = 0;
        return ;
    }

    s_gl2.glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    s_gl2.glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, m_blitTex, 0);
    GLenum status = s_gl2.glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        s_gl2.glBindFramebuffer(GL_FRAMEBUFFER, 0);
        s_gl2.glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;

        s_egl.eglDestroyImageKHR(FrameBuffer::getFB()->getDisplay(), m_blitEGLImage);
        m_blitEGLImage = 0;
        m_blitTex = 0;

        printf("WindowSurface::create_fboApi_2: FBO not complete: %#x\n", status);
    }
}



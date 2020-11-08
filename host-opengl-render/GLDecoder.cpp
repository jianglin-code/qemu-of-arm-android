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

#include "GLDecoder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "egl.h"
#include "gl.h"
#include "glext.h"
#include "gl_opcodes.h"
#include "GLDispatch.h"

typedef unsigned int tsize_t; // Target "size_t", which is 32-bit for now. It may or may not be the same as host's size_t when emugen is compiled.

//#undef DEBUG_PRINTOUT
//#undef CHECK_GL_ERROR

static inline void* SafePointerFromUInt(GLuint value) {
  return (void*)(uintptr_t)value;
}

GLDecoder::GLDecoder()
{
    m_contextData = NULL;
}

GLDecoder::~GLDecoder()
{
}

int GLDecoder::s_glFinishRoundTrip(void *self)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glFinish();
    return 0;
}

void GLDecoder::s_glVertexPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glVertexPointer(size, type, stride, SafePointerFromUInt(offset));
}

void GLDecoder::s_glColorPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glColorPointer(size, type, stride, SafePointerFromUInt(offset));
}

void GLDecoder::s_glTexCoordPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glTexCoordPointer(size, type, stride, SafePointerFromUInt(offset));
}

void GLDecoder::s_glNormalPointerOffset(void *self, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glNormalPointer(type, stride, SafePointerFromUInt(offset));
}

void GLDecoder::s_glPointSizePointerOffset(void *self, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glPointSizePointerOES(type, stride, SafePointerFromUInt(offset));
}

void GLDecoder::s_glWeightPointerOffset(void * self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glWeightPointerOES(size, type, stride, SafePointerFromUInt(offset));
}

void GLDecoder::s_glMatrixIndexPointerOffset(void * self, GLint size, GLenum type, GLsizei stride, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glMatrixIndexPointerOES(size, type, stride, SafePointerFromUInt(offset));
}



#define STORE_POINTER_DATA_OR_ABORT(location)    \
    if (ctx->m_contextData != NULL) {   \
        ctx->m_contextData->storePointerData((location), data, datalen); \
    } else { \
        return; \
    }

void GLDecoder::s_glVertexPointerData(void *self, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::VERTEX_LOCATION);

    s_gl.glVertexPointer(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::VERTEX_LOCATION));
}

void GLDecoder::s_glColorPointerData(void *self, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::COLOR_LOCATION);

    s_gl.glColorPointer(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::COLOR_LOCATION));
}

void GLDecoder::s_glTexCoordPointerData(void *self, GLint unit, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;
    STORE_POINTER_DATA_OR_ABORT((GLDecoderContextData::PointerDataLocation)
                                (GLDecoderContextData::TEXCOORD0_LOCATION + unit));

    s_gl.glTexCoordPointer(size, type, 0,
                           ctx->m_contextData->pointerData((GLDecoderContextData::PointerDataLocation)
                                                           (GLDecoderContextData::TEXCOORD0_LOCATION + unit)));
}

void GLDecoder::s_glNormalPointerData(void *self, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::NORMAL_LOCATION);

    s_gl.glNormalPointer(type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::NORMAL_LOCATION));
}

void GLDecoder::s_glPointSizePointerData(void *self, GLenum type, GLsizei stride, void *data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::POINTSIZE_LOCATION);

    s_gl.glPointSizePointerOES(type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::POINTSIZE_LOCATION));
}

void GLDecoder::s_glWeightPointerData(void * self, GLint size, GLenum type, GLsizei stride, void * data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::WEIGHT_LOCATION);

    s_gl.glWeightPointerOES(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::WEIGHT_LOCATION));
}

void GLDecoder::s_glMatrixIndexPointerData(void * self, GLint size, GLenum type, GLsizei stride, void * data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;

    STORE_POINTER_DATA_OR_ABORT(GLDecoderContextData::MATRIXINDEX_LOCATION);

    s_gl.glMatrixIndexPointerOES(size, type, 0, ctx->m_contextData->pointerData(GLDecoderContextData::MATRIXINDEX_LOCATION));
}

void GLDecoder::s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glDrawElements(mode, count, type, SafePointerFromUInt(offset));
}

void GLDecoder::s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen)
{
    GLDecoder *ctx = (GLDecoder *)self;
    s_gl.glDrawElements(mode, count, type, data);
}

void GLDecoder::s_glGetCompressedTextureFormats(void *self, GLint count, GLint *data)
{
    GLDecoder *ctx = (GLDecoder *) self;
    s_gl.glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, data);
}

size_t GLDecoder::decode(void *buf, size_t len, IOStream *stream)
{
                           
    size_t pos = 0;
    if (len < 8) return pos; 
    unsigned char *ptr = (unsigned char *)buf;
    bool unknownOpcode = false;  
#ifdef CHECK_GL_ERROR 
    char lastCall[256] = {0}; 
#endif 
    while ((len - pos >= 8) && !unknownOpcode) {   
        int opcode = *(int *)ptr;   
        unsigned int packetLen = *(int *)(ptr + 4);
        if (len - pos < packetLen)  return pos; 
        switch(opcode) {
            case OP_glAlphaFunc:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glAlphaFunc(0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
#endif
            s_gl.glAlphaFunc(*(GLenum *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glAlphaFunc");
#endif
            break;
            case OP_glClearColor:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearColor(%f %f %f %f )\n", stream,*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4), *(GLclampf *)(ptr + 8 + 4 + 4), *(GLclampf *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glClearColor(*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4), *(GLclampf *)(ptr + 8 + 4 + 4), *(GLclampf *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearColor");
#endif
            break;
            case OP_glClearDepthf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearDepthf(%f )\n", stream,*(GLclampf *)(ptr + 8));
#endif
            s_gl.glClearDepthf(*(GLclampf *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearDepthf");
#endif
            break;
            case OP_glClipPlanef:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClipPlanef(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glClipPlanef(*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClipPlanef");
#endif
            break;
            case OP_glColor4f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColor4f(%f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glColor4f(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColor4f");
#endif
            break;
            case OP_glDepthRangef:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDepthRangef(%f %f )\n", stream,*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
#endif
            s_gl.glDepthRangef(*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthRangef");
#endif
            break;
            case OP_glFogf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFogf(0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl.glFogf(*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFogf");
#endif
            break;
            case OP_glFogfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFogfv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glFogfv(*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFogfv");
#endif
            break;
            case OP_glFrustumf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFrustumf(%f %f %f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glFrustumf(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFrustumf");
#endif
            break;
            case OP_glGetClipPlanef:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetClipPlanef(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetClipPlanef(*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetClipPlanef");
#endif
            break;
            case OP_glGetFloatv:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetFloatv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetFloatv(*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFloatv");
#endif
            break;
            case OP_glGetLightfv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetLightfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetLightfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetLightfv");
#endif
            break;
            case OP_glGetMaterialfv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetMaterialfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetMaterialfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetMaterialfv");
#endif
            break;
            case OP_glGetTexEnvfv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexEnvfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexEnvfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexEnvfv");
#endif
            break;
            case OP_glGetTexParameterfv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexParameterfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexParameterfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexParameterfv");
#endif
            break;
            case OP_glLightModelf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightModelf(0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl.glLightModelf(*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightModelf");
#endif
            break;
            case OP_glLightModelfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightModelfv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glLightModelfv(*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightModelfv");
#endif
            break;
            case OP_glLightf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightf(0x%08x 0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glLightf(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightf");
#endif
            break;
            case OP_glLightfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glLightfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightfv");
#endif
            break;
            case OP_glLineWidth:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLineWidth(%f )\n", stream,*(GLfloat *)(ptr + 8));
#endif
            s_gl.glLineWidth(*(GLfloat *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLineWidth");
#endif
            break;
            case OP_glLoadMatrixf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLoadMatrixf(%p(%u) )\n", stream,(const GLfloat*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glLoadMatrixf((const GLfloat*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLoadMatrixf");
#endif
            break;
            case OP_glMaterialf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMaterialf(0x%08x 0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glMaterialf(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMaterialf");
#endif
            break;
            case OP_glMaterialfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMaterialfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glMaterialfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMaterialfv");
#endif
            break;
            case OP_glMultMatrixf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultMatrixf(%p(%u) )\n", stream,(const GLfloat*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glMultMatrixf((const GLfloat*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultMatrixf");
#endif
            break;
            case OP_glMultiTexCoord4f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiTexCoord4f(0x%08x %f %f %f %f )\n", stream,*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glMultiTexCoord4f(*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiTexCoord4f");
#endif
            break;
            case OP_glNormal3f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glNormal3f(%f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glNormal3f(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glNormal3f");
#endif
            break;
            case OP_glOrthof:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glOrthof(%f %f %f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glOrthof(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glOrthof");
#endif
            break;
            case OP_glPointParameterf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointParameterf(0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl.glPointParameterf(*(GLenum *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointParameterf");
#endif
            break;
            case OP_glPointParameterfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointParameterfv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glPointParameterfv(*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointParameterfv");
#endif
            break;
            case OP_glPointSize:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointSize(%f )\n", stream,*(GLfloat *)(ptr + 8));
#endif
            s_gl.glPointSize(*(GLfloat *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointSize");
#endif
            break;
            case OP_glPolygonOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPolygonOffset(%f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl.glPolygonOffset(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPolygonOffset");
#endif
            break;
            case OP_glRotatef:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glRotatef(%f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glRotatef(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glRotatef");
#endif
            break;
            case OP_glScalef:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glScalef(%f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glScalef(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glScalef");
#endif
            break;
            case OP_glTexEnvf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvf(0x%08x 0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvf(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvf");
#endif
            break;
            case OP_glTexEnvfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvfv");
#endif
            break;
            case OP_glTexParameterf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameterf(0x%08x 0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameterf(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterf");
#endif
            break;
            case OP_glTexParameterfv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameterfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameterfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterfv");
#endif
            break;
            case OP_glTranslatef:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTranslatef(%f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTranslatef(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTranslatef");
#endif
            break;
            case OP_glActiveTexture:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glActiveTexture(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glActiveTexture(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glActiveTexture");
#endif
            break;
            case OP_glAlphaFuncx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glAlphaFuncx(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
#endif
            s_gl.glAlphaFuncx(*(GLenum *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glAlphaFuncx");
#endif
            break;
            case OP_glBindBuffer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBindBuffer(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl.glBindBuffer(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindBuffer");
#endif
            break;
            case OP_glBindTexture:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBindTexture(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl.glBindTexture(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindTexture");
#endif
            break;
            case OP_glBlendFunc:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBlendFunc(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl.glBlendFunc(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendFunc");
#endif
            break;
            case OP_glBufferData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBufferData(0x%08x %p %p(%u) 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLsizeiptr *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl.glBufferData(*(GLenum *)(ptr + 8), *(GLsizeiptr *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBufferData");
#endif
            break;
            case OP_glBufferSubData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBufferSubData(0x%08x %p %p %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLintptr *)(ptr + 8 + 4), *(GLsizeiptr *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glBufferSubData(*(GLenum *)(ptr + 8), *(GLintptr *)(ptr + 8 + 4), *(GLsizeiptr *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBufferSubData");
#endif
            break;
            case OP_glClear:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClear(0x%08x )\n", stream,*(GLbitfield *)(ptr + 8));
#endif
            s_gl.glClear(*(GLbitfield *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClear");
#endif
            break;
            case OP_glClearColorx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearColorx(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4), *(GLclampx *)(ptr + 8 + 4 + 4), *(GLclampx *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glClearColorx(*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4), *(GLclampx *)(ptr + 8 + 4 + 4), *(GLclampx *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearColorx");
#endif
            break;
            case OP_glClearDepthx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearDepthx(0x%08x )\n", stream,*(GLclampx *)(ptr + 8));
#endif
            s_gl.glClearDepthx(*(GLclampx *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearDepthx");
#endif
            break;
            case OP_glClearStencil:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearStencil(%d )\n", stream,*(GLint *)(ptr + 8));
#endif
            s_gl.glClearStencil(*(GLint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearStencil");
#endif
            break;
            case OP_glClientActiveTexture:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClientActiveTexture(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glClientActiveTexture(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClientActiveTexture");
#endif
            break;
            case OP_glColor4ub:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColor4ub(0x%02x 0x%02x 0x%02x 0x%02x )\n", stream,*(GLubyte *)(ptr + 8), *(GLubyte *)(ptr + 8 + 1), *(GLubyte *)(ptr + 8 + 1 + 1), *(GLubyte *)(ptr + 8 + 1 + 1 + 1));
#endif
            s_gl.glColor4ub(*(GLubyte *)(ptr + 8), *(GLubyte *)(ptr + 8 + 1), *(GLubyte *)(ptr + 8 + 1 + 1), *(GLubyte *)(ptr + 8 + 1 + 1 + 1));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColor4ub");
#endif
            break;
            case OP_glColor4x:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColor4x(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glColor4x(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColor4x");
#endif
            break;
            case OP_glColorMask:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColorMask(%d %d %d %d )\n", stream,*(GLboolean *)(ptr + 8), *(GLboolean *)(ptr + 8 + 1), *(GLboolean *)(ptr + 8 + 1 + 1), *(GLboolean *)(ptr + 8 + 1 + 1 + 1));
#endif
            s_gl.glColorMask(*(GLboolean *)(ptr + 8), *(GLboolean *)(ptr + 8 + 1), *(GLboolean *)(ptr + 8 + 1 + 1), *(GLboolean *)(ptr + 8 + 1 + 1 + 1));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColorMask");
#endif
            break;
            case OP_glColorPointer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColorPointer(%d 0x%08x %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glColorPointer(*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColorPointer");
#endif
            break;
            case OP_glCompressedTexImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCompressedTexImage2D(0x%08x %d 0x%08x %d %d %d %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glCompressedTexImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *((unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4)) == 0 ? NULL : (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCompressedTexImage2D");
#endif
            break;
            case OP_glCompressedTexSubImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCompressedTexSubImage2D(0x%08x %d %d %d %d %d 0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glCompressedTexSubImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCompressedTexSubImage2D");
#endif
            break;
            case OP_glCopyTexImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCopyTexImage2D(0x%08x %d 0x%08x %d %d %d %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glCopyTexImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCopyTexImage2D");
#endif
            break;
            case OP_glCopyTexSubImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCopyTexSubImage2D(0x%08x %d %d %d %d %d %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glCopyTexSubImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCopyTexSubImage2D");
#endif
            break;
            case OP_glCullFace:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCullFace(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glCullFace(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCullFace");
#endif
            break;
            case OP_glDeleteBuffers:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDeleteBuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glDeleteBuffers(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteBuffers");
#endif
            break;
            case OP_glDeleteTextures:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDeleteTextures(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glDeleteTextures(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteTextures");
#endif
            break;
            case OP_glDepthFunc:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDepthFunc(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glDepthFunc(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthFunc");
#endif
            break;
            case OP_glDepthMask:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDepthMask(%d )\n", stream,*(GLboolean *)(ptr + 8));
#endif
            s_gl.glDepthMask(*(GLboolean *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthMask");
#endif
            break;
            case OP_glDepthRangex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDepthRangex(0x%08x 0x%08x )\n", stream,*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
#endif
            s_gl.glDepthRangex(*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthRangex");
#endif
            break;
            case OP_glDisable:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDisable(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glDisable(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDisable");
#endif
            break;
            case OP_glDisableClientState:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDisableClientState(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glDisableClientState(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDisableClientState");
#endif
            break;
            case OP_glDrawArrays:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawArrays(0x%08x %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glDrawArrays(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawArrays");
#endif
            break;
            case OP_glDrawElements:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawElements(0x%08x %d 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glDrawElements(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawElements");
#endif
            break;
            case OP_glEnable:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glEnable(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glEnable(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEnable");
#endif
            break;
            case OP_glEnableClientState:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glEnableClientState(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glEnableClientState(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEnableClientState");
#endif
            break;
            case OP_glFinish:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFinish()\n", stream);
#endif
            s_gl.glFinish();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFinish");
#endif
            break;
            case OP_glFlush:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFlush()\n", stream);
#endif
            s_gl.glFlush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFlush");
#endif
            break;
            case OP_glFogx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFogx(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glFogx(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFogx");
#endif
            break;
            case OP_glFogxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFogxv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glFogxv(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFogxv");
#endif
            break;
            case OP_glFrontFace:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFrontFace(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glFrontFace(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFrontFace");
#endif
            break;
            case OP_glFrustumx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFrustumx(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glFrustumx(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFrustumx");
#endif
            break;
            case OP_glGetBooleanv:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetBooleanv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLboolean*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetBooleanv(*(GLenum *)(ptr + 8), (GLboolean*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetBooleanv");
#endif
            break;
            case OP_glGetBufferParameteriv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetBufferParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetBufferParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetBufferParameteriv");
#endif
            break;
            case OP_glClipPlanex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClipPlanex(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glClipPlanex(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClipPlanex");
#endif
            break;
            case OP_glGenBuffers:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenBuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGenBuffers(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenBuffers");
#endif
            break;
            case OP_glGenTextures:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenTextures(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGenTextures(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenTextures");
#endif
            break;
            case OP_glGetError:
            {
            size_t totalTmpSize = sizeof(GLenum);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetError()\n", stream);
#endif
            *(GLenum *)(&tmpBuf[0]) =           s_gl.glGetError();
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetError");
#endif
            break;
            case OP_glGetFixedv:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetFixedv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetFixedv(*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFixedv");
#endif
            break;
            case OP_glGetIntegerv:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetIntegerv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetIntegerv(*(GLenum *)(ptr + 8), (GLint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetIntegerv");
#endif
            break;
            case OP_glGetLightxv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetLightxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetLightxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetLightxv");
#endif
            break;
            case OP_glGetMaterialxv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetMaterialxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetMaterialxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetMaterialxv");
#endif
            break;
            case OP_glGetPointerv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetPointerv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLvoid**)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetPointerv(*(GLenum *)(ptr + 8), (GLvoid**)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPointerv");
#endif
            break;
            case OP_glGetString:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetString(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glGetString(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetString");
#endif
            break;
            case OP_glGetTexEnviv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexEnviv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexEnviv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexEnviv");
#endif
            break;
            case OP_glGetTexEnvxv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexEnvxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexEnvxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexEnvxv");
#endif
            break;
            case OP_glGetTexParameteriv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexParameteriv");
#endif
            break;
            case OP_glGetTexParameterxv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexParameterxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexParameterxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexParameterxv");
#endif
            break;
            case OP_glHint:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glHint(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl.glHint(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glHint");
#endif
            break;
            case OP_glIsBuffer:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsBuffer(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsBuffer(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsBuffer");
#endif
            break;
            case OP_glIsEnabled:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsEnabled(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsEnabled(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsEnabled");
#endif
            break;
            case OP_glIsTexture:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsTexture(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsTexture(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsTexture");
#endif
            break;
            case OP_glLightModelx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightModelx(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glLightModelx(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightModelx");
#endif
            break;
            case OP_glLightModelxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightModelxv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glLightModelxv(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightModelxv");
#endif
            break;
            case OP_glLightx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightx(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glLightx(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightx");
#endif
            break;
            case OP_glLightxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glLightxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightxv");
#endif
            break;
            case OP_glLineWidthx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLineWidthx(0x%08x )\n", stream,*(GLfixed *)(ptr + 8));
#endif
            s_gl.glLineWidthx(*(GLfixed *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLineWidthx");
#endif
            break;
            case OP_glLoadIdentity:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLoadIdentity()\n", stream);
#endif
            s_gl.glLoadIdentity();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLoadIdentity");
#endif
            break;
            case OP_glLoadMatrixx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLoadMatrixx(%p(%u) )\n", stream,(const GLfixed*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glLoadMatrixx((const GLfixed*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLoadMatrixx");
#endif
            break;
            case OP_glLogicOp:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLogicOp(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glLogicOp(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLogicOp");
#endif
            break;
            case OP_glMaterialx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMaterialx(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glMaterialx(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMaterialx");
#endif
            break;
            case OP_glMaterialxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMaterialxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glMaterialxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMaterialxv");
#endif
            break;
            case OP_glMatrixMode:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMatrixMode(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glMatrixMode(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMatrixMode");
#endif
            break;
            case OP_glMultMatrixx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultMatrixx(%p(%u) )\n", stream,(const GLfixed*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glMultMatrixx((const GLfixed*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultMatrixx");
#endif
            break;
            case OP_glMultiTexCoord4x:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiTexCoord4x(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glMultiTexCoord4x(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiTexCoord4x");
#endif
            break;
            case OP_glNormal3x:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glNormal3x(0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glNormal3x(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glNormal3x");
#endif
            break;
            case OP_glNormalPointer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glNormalPointer(0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glNormalPointer(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glNormalPointer");
#endif
            break;
            case OP_glOrthox:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glOrthox(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glOrthox(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glOrthox");
#endif
            break;
            case OP_glPixelStorei:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPixelStorei(0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4));
#endif
            s_gl.glPixelStorei(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPixelStorei");
#endif
            break;
            case OP_glPointParameterx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointParameterx(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glPointParameterx(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointParameterx");
#endif
            break;
            case OP_glPointParameterxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointParameterxv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glPointParameterxv(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointParameterxv");
#endif
            break;
            case OP_glPointSizex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointSizex(0x%08x )\n", stream,*(GLfixed *)(ptr + 8));
#endif
            s_gl.glPointSizex(*(GLfixed *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointSizex");
#endif
            break;
            case OP_glPolygonOffsetx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPolygonOffsetx(0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glPolygonOffsetx(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPolygonOffsetx");
#endif
            break;
            case OP_glPopMatrix:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPopMatrix()\n", stream);
#endif
            s_gl.glPopMatrix();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPopMatrix");
#endif
            break;
            case OP_glPushMatrix:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPushMatrix()\n", stream);
#endif
            s_gl.glPushMatrix();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPushMatrix");
#endif
            break;
            case OP_glReadPixels:
            {
            size_t tmpPtr6Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr6Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr6 = &tmpBuf[0];
            memset(tmpPtr6, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glReadPixels(%d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(tmpPtr6), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glReadPixels(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(tmpPtr6));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glReadPixels");
#endif
            break;
            case OP_glRotatex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glRotatex(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glRotatex(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glRotatex");
#endif
            break;
            case OP_glSampleCoverage:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glSampleCoverage(%f %d )\n", stream,*(GLclampf *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
#endif
            s_gl.glSampleCoverage(*(GLclampf *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSampleCoverage");
#endif
            break;
            case OP_glSampleCoveragex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glSampleCoveragex(0x%08x %d )\n", stream,*(GLclampx *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
#endif
            s_gl.glSampleCoveragex(*(GLclampx *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSampleCoveragex");
#endif
            break;
            case OP_glScalex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glScalex(0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glScalex(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glScalex");
#endif
            break;
            case OP_glScissor:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glScissor(%d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glScissor(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glScissor");
#endif
            break;
            case OP_glShadeModel:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glShadeModel(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glShadeModel(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glShadeModel");
#endif
            break;
            case OP_glStencilFunc:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glStencilFunc(0x%08x %d %u )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glStencilFunc(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilFunc");
#endif
            break;
            case OP_glStencilMask:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glStencilMask(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl.glStencilMask(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilMask");
#endif
            break;
            case OP_glStencilOp:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glStencilOp(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glStencilOp(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilOp");
#endif
            break;
            case OP_glTexCoordPointer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexCoordPointer(%d 0x%08x %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glTexCoordPointer(*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexCoordPointer");
#endif
            break;
            case OP_glTexEnvi:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvi(0x%08x 0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvi(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvi");
#endif
            break;
            case OP_glTexEnvx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvx(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvx(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvx");
#endif
            break;
            case OP_glTexEnviv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnviv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnviv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnviv");
#endif
            break;
            case OP_glTexEnvxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvxv");
#endif
            break;
            case OP_glTexImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexImage2D(0x%08x %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glTexImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *((unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4)) == 0 ? NULL : (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexImage2D");
#endif
            break;
            case OP_glTexParameteri:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameteri(0x%08x 0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameteri(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameteri");
#endif
            break;
            case OP_glTexParameterx:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameterx(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameterx(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterx");
#endif
            break;
            case OP_glTexParameteriv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameteriv");
#endif
            break;
            case OP_glTexParameterxv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameterxv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameterxv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterxv");
#endif
            break;
            case OP_glTexSubImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexSubImage2D(0x%08x %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glTexSubImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexSubImage2D");
#endif
            break;
            case OP_glTranslatex:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTranslatex(0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTranslatex(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTranslatex");
#endif
            break;
            case OP_glVertexPointer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glVertexPointer(%d 0x%08x %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glVertexPointer(*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexPointer");
#endif
            break;
            case OP_glViewport:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glViewport(%d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glViewport(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glViewport");
#endif
            break;
            case OP_glPointSizePointerOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointSizePointerOES(0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glPointSizePointerOES(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointSizePointerOES");
#endif
            break;
            case OP_glVertexPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glVertexPointerOffset(%d 0x%08x %d %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            this->s_glVertexPointerOffset(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexPointerOffset");
#endif
            break;
            case OP_glColorPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColorPointerOffset(%d 0x%08x %d %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            this->s_glColorPointerOffset(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColorPointerOffset");
#endif
            break;
            case OP_glNormalPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glNormalPointerOffset(0x%08x %d %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
#endif
            this->s_glNormalPointerOffset(this, *(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glNormalPointerOffset");
#endif
            break;
            case OP_glPointSizePointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointSizePointerOffset(0x%08x %d %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
#endif
            this->s_glPointSizePointerOffset(this, *(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointSizePointerOffset");
#endif
            break;
            case OP_glTexCoordPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexCoordPointerOffset(%d 0x%08x %d %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            this->s_glTexCoordPointerOffset(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexCoordPointerOffset");
#endif
            break;
            case OP_glWeightPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glWeightPointerOffset(%d 0x%08x %d %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            this->s_glWeightPointerOffset(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glWeightPointerOffset");
#endif
            break;
            case OP_glMatrixIndexPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMatrixIndexPointerOffset(%d 0x%08x %d %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            this->s_glMatrixIndexPointerOffset(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMatrixIndexPointerOffset");
#endif
            break;
            case OP_glVertexPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glVertexPointerData(%d 0x%08x %d %p(%u) %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            this->s_glVertexPointerData(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexPointerData");
#endif
            break;
            case OP_glColorPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColorPointerData(%d 0x%08x %d %p(%u) %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            this->s_glColorPointerData(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColorPointerData");
#endif
            break;
            case OP_glNormalPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glNormalPointerData(0x%08x %d %p(%u) %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (void*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            this->s_glNormalPointerData(this, *(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (void*)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glNormalPointerData");
#endif
            break;
            case OP_glTexCoordPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexCoordPointerData(%d %d 0x%08x %d %p(%u) %u )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + 4)));
#endif
            this->s_glTexCoordPointerData(this, *(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexCoordPointerData");
#endif
            break;
            case OP_glPointSizePointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointSizePointerData(0x%08x %d %p(%u) %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (void*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            this->s_glPointSizePointerData(this, *(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (void*)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointSizePointerData");
#endif
            break;
            case OP_glWeightPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glWeightPointerData(%d 0x%08x %d %p(%u) %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            this->s_glWeightPointerData(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glWeightPointerData");
#endif
            break;
            case OP_glMatrixIndexPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMatrixIndexPointerData(%d 0x%08x %d %p(%u) %u )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            this->s_glMatrixIndexPointerData(this, *(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMatrixIndexPointerData");
#endif
            break;
            case OP_glDrawElementsOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawElementsOffset(0x%08x %d 0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            this->s_glDrawElementsOffset(this, *(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawElementsOffset");
#endif
            break;
            case OP_glDrawElementsData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawElementsData(0x%08x %d 0x%08x %p(%u) %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            this->s_glDrawElementsData(this, *(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawElementsData");
#endif
            break;
            case OP_glGetCompressedTextureFormats:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetCompressedTextureFormats(%d %p(%u) )\n", stream,*(int *)(ptr + 8), (GLint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            this->s_glGetCompressedTextureFormats(this, *(int *)(ptr + 8), (GLint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetCompressedTextureFormats");
#endif
            break;
            case OP_glFinishRoundTrip:
            {
            size_t totalTmpSize = sizeof(int);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFinishRoundTrip()\n", stream);
#endif
            *(int *)(&tmpBuf[0]) =          this->s_glFinishRoundTrip(this);
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFinishRoundTrip");
#endif
            break;
            case OP_glBlendEquationSeparateOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBlendEquationSeparateOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl.glBlendEquationSeparateOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendEquationSeparateOES");
#endif
            break;
            case OP_glBlendFuncSeparateOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBlendFuncSeparateOES(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glBlendFuncSeparateOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendFuncSeparateOES");
#endif
            break;
            case OP_glBlendEquationOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBlendEquationOES(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glBlendEquationOES(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendEquationOES");
#endif
            break;
            case OP_glDrawTexsOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexsOES(%d %d %d %d %d )\n", stream,*(GLshort *)(ptr + 8), *(GLshort *)(ptr + 8 + 2), *(GLshort *)(ptr + 8 + 2 + 2), *(GLshort *)(ptr + 8 + 2 + 2 + 2), *(GLshort *)(ptr + 8 + 2 + 2 + 2 + 2));
#endif
            s_gl.glDrawTexsOES(*(GLshort *)(ptr + 8), *(GLshort *)(ptr + 8 + 2), *(GLshort *)(ptr + 8 + 2 + 2), *(GLshort *)(ptr + 8 + 2 + 2 + 2), *(GLshort *)(ptr + 8 + 2 + 2 + 2 + 2));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexsOES");
#endif
            break;
            case OP_glDrawTexiOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexiOES(%d %d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glDrawTexiOES(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexiOES");
#endif
            break;
            case OP_glDrawTexxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexxOES(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glDrawTexxOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexxOES");
#endif
            break;
            case OP_glDrawTexsvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexsvOES(%p(%u) )\n", stream,(const GLshort*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glDrawTexsvOES((const GLshort*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexsvOES");
#endif
            break;
            case OP_glDrawTexivOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexivOES(%p(%u) )\n", stream,(const GLint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glDrawTexivOES((const GLint*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexivOES");
#endif
            break;
            case OP_glDrawTexxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexxvOES(%p(%u) )\n", stream,(const GLfixed*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glDrawTexxvOES((const GLfixed*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexxvOES");
#endif
            break;
            case OP_glDrawTexfOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexfOES(%f %f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glDrawTexfOES(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexfOES");
#endif
            break;
            case OP_glDrawTexfvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDrawTexfvOES(%p(%u) )\n", stream,(const GLfloat*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glDrawTexfvOES((const GLfloat*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDrawTexfvOES");
#endif
            break;
            case OP_glEGLImageTargetTexture2DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glEGLImageTargetTexture2DOES(0x%08x %p )\n", stream,*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
#endif
            s_gl.glEGLImageTargetTexture2DOES(*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEGLImageTargetTexture2DOES");
#endif
            break;
            case OP_glEGLImageTargetRenderbufferStorageOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glEGLImageTargetRenderbufferStorageOES(0x%08x %p )\n", stream,*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
#endif
            s_gl.glEGLImageTargetRenderbufferStorageOES(*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEGLImageTargetRenderbufferStorageOES");
#endif
            break;
            case OP_glAlphaFuncxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glAlphaFuncxOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
#endif
            s_gl.glAlphaFuncxOES(*(GLenum *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glAlphaFuncxOES");
#endif
            break;
            case OP_glClearColorxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearColorxOES(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4), *(GLclampx *)(ptr + 8 + 4 + 4), *(GLclampx *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glClearColorxOES(*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4), *(GLclampx *)(ptr + 8 + 4 + 4), *(GLclampx *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearColorxOES");
#endif
            break;
            case OP_glClearDepthxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearDepthxOES(0x%08x )\n", stream,*(GLclampx *)(ptr + 8));
#endif
            s_gl.glClearDepthxOES(*(GLclampx *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearDepthxOES");
#endif
            break;
            case OP_glClipPlanexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClipPlanexOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glClipPlanexOES(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClipPlanexOES");
#endif
            break;
            case OP_glClipPlanexIMG:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClipPlanexIMG(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glClipPlanexIMG(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClipPlanexIMG");
#endif
            break;
            case OP_glColor4xOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glColor4xOES(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glColor4xOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColor4xOES");
#endif
            break;
            case OP_glDepthRangexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDepthRangexOES(0x%08x 0x%08x )\n", stream,*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
#endif
            s_gl.glDepthRangexOES(*(GLclampx *)(ptr + 8), *(GLclampx *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthRangexOES");
#endif
            break;
            case OP_glFogxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFogxOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glFogxOES(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFogxOES");
#endif
            break;
            case OP_glFogxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFogxvOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glFogxvOES(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFogxvOES");
#endif
            break;
            case OP_glFrustumxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFrustumxOES(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glFrustumxOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFrustumxOES");
#endif
            break;
            case OP_glGetClipPlanexOES:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetClipPlanexOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetClipPlanexOES(*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetClipPlanexOES");
#endif
            break;
            case OP_glGetClipPlanex:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetClipPlanex(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetClipPlanex(*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetClipPlanex");
#endif
            break;
            case OP_glGetFixedvOES:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetFixedvOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetFixedvOES(*(GLenum *)(ptr + 8), (GLfixed*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFixedvOES");
#endif
            break;
            case OP_glGetLightxvOES:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetLightxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetLightxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetLightxvOES");
#endif
            break;
            case OP_glGetMaterialxvOES:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetMaterialxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetMaterialxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetMaterialxvOES");
#endif
            break;
            case OP_glGetTexEnvxvOES:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexEnvxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexEnvxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexEnvxvOES");
#endif
            break;
            case OP_glGetTexParameterxvOES:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexParameterxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexParameterxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexParameterxvOES");
#endif
            break;
            case OP_glLightModelxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightModelxOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glLightModelxOES(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightModelxOES");
#endif
            break;
            case OP_glLightModelxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightModelxvOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glLightModelxvOES(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightModelxvOES");
#endif
            break;
            case OP_glLightxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightxOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glLightxOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightxOES");
#endif
            break;
            case OP_glLightxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLightxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glLightxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLightxvOES");
#endif
            break;
            case OP_glLineWidthxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLineWidthxOES(0x%08x )\n", stream,*(GLfixed *)(ptr + 8));
#endif
            s_gl.glLineWidthxOES(*(GLfixed *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLineWidthxOES");
#endif
            break;
            case OP_glLoadMatrixxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLoadMatrixxOES(%p(%u) )\n", stream,(const GLfixed*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glLoadMatrixxOES((const GLfixed*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLoadMatrixxOES");
#endif
            break;
            case OP_glMaterialxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMaterialxOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glMaterialxOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMaterialxOES");
#endif
            break;
            case OP_glMaterialxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMaterialxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glMaterialxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMaterialxvOES");
#endif
            break;
            case OP_glMultMatrixxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultMatrixxOES(%p(%u) )\n", stream,(const GLfixed*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8));
#endif
            s_gl.glMultMatrixxOES((const GLfixed*)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultMatrixxOES");
#endif
            break;
            case OP_glMultiTexCoord4xOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiTexCoord4xOES(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glMultiTexCoord4xOES(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiTexCoord4xOES");
#endif
            break;
            case OP_glNormal3xOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glNormal3xOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glNormal3xOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glNormal3xOES");
#endif
            break;
            case OP_glOrthoxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glOrthoxOES(0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glOrthoxOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glOrthoxOES");
#endif
            break;
            case OP_glPointParameterxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointParameterxOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glPointParameterxOES(*(GLenum *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointParameterxOES");
#endif
            break;
            case OP_glPointParameterxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointParameterxvOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glPointParameterxvOES(*(GLenum *)(ptr + 8), (const GLfixed*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointParameterxvOES");
#endif
            break;
            case OP_glPointSizexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPointSizexOES(0x%08x )\n", stream,*(GLfixed *)(ptr + 8));
#endif
            s_gl.glPointSizexOES(*(GLfixed *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPointSizexOES");
#endif
            break;
            case OP_glPolygonOffsetxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glPolygonOffsetxOES(0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
#endif
            s_gl.glPolygonOffsetxOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPolygonOffsetxOES");
#endif
            break;
            case OP_glRotatexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glRotatexOES(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glRotatexOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4), *(GLfixed *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glRotatexOES");
#endif
            break;
            case OP_glSampleCoveragexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glSampleCoveragexOES(0x%08x %d )\n", stream,*(GLclampx *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
#endif
            s_gl.glSampleCoveragexOES(*(GLclampx *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSampleCoveragexOES");
#endif
            break;
            case OP_glScalexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glScalexOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glScalexOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glScalexOES");
#endif
            break;
            case OP_glTexEnvxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvxOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvxOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvxOES");
#endif
            break;
            case OP_glTexEnvxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexEnvxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexEnvxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexEnvxvOES");
#endif
            break;
            case OP_glTexParameterxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameterxOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameterxOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterxOES");
#endif
            break;
            case OP_glTexParameterxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexParameterxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexParameterxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterxvOES");
#endif
            break;
            case OP_glTranslatexOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTranslatexOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTranslatexOES(*(GLfixed *)(ptr + 8), *(GLfixed *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTranslatexOES");
#endif
            break;
            case OP_glIsRenderbufferOES:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsRenderbufferOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsRenderbufferOES(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsRenderbufferOES");
#endif
            break;
            case OP_glBindRenderbufferOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBindRenderbufferOES(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl.glBindRenderbufferOES(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindRenderbufferOES");
#endif
            break;
            case OP_glDeleteRenderbuffersOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDeleteRenderbuffersOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glDeleteRenderbuffersOES(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteRenderbuffersOES");
#endif
            break;
            case OP_glGenRenderbuffersOES:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenRenderbuffersOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGenRenderbuffersOES(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenRenderbuffersOES");
#endif
            break;
            case OP_glRenderbufferStorageOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glRenderbufferStorageOES(0x%08x 0x%08x %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glRenderbufferStorageOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glRenderbufferStorageOES");
#endif
            break;
            case OP_glGetRenderbufferParameterivOES:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetRenderbufferParameterivOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetRenderbufferParameterivOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetRenderbufferParameterivOES");
#endif
            break;
            case OP_glIsFramebufferOES:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsFramebufferOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsFramebufferOES(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsFramebufferOES");
#endif
            break;
            case OP_glBindFramebufferOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBindFramebufferOES(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl.glBindFramebufferOES(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindFramebufferOES");
#endif
            break;
            case OP_glDeleteFramebuffersOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDeleteFramebuffersOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glDeleteFramebuffersOES(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteFramebuffersOES");
#endif
            break;
            case OP_glGenFramebuffersOES:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenFramebuffersOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGenFramebuffersOES(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenFramebuffersOES");
#endif
            break;
            case OP_glCheckFramebufferStatusOES:
            {
            size_t totalTmpSize = sizeof(GLenum);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCheckFramebufferStatusOES(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLenum *)(&tmpBuf[0]) =           s_gl.glCheckFramebufferStatusOES(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCheckFramebufferStatusOES");
#endif
            break;
            case OP_glFramebufferRenderbufferOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFramebufferRenderbufferOES(0x%08x 0x%08x 0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glFramebufferRenderbufferOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFramebufferRenderbufferOES");
#endif
            break;
            case OP_glFramebufferTexture2DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFramebufferTexture2DOES(0x%08x 0x%08x 0x%08x %u %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glFramebufferTexture2DOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFramebufferTexture2DOES");
#endif
            break;
            case OP_glGetFramebufferAttachmentParameterivOES:
            {
            size_t tmpPtr3Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr3Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr3 = &tmpBuf[0];
            memset(tmpPtr3, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetFramebufferAttachmentParameterivOES(0x%08x 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (GLint*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glGetFramebufferAttachmentParameterivOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (GLint*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFramebufferAttachmentParameterivOES");
#endif
            break;
            case OP_glGenerateMipmapOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenerateMipmapOES(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl.glGenerateMipmapOES(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenerateMipmapOES");
#endif
            break;
            case OP_glMapBufferOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMapBufferOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl.glMapBufferOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMapBufferOES");
#endif
            break;
            case OP_glUnmapBufferOES:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glUnmapBufferOES(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glUnmapBufferOES(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUnmapBufferOES");
#endif
            break;
            case OP_glGetBufferPointervOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetBufferPointervOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLvoid**)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetBufferPointervOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLvoid**)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetBufferPointervOES");
#endif
            break;
            case OP_glCurrentPaletteMatrixOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glCurrentPaletteMatrixOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl.glCurrentPaletteMatrixOES(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCurrentPaletteMatrixOES");
#endif
            break;
            case OP_glLoadPaletteFromModelViewMatrixOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glLoadPaletteFromModelViewMatrixOES()\n", stream);
#endif
            s_gl.glLoadPaletteFromModelViewMatrixOES();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLoadPaletteFromModelViewMatrixOES");
#endif
            break;
            case OP_glMatrixIndexPointerOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMatrixIndexPointerOES(%d 0x%08x %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glMatrixIndexPointerOES(*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMatrixIndexPointerOES");
#endif
            break;
            case OP_glWeightPointerOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glWeightPointerOES(%d 0x%08x %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glWeightPointerOES(*(GLint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glWeightPointerOES");
#endif
            break;
            case OP_glQueryMatrixxOES:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            totalTmpSize += tmpPtr1Size;
            totalTmpSize += sizeof(GLbitfield);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr1 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glQueryMatrixxOES(%p(%u) %p(%u) )\n", stream,(GLfixed*)(tmpPtr0), *(unsigned int *)(ptr + 8), (GLint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            *(GLbitfield *)(&tmpBuf[0 + tmpPtr0Size + tmpPtr1Size]) =           s_gl.glQueryMatrixxOES((GLfixed*)(tmpPtr0), (GLint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glQueryMatrixxOES");
#endif
            break;
            case OP_glDepthRangefOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDepthRangefOES(%f %f )\n", stream,*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
#endif
            s_gl.glDepthRangefOES(*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthRangefOES");
#endif
            break;
            case OP_glFrustumfOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFrustumfOES(%f %f %f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glFrustumfOES(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFrustumfOES");
#endif
            break;
            case OP_glOrthofOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glOrthofOES(%f %f %f %f %f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glOrthofOES(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glOrthofOES");
#endif
            break;
            case OP_glClipPlanefOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClipPlanefOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glClipPlanefOES(*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClipPlanefOES");
#endif
            break;
            case OP_glClipPlanefIMG:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClipPlanefIMG(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glClipPlanefIMG(*(GLenum *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClipPlanefIMG");
#endif
            break;
            case OP_glGetClipPlanefOES:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetClipPlanefOES(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGetClipPlanefOES(*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetClipPlanefOES");
#endif
            break;
            case OP_glClearDepthfOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glClearDepthfOES(%f )\n", stream,*(GLclampf *)(ptr + 8));
#endif
            s_gl.glClearDepthfOES(*(GLclampf *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearDepthfOES");
#endif
            break;
            case OP_glTexGenfOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexGenfOES(0x%08x 0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexGenfOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexGenfOES");
#endif
            break;
            case OP_glTexGenfvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexGenfvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexGenfvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexGenfvOES");
#endif
            break;
            case OP_glTexGeniOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexGeniOES(0x%08x 0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexGeniOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexGeniOES");
#endif
            break;
            case OP_glTexGenivOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexGenivOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexGenivOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexGenivOES");
#endif
            break;
            case OP_glTexGenxOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexGenxOES(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexGenxOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfixed *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexGenxOES");
#endif
            break;
            case OP_glTexGenxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTexGenxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glTexGenxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexGenxvOES");
#endif
            break;
            case OP_glGetTexGenfvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexGenfvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexGenfvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexGenfvOES");
#endif
            break;
            case OP_glGetTexGenivOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexGenivOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexGenivOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexGenivOES");
#endif
            break;
            case OP_glGetTexGenxvOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetTexGenxvOES(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetTexGenxvOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfixed*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexGenxvOES");
#endif
            break;
            case OP_glBindVertexArrayOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glBindVertexArrayOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl.glBindVertexArrayOES(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindVertexArrayOES");
#endif
            break;
            case OP_glDeleteVertexArraysOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDeleteVertexArraysOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glDeleteVertexArraysOES(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteVertexArraysOES");
#endif
            break;
            case OP_glGenVertexArraysOES:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenVertexArraysOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGenVertexArraysOES(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenVertexArraysOES");
#endif
            break;
            case OP_glIsVertexArrayOES:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsVertexArrayOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsVertexArrayOES(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsVertexArrayOES");
#endif
            break;
            case OP_glDiscardFramebufferEXT:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDiscardFramebufferEXT(0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLenum*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glDiscardFramebufferEXT(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLenum*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDiscardFramebufferEXT");
#endif
            break;
            case OP_glMultiDrawArraysEXT:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiDrawArraysEXT(0x%08x %p(%u) %p(%u) %d )\n", stream,*(GLenum *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))));
#endif
            s_gl.glMultiDrawArraysEXT(*(GLenum *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiDrawArraysEXT");
#endif
            break;
            case OP_glMultiDrawElementsEXT:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiDrawElementsEXT(0x%08x %p(%u) 0x%08x %p(%u) %d )\n", stream,*(GLenum *)(ptr + 8), (const GLsizei*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid**)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
#endif
            s_gl.glMultiDrawElementsEXT(*(GLenum *)(ptr + 8), (const GLsizei*)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid**)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiDrawElementsEXT");
#endif
            break;
            case OP_glMultiDrawArraysSUN:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiDrawArraysSUN(0x%08x %p(%u) %p(%u) %d )\n", stream,*(GLenum *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))));
#endif
            //s_gl.glMultiDrawArraysSUN(*(GLenum *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiDrawArraysSUN");
#endif
            break;
            case OP_glMultiDrawElementsSUN:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glMultiDrawElementsSUN(0x%08x %p(%u) 0x%08x %p(%u) %d )\n", stream,*(GLenum *)(ptr + 8), (const GLsizei*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid**)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
#endif
            //s_gl.glMultiDrawElementsSUN(*(GLenum *)(ptr + 8), (const GLsizei*)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid**)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiDrawElementsSUN");
#endif
            break;
            case OP_glRenderbufferStorageMultisampleIMG:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glRenderbufferStorageMultisampleIMG(0x%08x %d 0x%08x %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glRenderbufferStorageMultisampleIMG(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glRenderbufferStorageMultisampleIMG");
#endif
            break;
            case OP_glFramebufferTexture2DMultisampleIMG:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFramebufferTexture2DMultisampleIMG(0x%08x 0x%08x 0x%08x %u %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glFramebufferTexture2DMultisampleIMG(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFramebufferTexture2DMultisampleIMG");
#endif
            break;
            case OP_glDeleteFencesNV:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDeleteFencesNV(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glDeleteFencesNV(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteFencesNV");
#endif
            break;
            case OP_glGenFencesNV:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGenFencesNV(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glGenFencesNV(*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenFencesNV");
#endif
            break;
            case OP_glIsFenceNV:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glIsFenceNV(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glIsFenceNV(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsFenceNV");
#endif
            break;
            case OP_glTestFenceNV:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glTestFenceNV(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glTestFenceNV(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTestFenceNV");
#endif
            break;
            case OP_glGetFenceivNV:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetFenceivNV(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetFenceivNV(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFenceivNV");
#endif
            break;
            case OP_glFinishFenceNV:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glFinishFenceNV(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl.glFinishFenceNV(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFinishFenceNV");
#endif
            break;
            case OP_glSetFenceNV:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glSetFenceNV(%u 0x%08x )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl.glSetFenceNV(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSetFenceNV");
#endif
            break;
            case OP_glGetDriverControlsQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetDriverControlsQCOM(%p(%u) %d %p(%u) )\n", stream,(GLint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLuint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glGetDriverControlsQCOM((GLint*)(tmpPtr0), *(GLsizei *)(ptr + 8 + 4), (GLuint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetDriverControlsQCOM");
#endif
            break;
            case OP_glGetDriverControlStringQCOM:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            size_t tmpPtr3Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4);
            totalTmpSize += tmpPtr3Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
            unsigned char *tmpPtr3 = &tmpBuf[0 + tmpPtr2Size];
            memset(tmpPtr3, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glGetDriverControlStringQCOM(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4), (GLchar*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl.glGetDriverControlStringQCOM(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), (GLchar*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetDriverControlStringQCOM");
#endif
            break;
            case OP_glEnableDriverControlQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glEnableDriverControlQCOM(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl.glEnableDriverControlQCOM(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEnableDriverControlQCOM");
#endif
            break;
            case OP_glDisableDriverControlQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glDisableDriverControlQCOM(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl.glDisableDriverControlQCOM(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDisableDriverControlQCOM");
#endif
            break;
            case OP_glExtGetTexturesQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetTexturesQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtGetTexturesQCOM((GLuint*)(tmpPtr0), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetTexturesQCOM");
#endif
            break;
            case OP_glExtGetBuffersQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetBuffersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtGetBuffersQCOM((GLuint*)(tmpPtr0), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetBuffersQCOM");
#endif
            break;
            case OP_glExtGetRenderbuffersQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetRenderbuffersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtGetRenderbuffersQCOM((GLuint*)(tmpPtr0), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetRenderbuffersQCOM");
#endif
            break;
            case OP_glExtGetFramebuffersQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetFramebuffersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtGetFramebuffersQCOM((GLuint*)(tmpPtr0), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetFramebuffersQCOM");
#endif
            break;
            case OP_glExtGetTexLevelParameterivQCOM:
            {
            size_t tmpPtr4Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr4Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr4 = &tmpBuf[0];
            memset(tmpPtr4, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetTexLevelParameterivQCOM(%u 0x%08x %d 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4), (GLint*)(tmpPtr4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glExtGetTexLevelParameterivQCOM(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4), (GLint*)(tmpPtr4));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetTexLevelParameterivQCOM");
#endif
            break;
            case OP_glExtTexObjectStateOverrideiQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtTexObjectStateOverrideiQCOM(0x%08x 0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtTexObjectStateOverrideiQCOM(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtTexObjectStateOverrideiQCOM");
#endif
            break;
            case OP_glExtGetTexSubImageQCOM:
            {
            size_t tmpPtr10Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr10Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr10 = &tmpBuf[0];
            memset(tmpPtr10, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetTexSubImageQCOM(0x%08x %d %d %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(tmpPtr10), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl.glExtGetTexSubImageQCOM(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(tmpPtr10));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetTexSubImageQCOM");
#endif
            break;
            case OP_glExtGetBufferPointervQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetBufferPointervQCOM(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLvoid**)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl.glExtGetBufferPointervQCOM(*(GLenum *)(ptr + 8), (GLvoid**)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetBufferPointervQCOM");
#endif
            break;
            case OP_glExtGetShadersQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetShadersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtGetShadersQCOM((GLuint*)(tmpPtr0), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetShadersQCOM");
#endif
            break;
            case OP_glExtGetProgramsQCOM:
            {
            size_t tmpPtr0Size = (size_t)*(unsigned int *)(ptr + 8);
            size_t totalTmpSize = tmpPtr0Size;
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            totalTmpSize += tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr0 = &tmpBuf[0];
            memset(tmpPtr0, 0, 4);
            unsigned char *tmpPtr2 = &tmpBuf[0 + tmpPtr0Size];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetProgramsQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(tmpPtr0), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl.glExtGetProgramsQCOM((GLuint*)(tmpPtr0), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetProgramsQCOM");
#endif
            break;
            case OP_glExtIsProgramBinaryQCOM:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtIsProgramBinaryQCOM(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl.glExtIsProgramBinaryQCOM(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtIsProgramBinaryQCOM");
#endif
            break;
            case OP_glExtGetProgramBinarySourceQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glExtGetProgramBinarySourceQCOM(%u 0x%08x %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl.glExtGetProgramBinarySourceQCOM(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetProgramBinarySourceQCOM");
#endif
            break;
            case OP_glStartTilingQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glStartTilingQCOM(%u %u %u %u 0x%08x )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLbitfield *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl.glStartTilingQCOM(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLbitfield *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStartTilingQCOM");
#endif
            break;
            case OP_glEndTilingQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl(%p): glEndTilingQCOM(0x%08x )\n", stream,*(GLbitfield *)(ptr + 8));
#endif
            s_gl.glEndTilingQCOM(*(GLbitfield *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEndTilingQCOM");
#endif
            break;
            default:
                unknownOpcode = true;
        } //switch
#ifdef CHECK_GL_ERROR
    int err = lastCall[0] ? s_gl.glGetError() : GL_NO_ERROR;
    if (err) fprintf(stderr, "gl Error: 0x%X in %s\n", err, lastCall);
#endif
    } // while
    return pos;
}

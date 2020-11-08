/*
* Copyright 2011 The Android Open Source Project
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

#include "GL2Decoder.h"
#include "egl.h"
#include "gl2.h"
#include "gl2ext.h"
#include "GL2Dispatch.h"
#include "gl2_opcodes.h"
#include <string.h>
#include <stdio.h>

#ifdef WITH_GLES2

//#undef DEBUG_PRINTOUT
//#undef CHECK_GL_ERROR

typedef unsigned int tsize_t;

static inline void* SafePointerFromUInt(GLuint value) {
  return (void*)(uintptr_t)value;
}

GL2Decoder::GL2Decoder()
{
    m_contextData = NULL;
}

GL2Decoder::~GL2Decoder()
{

}

int GL2Decoder::s_glFinishRoundTrip(void *self)
{
    GL2Decoder *ctx = (GL2Decoder *)self;
    s_gl2.glFinish();
    return 0;
}

void GL2Decoder::s_glGetCompressedTextureFormats(void *self, int count, GLint *formats)
{
    GL2Decoder *ctx = (GL2Decoder *) self;

    int nFormats;
    s_gl2.glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &nFormats);
    if (nFormats > count) {
        fprintf(stderr, "%s: GetCompressedTextureFormats: The requested number of formats does not match the number that is reported by OpenGL\n", __FUNCTION__);
    } else {
        s_gl2.glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
    }
}

void GL2Decoder::s_glVertexAttribPointerData(void *self, GLuint indx, GLint size, GLenum type,
                                             GLboolean normalized, GLsizei stride,  void * data, GLuint datalen)
{
    GL2Decoder *ctx = (GL2Decoder *) self;
    if (ctx->m_contextData != NULL) {
        ctx->m_contextData->storePointerData(indx, data, datalen);
        // note - the stride of the data is always zero when it comes out of the codec.
        // See gl2.attrib for the packing function call.
        s_gl2.glVertexAttribPointer(indx, size, type, normalized, 0, ctx->m_contextData->pointerData(indx));
    }
}

void GL2Decoder::s_glVertexAttribPointerOffset(void *self, GLuint indx, GLint size, GLenum type,
                                               GLboolean normalized, GLsizei stride,  GLuint data)
{
    GL2Decoder *ctx = (GL2Decoder *) self;
    s_gl2.glVertexAttribPointer(indx, size, type, normalized, stride, SafePointerFromUInt(data));
}


void GL2Decoder::s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen)
{
    GL2Decoder *ctx = (GL2Decoder *)self;
    s_gl2.glDrawElements(mode, count, type, data);
}


void GL2Decoder::s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset)
{
    GL2Decoder *ctx = (GL2Decoder *)self;
    s_gl2.glDrawElements(mode, count, type, SafePointerFromUInt(offset));
}

void GL2Decoder::s_glShaderString(void *self, GLuint shader, const GLchar* string, GLsizei len)
{
    GL2Decoder *ctx = (GL2Decoder *)self;
    s_gl2.glShaderSource(shader, 1, &string, NULL);
}

size_t GL2Decoder::decode(void *buf, size_t len, IOStream *stream)
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
            case OP_glActiveTexture:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glActiveTexture(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glActiveTexture(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glActiveTexture");
#endif
            break;
            case OP_glAttachShader:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glAttachShader(%u %u )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glAttachShader(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glAttachShader");
#endif
            break;
            case OP_glBindAttribLocation:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBindAttribLocation(%u %u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), (const GLchar*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glBindAttribLocation(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), (const GLchar*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindAttribLocation");
#endif
            break;
            case OP_glBindBuffer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBindBuffer(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glBindBuffer(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindBuffer");
#endif
            break;
            case OP_glBindFramebuffer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBindFramebuffer(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glBindFramebuffer(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindFramebuffer");
#endif
            break;
            case OP_glBindRenderbuffer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBindRenderbuffer(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glBindRenderbuffer(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindRenderbuffer");
#endif
            break;
            case OP_glBindTexture:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBindTexture(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glBindTexture(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBindTexture");
#endif
            break;
            case OP_glBlendColor:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBlendColor(%f %f %f %f )\n", stream,*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4), *(GLclampf *)(ptr + 8 + 4 + 4), *(GLclampf *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glBlendColor(*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4), *(GLclampf *)(ptr + 8 + 4 + 4), *(GLclampf *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendColor");
#endif
            break;
            case OP_glBlendEquation:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBlendEquation(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glBlendEquation(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendEquation");
#endif
            break;
            case OP_glBlendEquationSeparate:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBlendEquationSeparate(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl2.glBlendEquationSeparate(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendEquationSeparate");
#endif
            break;
            case OP_glBlendFunc:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBlendFunc(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl2.glBlendFunc(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendFunc");
#endif
            break;
            case OP_glBlendFuncSeparate:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBlendFuncSeparate(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glBlendFuncSeparate(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBlendFuncSeparate");
#endif
            break;
            case OP_glBufferData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBufferData(0x%08x %p %p(%u) 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLsizeiptr *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl2.glBufferData(*(GLenum *)(ptr + 8), *(GLsizeiptr *)(ptr + 8 + 4), *((unsigned int *)(ptr + 8 + 4 + 4)) == 0 ? NULL : (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBufferData");
#endif
            break;
            case OP_glBufferSubData:
            {
            static int nCount = 0;
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBufferSubData(0x%08x %p %p %p(%u) ) (nCount %d)\n", stream,*(GLenum *)(ptr + 8), *(GLintptr *)(ptr + 8 + 4), *(GLsizeiptr *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4),nCount++);
#endif
            s_gl2.glBufferSubData(*(GLenum *)(ptr + 8), *(GLintptr *)(ptr + 8 + 4), *(GLsizeiptr *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBufferSubData");
#endif
            break;
            case OP_glCheckFramebufferStatus:
            {
            size_t totalTmpSize = sizeof(GLenum);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCheckFramebufferStatus(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLenum *)(&tmpBuf[0]) =           s_gl2.glCheckFramebufferStatus(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCheckFramebufferStatus");
#endif
            break;
            case OP_glClear:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glClear(0x%08x )\n", stream,*(GLbitfield *)(ptr + 8));
#endif
            s_gl2.glClear(*(GLbitfield *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClear");
#endif
            break;
            case OP_glClearColor:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glClearColor(%f %f %f %f )\n", stream,*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4), *(GLclampf *)(ptr + 8 + 4 + 4), *(GLclampf *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glClearColor(*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4), *(GLclampf *)(ptr + 8 + 4 + 4), *(GLclampf *)(ptr + 8 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glClearDepthf(%f )\n", stream,*(GLclampf *)(ptr + 8));
#endif
            s_gl2.glClearDepthf(*(GLclampf *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearDepthf");
#endif
            break;
            case OP_glClearStencil:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glClearStencil(%d )\n", stream,*(GLint *)(ptr + 8));
#endif
            s_gl2.glClearStencil(*(GLint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glClearStencil");
#endif
            break;
            case OP_glColorMask:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glColorMask(%d %d %d %d )\n", stream,*(GLboolean *)(ptr + 8), *(GLboolean *)(ptr + 8 + 1), *(GLboolean *)(ptr + 8 + 1 + 1), *(GLboolean *)(ptr + 8 + 1 + 1 + 1));
#endif
            s_gl2.glColorMask(*(GLboolean *)(ptr + 8), *(GLboolean *)(ptr + 8 + 1), *(GLboolean *)(ptr + 8 + 1 + 1), *(GLboolean *)(ptr + 8 + 1 + 1 + 1));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glColorMask");
#endif
            break;
            case OP_glCompileShader:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCompileShader(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glCompileShader(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCompileShader");
#endif
            break;
            case OP_glCompressedTexImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCompressedTexImage2D(0x%08x %d 0x%08x %d %d %d %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCompressedTexImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *((unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4)) == 0 ? NULL : (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glCompressedTexSubImage2D(0x%08x %d %d %d %d %d 0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCompressedTexSubImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glCopyTexImage2D(0x%08x %d 0x%08x %d %d %d %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCopyTexImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glCopyTexSubImage2D(0x%08x %d %d %d %d %d %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCopyTexSubImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCopyTexSubImage2D");
#endif
            break;
            case OP_glCreateProgram:
            {
            size_t totalTmpSize = sizeof(GLuint);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCreateProgram()\n", stream);
#endif
            *(GLuint *)(&tmpBuf[0]) =           s_gl2.glCreateProgram();
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCreateProgram");
#endif
            break;
            case OP_glCreateShader:
            {
            size_t totalTmpSize = sizeof(GLuint);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCreateShader(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLuint *)(&tmpBuf[0]) =           s_gl2.glCreateShader(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCreateShader");
#endif
            break;
            case OP_glCullFace:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCullFace(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glCullFace(*(GLenum *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glDeleteBuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeleteBuffers(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteBuffers");
#endif
            break;
            case OP_glDeleteFramebuffers:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDeleteFramebuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeleteFramebuffers(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteFramebuffers");
#endif
            break;
            case OP_glDeleteProgram:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDeleteProgram(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glDeleteProgram(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteProgram");
#endif
            break;
            case OP_glDeleteRenderbuffers:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDeleteRenderbuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeleteRenderbuffers(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteRenderbuffers");
#endif
            break;
            case OP_glDeleteShader:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDeleteShader(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glDeleteShader(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeleteShader");
#endif
            break;
            case OP_glDeleteTextures:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDeleteTextures(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeleteTextures(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glDepthFunc(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glDepthFunc(*(GLenum *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glDepthMask(%d )\n", stream,*(GLboolean *)(ptr + 8));
#endif
            s_gl2.glDepthMask(*(GLboolean *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthMask");
#endif
            break;
            case OP_glDepthRangef:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDepthRangef(%f %f )\n", stream,*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
#endif
            s_gl2.glDepthRangef(*(GLclampf *)(ptr + 8), *(GLclampf *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDepthRangef");
#endif
            break;
            case OP_glDetachShader:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDetachShader(%u %u )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glDetachShader(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDetachShader");
#endif
            break;
            case OP_glDisable:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDisable(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glDisable(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDisable");
#endif
            break;
            case OP_glDisableVertexAttribArray:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDisableVertexAttribArray(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glDisableVertexAttribArray(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDisableVertexAttribArray");
#endif
            break;
            case OP_glDrawArrays:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDrawArrays(0x%08x %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glDrawArrays(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glDrawElements(0x%08x %d 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glDrawElements(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glEnable(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glEnable(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEnable");
#endif
            break;
            case OP_glEnableVertexAttribArray:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glEnableVertexAttribArray(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glEnableVertexAttribArray(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEnableVertexAttribArray");
#endif
            break;
            case OP_glFinish:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glFinish()\n", stream);
#endif
            s_gl2.glFinish();
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
            fprintf(stderr,"gl2(%p): glFlush()\n", stream);
#endif
            s_gl2.glFlush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFlush");
#endif
            break;
            case OP_glFramebufferRenderbuffer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glFramebufferRenderbuffer(0x%08x 0x%08x 0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glFramebufferRenderbuffer(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFramebufferRenderbuffer");
#endif
            break;
            case OP_glFramebufferTexture2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glFramebufferTexture2D(0x%08x 0x%08x 0x%08x %u %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glFramebufferTexture2D(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFramebufferTexture2D");
#endif
            break;
            case OP_glFrontFace:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glFrontFace(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glFrontFace(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFrontFace");
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
            fprintf(stderr,"gl2(%p): glGenBuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenBuffers(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenBuffers");
#endif
            break;
            case OP_glGenerateMipmap:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGenerateMipmap(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glGenerateMipmap(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenerateMipmap");
#endif
            break;
            case OP_glGenFramebuffers:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGenFramebuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenFramebuffers(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenFramebuffers");
#endif
            break;
            case OP_glGenRenderbuffers:
            {
            size_t tmpPtr1Size = (size_t)*(unsigned int *)(ptr + 8 + 4);
            size_t totalTmpSize = tmpPtr1Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr1 = &tmpBuf[0];
            memset(tmpPtr1, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGenRenderbuffers(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenRenderbuffers(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenRenderbuffers");
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
            fprintf(stderr,"gl2(%p): glGenTextures(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenTextures(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenTextures");
#endif
            break;
            case OP_glGetActiveAttrib:
            {
            size_t tmpPtr3Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr3Size;
            size_t tmpPtr4Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4);
            totalTmpSize += tmpPtr4Size;
            size_t tmpPtr5Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4);
            totalTmpSize += tmpPtr5Size;
            size_t tmpPtr6Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4);
            totalTmpSize += tmpPtr6Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr3 = &tmpBuf[0];
            memset(tmpPtr3, 0, 4);
            unsigned char *tmpPtr4 = &tmpBuf[0 + tmpPtr3Size];
            memset(tmpPtr4, 0, 4);
            unsigned char *tmpPtr5 = &tmpBuf[0 + tmpPtr3Size + tmpPtr4Size];
            memset(tmpPtr5, 0, 4);
            unsigned char *tmpPtr6 = &tmpBuf[0 + tmpPtr3Size + tmpPtr4Size + tmpPtr5Size];
            memset(tmpPtr6, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetActiveAttrib(%u %u %d %p(%u) %p(%u) %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (GLsizei*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), (GLint*)(tmpPtr4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4), (GLenum*)(tmpPtr5), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), (GLchar*)(tmpPtr6), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glGetActiveAttrib(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), tmpPtr3Size == 0 ? NULL : (GLsizei*)(tmpPtr3), (GLint*)(tmpPtr4), (GLenum*)(tmpPtr5), tmpPtr6Size == 0 ? NULL : (GLchar*)(tmpPtr6));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetActiveAttrib");
#endif
            break;
            case OP_glGetActiveUniform:
            {
            size_t tmpPtr3Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr3Size;
            size_t tmpPtr4Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4);
            totalTmpSize += tmpPtr4Size;
            size_t tmpPtr5Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4);
            totalTmpSize += tmpPtr5Size;
            size_t tmpPtr6Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4);
            totalTmpSize += tmpPtr6Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr3 = &tmpBuf[0];
            memset(tmpPtr3, 0, 4);
            unsigned char *tmpPtr4 = &tmpBuf[0 + tmpPtr3Size];
            memset(tmpPtr4, 0, 4);
            unsigned char *tmpPtr5 = &tmpBuf[0 + tmpPtr3Size + tmpPtr4Size];
            memset(tmpPtr5, 0, 4);
            unsigned char *tmpPtr6 = &tmpBuf[0 + tmpPtr3Size + tmpPtr4Size + tmpPtr5Size];
            memset(tmpPtr6, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetActiveUniform(%u %u %d %p(%u) %p(%u) %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (GLsizei*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), (GLint*)(tmpPtr4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4), (GLenum*)(tmpPtr5), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), (GLchar*)(tmpPtr6), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glGetActiveUniform(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), tmpPtr3Size == 0 ? NULL : (GLsizei*)(tmpPtr3), (GLint*)(tmpPtr4), (GLenum*)(tmpPtr5), tmpPtr6Size == 0 ? NULL : (GLchar*)(tmpPtr6));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetActiveUniform");
#endif
            break;
            case OP_glGetAttachedShaders:
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
            fprintf(stderr,"gl2(%p): glGetAttachedShaders(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4), (GLuint*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetAttachedShaders(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), tmpPtr2Size == 0 ? NULL : (GLsizei*)(tmpPtr2), (GLuint*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetAttachedShaders");
#endif
            break;
            case OP_glGetAttribLocation:
            {
            size_t totalTmpSize = sizeof(int);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetAttribLocation(%u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (const GLchar*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            *(int *)(&tmpBuf[0]) =          s_gl2.glGetAttribLocation(*(GLuint *)(ptr + 8), (const GLchar*)(ptr + 8 + 4 + 4));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetAttribLocation");
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
            fprintf(stderr,"gl2(%p): glGetBooleanv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLboolean*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGetBooleanv(*(GLenum *)(ptr + 8), (GLboolean*)(tmpPtr1));
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
            fprintf(stderr,"gl2(%p): glGetBufferParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetBufferParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetBufferParameteriv");
#endif
            break;
            case OP_glGetError:
            {
            size_t totalTmpSize = sizeof(GLenum);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetError()\n", stream);
#endif
            *(GLenum *)(&tmpBuf[0]) =           s_gl2.glGetError();
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetError");
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
            fprintf(stderr,"gl2(%p): glGetFloatv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGetFloatv(*(GLenum *)(ptr + 8), (GLfloat*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFloatv");
#endif
            break;
            case OP_glGetFramebufferAttachmentParameteriv:
            {
            size_t tmpPtr3Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4 + 4);
            size_t totalTmpSize = tmpPtr3Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr3 = &tmpBuf[0];
            memset(tmpPtr3, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetFramebufferAttachmentParameteriv(0x%08x 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (GLint*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetFramebufferAttachmentParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (GLint*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetFramebufferAttachmentParameteriv");
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
            fprintf(stderr,"gl2(%p): glGetIntegerv(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGetIntegerv(*(GLenum *)(ptr + 8), (GLint*)(tmpPtr1));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetIntegerv");
#endif
            break;
            case OP_glGetProgramiv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetProgramiv(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetProgramiv(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetProgramiv");
#endif
            break;
            case OP_glGetProgramInfoLog:
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
            fprintf(stderr,"gl2(%p): glGetProgramInfoLog(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4), (GLchar*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetProgramInfoLog(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), (GLchar*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetProgramInfoLog");
#endif
            break;
            case OP_glGetRenderbufferParameteriv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetRenderbufferParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetRenderbufferParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetRenderbufferParameteriv");
#endif
            break;
            case OP_glGetShaderiv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetShaderiv(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetShaderiv(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetShaderiv");
#endif
            break;
            case OP_glGetShaderInfoLog:
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
            fprintf(stderr,"gl2(%p): glGetShaderInfoLog(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4), (GLchar*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetShaderInfoLog(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), tmpPtr2Size == 0 ? NULL : (GLsizei*)(tmpPtr2), (GLchar*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetShaderInfoLog");
#endif
            break;
            case OP_glGetShaderPrecisionFormat:
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
            fprintf(stderr,"gl2(%p): glGetShaderPrecisionFormat(0x%08x 0x%08x %p(%u) %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4), (GLint*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetShaderPrecisionFormat(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), (GLint*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetShaderPrecisionFormat");
#endif
            break;
            case OP_glGetShaderSource:
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
            fprintf(stderr,"gl2(%p): glGetShaderSource(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4), (GLchar*)(tmpPtr3), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetShaderSource(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), tmpPtr2Size == 0 ? NULL : (GLsizei*)(tmpPtr2), (GLchar*)(tmpPtr3));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetShaderSource");
#endif
            break;
            case OP_glGetString:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetString(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glGetString(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetString");
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
            fprintf(stderr,"gl2(%p): glGetTexParameterfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetTexParameterfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexParameterfv");
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
            fprintf(stderr,"gl2(%p): glGetTexParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetTexParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetTexParameteriv");
#endif
            break;
            case OP_glGetUniformfv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetUniformfv(%u %d %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetUniformfv(*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetUniformfv");
#endif
            break;
            case OP_glGetUniformiv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetUniformiv(%u %d %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetUniformiv(*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetUniformiv");
#endif
            break;
            case OP_glGetUniformLocation:
            {
            size_t totalTmpSize = sizeof(int);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetUniformLocation(%u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (const GLchar*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            *(int *)(&tmpBuf[0]) =          s_gl2.glGetUniformLocation(*(GLuint *)(ptr + 8), (const GLchar*)(ptr + 8 + 4 + 4));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetUniformLocation");
#endif
            break;
            case OP_glGetVertexAttribfv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetVertexAttribfv(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetVertexAttribfv(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLfloat*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetVertexAttribfv");
#endif
            break;
            case OP_glGetVertexAttribiv:
            {
            size_t tmpPtr2Size = (size_t)*(unsigned int *)(ptr + 8 + 4 + 4);
            size_t totalTmpSize = tmpPtr2Size;
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
            unsigned char *tmpPtr2 = &tmpBuf[0];
            memset(tmpPtr2, 0, 4);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetVertexAttribiv(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetVertexAttribiv(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(tmpPtr2));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetVertexAttribiv");
#endif
            break;
            case OP_glGetVertexAttribPointerv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetVertexAttribPointerv(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLvoid**)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetVertexAttribPointerv(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLvoid**)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetVertexAttribPointerv");
#endif
            break;
            case OP_glHint:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glHint(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl2.glHint(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
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
            fprintf(stderr,"gl2(%p): glIsBuffer(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsBuffer(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glIsEnabled(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsEnabled(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsEnabled");
#endif
            break;
            case OP_glIsFramebuffer:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glIsFramebuffer(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsFramebuffer(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsFramebuffer");
#endif
            break;
            case OP_glIsProgram:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glIsProgram(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsProgram(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsProgram");
#endif
            break;
            case OP_glIsRenderbuffer:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glIsRenderbuffer(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsRenderbuffer(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsRenderbuffer");
#endif
            break;
            case OP_glIsShader:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glIsShader(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsShader(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsShader");
#endif
            break;
            case OP_glIsTexture:
            {
            size_t totalTmpSize = sizeof(GLboolean);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glIsTexture(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsTexture(*(GLuint *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glIsTexture");
#endif
            break;
            case OP_glLineWidth:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glLineWidth(%f )\n", stream,*(GLfloat *)(ptr + 8));
#endif
            s_gl2.glLineWidth(*(GLfloat *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLineWidth");
#endif
            break;
            case OP_glLinkProgram:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glLinkProgram(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glLinkProgram(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glLinkProgram");
#endif
            break;
            case OP_glPixelStorei:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glPixelStorei(0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4));
#endif
            s_gl2.glPixelStorei(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPixelStorei");
#endif
            break;
            case OP_glPolygonOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glPolygonOffset(%f %f )\n", stream,*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl2.glPolygonOffset(*(GLfloat *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glPolygonOffset");
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
            fprintf(stderr,"gl2(%p): glReadPixels(%d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(tmpPtr6), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glReadPixels(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(tmpPtr6));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glReadPixels");
#endif
            break;
            case OP_glReleaseShaderCompiler:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glReleaseShaderCompiler()\n", stream);
#endif
            s_gl2.glReleaseShaderCompiler();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glReleaseShaderCompiler");
#endif
            break;
            case OP_glRenderbufferStorage:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glRenderbufferStorage(0x%08x 0x%08x %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glRenderbufferStorage(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glRenderbufferStorage");
#endif
            break;
            case OP_glSampleCoverage:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glSampleCoverage(%f %d )\n", stream,*(GLclampf *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
#endif
            s_gl2.glSampleCoverage(*(GLclampf *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSampleCoverage");
#endif
            break;
            case OP_glScissor:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glScissor(%d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glScissor(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glScissor");
#endif
            break;
            case OP_glShaderBinary:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glShaderBinary(%d %p(%u) 0x%08x %p(%u) %d )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
#endif
            s_gl2.glShaderBinary(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glShaderBinary");
#endif
            break;
            case OP_glShaderSource:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glShaderSource(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLchar**)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl2.glShaderSource(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLchar**)(ptr + 8 + 4 + 4 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glShaderSource");
#endif
            break;
            case OP_glStencilFunc:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glStencilFunc(0x%08x %d %u )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glStencilFunc(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilFunc");
#endif
            break;
            case OP_glStencilFuncSeparate:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glStencilFuncSeparate(0x%08x 0x%08x %d %u )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glStencilFuncSeparate(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilFuncSeparate");
#endif
            break;
            case OP_glStencilMask:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glStencilMask(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glStencilMask(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilMask");
#endif
            break;
            case OP_glStencilMaskSeparate:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glStencilMaskSeparate(0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
#endif
            s_gl2.glStencilMaskSeparate(*(GLenum *)(ptr + 8), *(GLuint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilMaskSeparate");
#endif
            break;
            case OP_glStencilOp:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glStencilOp(0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glStencilOp(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilOp");
#endif
            break;
            case OP_glStencilOpSeparate:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glStencilOpSeparate(0x%08x 0x%08x 0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glStencilOpSeparate(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glStencilOpSeparate");
#endif
            break;
            case OP_glTexImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexImage2D(0x%08x %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glTexImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *((unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4)) == 0 ? NULL : (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexImage2D");
#endif
            break;
            case OP_glTexParameterf:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexParameterf(0x%08x 0x%08x %f )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glTexParameterf(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glTexParameterfv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glTexParameterfv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameterfv");
#endif
            break;
            case OP_glTexParameteri:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexParameteri(0x%08x 0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glTexParameteri(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameteri");
#endif
            break;
            case OP_glTexParameteriv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexParameteriv(0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glTexParameteriv(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexParameteriv");
#endif
            break;
            case OP_glTexSubImage2D:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexSubImage2D(0x%08x %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glTexSubImage2D(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexSubImage2D");
#endif
            break;
            case OP_glUniform1f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform1f(%d %f )\n", stream,*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl2.glUniform1f(*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform1f");
#endif
            break;
            case OP_glUniform1fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform1fv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform1fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform1fv");
#endif
            break;
            case OP_glUniform1i:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform1i(%d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4));
#endif
            s_gl2.glUniform1i(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform1i");
#endif
            break;
            case OP_glUniform1iv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform1iv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform1iv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform1iv");
#endif
            break;
            case OP_glUniform2f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform2f(%d %f %f )\n", stream,*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform2f(*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform2f");
#endif
            break;
            case OP_glUniform2fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform2fv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform2fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform2fv");
#endif
            break;
            case OP_glUniform2i:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform2i(%d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform2i(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform2i");
#endif
            break;
            case OP_glUniform2iv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform2iv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform2iv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform2iv");
#endif
            break;
            case OP_glUniform3f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform3f(%d %f %f %f )\n", stream,*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glUniform3f(*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform3f");
#endif
            break;
            case OP_glUniform3fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform3fv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform3fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform3fv");
#endif
            break;
            case OP_glUniform3i:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform3i(%d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glUniform3i(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform3i");
#endif
            break;
            case OP_glUniform3iv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform3iv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform3iv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform3iv");
#endif
            break;
            case OP_glUniform4f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform4f(%d %f %f %f %f )\n", stream,*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glUniform4f(*(GLint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform4f");
#endif
            break;
            case OP_glUniform4fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform4fv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform4fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform4fv");
#endif
            break;
            case OP_glUniform4i:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform4i(%d %d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glUniform4i(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform4i");
#endif
            break;
            case OP_glUniform4iv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniform4iv(%d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glUniform4iv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLint*)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniform4iv");
#endif
            break;
            case OP_glUniformMatrix2fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniformMatrix2fv(%d %d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLboolean *)(ptr + 8 + 4 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 1 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 1));
#endif
            s_gl2.glUniformMatrix2fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLboolean *)(ptr + 8 + 4 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 1 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniformMatrix2fv");
#endif
            break;
            case OP_glUniformMatrix3fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniformMatrix3fv(%d %d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLboolean *)(ptr + 8 + 4 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 1 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 1));
#endif
            s_gl2.glUniformMatrix3fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLboolean *)(ptr + 8 + 4 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 1 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniformMatrix3fv");
#endif
            break;
            case OP_glUniformMatrix4fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUniformMatrix4fv(%d %d %d %p(%u) )\n", stream,*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLboolean *)(ptr + 8 + 4 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 1 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 1));
#endif
            s_gl2.glUniformMatrix4fv(*(GLint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLboolean *)(ptr + 8 + 4 + 4), (const GLfloat*)(ptr + 8 + 4 + 4 + 1 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUniformMatrix4fv");
#endif
            break;
            case OP_glUseProgram:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glUseProgram(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glUseProgram(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUseProgram");
#endif
            break;
            case OP_glValidateProgram:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glValidateProgram(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glValidateProgram(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glValidateProgram");
#endif
            break;
            case OP_glVertexAttrib1f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib1f(%u %f )\n", stream,*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
#endif
            s_gl2.glVertexAttrib1f(*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib1f");
#endif
            break;
            case OP_glVertexAttrib1fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib1fv(%u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glVertexAttrib1fv(*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib1fv");
#endif
            break;
            case OP_glVertexAttrib2f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib2f(%u %f %f )\n", stream,*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glVertexAttrib2f(*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib2f");
#endif
            break;
            case OP_glVertexAttrib2fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib2fv(%u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glVertexAttrib2fv(*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib2fv");
#endif
            break;
            case OP_glVertexAttrib3f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib3f(%u %f %f %f )\n", stream,*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glVertexAttrib3f(*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib3f");
#endif
            break;
            case OP_glVertexAttrib3fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib3fv(%u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glVertexAttrib3fv(*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib3fv");
#endif
            break;
            case OP_glVertexAttrib4f:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib4f(%u %f %f %f %f )\n", stream,*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glVertexAttrib4f(*(GLuint *)(ptr + 8), *(GLfloat *)(ptr + 8 + 4), *(GLfloat *)(ptr + 8 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4), *(GLfloat *)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib4f");
#endif
            break;
            case OP_glVertexAttrib4fv:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttrib4fv(%u %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glVertexAttrib4fv(*(GLuint *)(ptr + 8), (const GLfloat*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttrib4fv");
#endif
            break;
            case OP_glVertexAttribPointer:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttribPointer(%u %d 0x%08x %d %d %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLboolean *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 1), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 1 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 1 + 4));
#endif
            s_gl2.glVertexAttribPointer(*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLboolean *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 1), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 1 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttribPointer");
#endif
            break;
            case OP_glViewport:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glViewport(%d %d %d %d )\n", stream,*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glViewport(*(GLint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glViewport");
#endif
            break;
            case OP_glEGLImageTargetTexture2DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glEGLImageTargetTexture2DOES(0x%08x %p )\n", stream,*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
#endif
            s_gl2.glEGLImageTargetTexture2DOES(*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
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
            fprintf(stderr,"gl2(%p): glEGLImageTargetRenderbufferStorageOES(0x%08x %p )\n", stream,*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
#endif
            s_gl2.glEGLImageTargetRenderbufferStorageOES(*(GLenum *)(ptr + 8), *(GLeglImageOES *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEGLImageTargetRenderbufferStorageOES");
#endif
            break;
            case OP_glGetProgramBinaryOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetProgramBinaryOES(%u %d %p(%u) %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), (GLenum*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)), (GLvoid*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4))));
#endif
            s_gl2.glGetProgramBinaryOES(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4), (GLenum*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), (GLvoid*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)) + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetProgramBinaryOES");
#endif
            break;
            case OP_glProgramBinaryOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glProgramBinaryOES(%u 0x%08x %p(%u) %d )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl2.glProgramBinaryOES(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glProgramBinaryOES");
#endif
            break;
            case OP_glMapBufferOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glMapBufferOES(0x%08x 0x%08x )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl2.glMapBufferOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
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
            fprintf(stderr,"gl2(%p): glUnmapBufferOES(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glUnmapBufferOES(*(GLenum *)(ptr + 8));
            stream->flush();
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glUnmapBufferOES");
#endif
            break;
            case OP_glTexImage3DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexImage3DOES(0x%08x %d 0x%08x %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glTexImage3DOES(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *((unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4)) == 0 ? NULL : (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexImage3DOES");
#endif
            break;
            case OP_glTexSubImage3DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glTexSubImage3DOES(0x%08x %d %d %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glTexSubImage3DOES(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glTexSubImage3DOES");
#endif
            break;
            case OP_glCopyTexSubImage3DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCopyTexSubImage3DOES(0x%08x %d %d %d %d %d %d %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCopyTexSubImage3DOES(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCopyTexSubImage3DOES");
#endif
            break;
            case OP_glCompressedTexImage3DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCompressedTexImage3DOES(0x%08x %d 0x%08x %d %d %d %d %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCompressedTexImage3DOES(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCompressedTexImage3DOES");
#endif
            break;
            case OP_glCompressedTexSubImage3DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCompressedTexSubImage3DOES(0x%08x %d %d %d %d %d %d %d 0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glCompressedTexSubImage3DOES(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (const GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCompressedTexSubImage3DOES");
#endif
            break;
            case OP_glFramebufferTexture3DOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glFramebufferTexture3DOES(0x%08x 0x%08x 0x%08x %u %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glFramebufferTexture3DOES(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glFramebufferTexture3DOES");
#endif
            break;
            case OP_glBindVertexArrayOES:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBindVertexArrayOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glBindVertexArrayOES(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glDeleteVertexArraysOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeleteVertexArraysOES(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glGenVertexArraysOES(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenVertexArraysOES(*(GLsizei *)(ptr + 8), (GLuint*)(tmpPtr1));
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
            fprintf(stderr,"gl2(%p): glIsVertexArrayOES(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsVertexArrayOES(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glDiscardFramebufferEXT(0x%08x %d %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLenum*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glDiscardFramebufferEXT(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (const GLenum*)(ptr + 8 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glMultiDrawArraysEXT(0x%08x %p(%u) %p(%u) %d )\n", stream,*(GLenum *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))));
#endif
            s_gl2.glMultiDrawArraysEXT(*(GLenum *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))));
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
            fprintf(stderr,"gl2(%p): glMultiDrawElementsEXT(0x%08x %p(%u) 0x%08x %p(%u) %d )\n", stream,*(GLenum *)(ptr + 8), (const GLsizei*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid**)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
#endif
            s_gl2.glMultiDrawElementsEXT(*(GLenum *)(ptr + 8), (const GLsizei*)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), (const GLvoid**)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glMultiDrawElementsEXT");
#endif
            break;
            case OP_glGetPerfMonitorGroupsAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetPerfMonitorGroupsAMD(%p(%u) %d %p(%u) )\n", stream,(GLint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLuint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glGetPerfMonitorGroupsAMD((GLint*)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLuint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPerfMonitorGroupsAMD");
#endif
            break;
            case OP_glGetPerfMonitorCountersAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetPerfMonitorCountersAMD(%u %p(%u) %p(%u) %d %p(%u) )\n", stream,*(GLuint *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), (GLint*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))), (GLuint*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)) + 4));
#endif
            s_gl2.glGetPerfMonitorCountersAMD(*(GLuint *)(ptr + 8), (GLint*)(ptr + 8 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4))), (GLuint*)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4) + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPerfMonitorCountersAMD");
#endif
            break;
            case OP_glGetPerfMonitorGroupStringAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetPerfMonitorGroupStringAMD(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl2.glGetPerfMonitorGroupStringAMD(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPerfMonitorGroupStringAMD");
#endif
            break;
            case OP_glGetPerfMonitorCounterStringAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetPerfMonitorCounterStringAMD(%u %u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            s_gl2.glGetPerfMonitorCounterStringAMD(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4) + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPerfMonitorCounterStringAMD");
#endif
            break;
            case OP_glGetPerfMonitorCounterInfoAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetPerfMonitorCounterInfoAMD(%u %u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4));
#endif
            s_gl2.glGetPerfMonitorCounterInfoAMD(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPerfMonitorCounterInfoAMD");
#endif
            break;
            case OP_glGenPerfMonitorsAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGenPerfMonitorsAMD(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenPerfMonitorsAMD(*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGenPerfMonitorsAMD");
#endif
            break;
            case OP_glDeletePerfMonitorsAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDeletePerfMonitorsAMD(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeletePerfMonitorsAMD(*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDeletePerfMonitorsAMD");
#endif
            break;
            case OP_glSelectPerfMonitorCountersAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glSelectPerfMonitorCountersAMD(%u %d %u %d %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 1), *(GLint *)(ptr + 8 + 4 + 1 + 4), (GLuint*)(ptr + 8 + 4 + 1 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 1 + 4 + 4));
#endif
            s_gl2.glSelectPerfMonitorCountersAMD(*(GLuint *)(ptr + 8), *(GLboolean *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 1), *(GLint *)(ptr + 8 + 4 + 1 + 4), (GLuint*)(ptr + 8 + 4 + 1 + 4 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSelectPerfMonitorCountersAMD");
#endif
            break;
            case OP_glBeginPerfMonitorAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glBeginPerfMonitorAMD(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glBeginPerfMonitorAMD(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glBeginPerfMonitorAMD");
#endif
            break;
            case OP_glEndPerfMonitorAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glEndPerfMonitorAMD(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glEndPerfMonitorAMD(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEndPerfMonitorAMD");
#endif
            break;
            case OP_glGetPerfMonitorCounterDataAMD:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetPerfMonitorCounterDataAMD(%u 0x%08x %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (GLuint*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
#endif
            s_gl2.glGetPerfMonitorCounterDataAMD(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4), (GLuint*)(ptr + 8 + 4 + 4 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4) + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetPerfMonitorCounterDataAMD");
#endif
            break;
            case OP_glRenderbufferStorageMultisampleIMG:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glRenderbufferStorageMultisampleIMG(0x%08x %d 0x%08x %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glRenderbufferStorageMultisampleIMG(*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glFramebufferTexture2DMultisampleIMG(0x%08x 0x%08x 0x%08x %u %d %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glFramebufferTexture2DMultisampleIMG(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glDeleteFencesNV(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glDeleteFencesNV(*(GLsizei *)(ptr + 8), (const GLuint*)(ptr + 8 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glGenFencesNV(%d %p(%u) )\n", stream,*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glGenFencesNV(*(GLsizei *)(ptr + 8), (GLuint*)(ptr + 8 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glIsFenceNV(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glIsFenceNV(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glTestFenceNV(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glTestFenceNV(*(GLuint *)(ptr + 8));
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
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetFenceivNV(%u 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glGetFenceivNV(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glFinishFenceNV(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glFinishFenceNV(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glSetFenceNV(%u 0x%08x )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
#endif
            s_gl2.glSetFenceNV(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glSetFenceNV");
#endif
            break;
            case OP_glCoverageMaskNV:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCoverageMaskNV(%d )\n", stream,*(GLboolean *)(ptr + 8));
#endif
            s_gl2.glCoverageMaskNV(*(GLboolean *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCoverageMaskNV");
#endif
            break;
            case OP_glCoverageOperationNV:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glCoverageOperationNV(0x%08x )\n", stream,*(GLenum *)(ptr + 8));
#endif
            s_gl2.glCoverageOperationNV(*(GLenum *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glCoverageOperationNV");
#endif
            break;
            case OP_glGetDriverControlsQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetDriverControlsQCOM(%p(%u) %d %p(%u) )\n", stream,(GLint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLuint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glGetDriverControlsQCOM((GLint*)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLuint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glGetDriverControlsQCOM");
#endif
            break;
            case OP_glGetDriverControlStringQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glGetDriverControlStringQCOM(%u %d %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl2.glGetDriverControlStringQCOM(*(GLuint *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), (GLsizei*)(ptr + 8 + 4 + 4 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4));
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
            fprintf(stderr,"gl2(%p): glEnableDriverControlQCOM(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glEnableDriverControlQCOM(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glDisableDriverControlQCOM(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            s_gl2.glDisableDriverControlQCOM(*(GLuint *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glDisableDriverControlQCOM");
#endif
            break;
            case OP_glExtGetTexturesQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetTexturesQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glExtGetTexturesQCOM((GLuint*)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetTexturesQCOM");
#endif
            break;
            case OP_glExtGetBuffersQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetBuffersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glExtGetBuffersQCOM((GLuint*)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetBuffersQCOM");
#endif
            break;
            case OP_glExtGetRenderbuffersQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetRenderbuffersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glExtGetRenderbuffersQCOM((GLuint*)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetRenderbuffersQCOM");
#endif
            break;
            case OP_glExtGetFramebuffersQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetFramebuffersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glExtGetFramebuffersQCOM((GLuint*)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetFramebuffersQCOM");
#endif
            break;
            case OP_glExtGetTexLevelParameterivQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetTexLevelParameterivQCOM(%u 0x%08x %d 0x%08x %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glExtGetTexLevelParameterivQCOM(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glExtTexObjectStateOverrideiQCOM(0x%08x 0x%08x %d )\n", stream,*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
#endif
            s_gl2.glExtTexObjectStateOverrideiQCOM(*(GLenum *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtTexObjectStateOverrideiQCOM");
#endif
            break;
            case OP_glExtGetTexSubImageQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetTexSubImageQCOM(0x%08x %d %d %d %d %d %d %d 0x%08x 0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glExtGetTexSubImageQCOM(*(GLenum *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4), *(GLint *)(ptr + 8 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), *(GLenum *)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4), (GLvoid*)(ptr + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glExtGetBufferPointervQCOM(0x%08x %p(%u) )\n", stream,*(GLenum *)(ptr + 8), (GLvoidptr*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4));
#endif
            s_gl2.glExtGetBufferPointervQCOM(*(GLenum *)(ptr + 8), (GLvoidptr*)(ptr + 8 + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetBufferPointervQCOM");
#endif
            break;
            case OP_glExtGetShadersQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetShadersQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glExtGetShadersQCOM((GLuint*)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glExtGetShadersQCOM");
#endif
            break;
            case OP_glExtGetProgramsQCOM:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glExtGetProgramsQCOM(%p(%u) %d %p(%u) )\n", stream,(GLuint*)(ptr + 8 + 4), *(unsigned int *)(ptr + 8), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4));
#endif
            s_gl2.glExtGetProgramsQCOM((GLuint*)(ptr + 8 + 4), *(GLint *)(ptr + 8 + 4 + *(tsize_t *)(ptr +8)), (GLint*)(ptr + 8 + 4 + *(tsize_t *)(ptr +8) + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glExtIsProgramBinaryQCOM(%u )\n", stream,*(GLuint *)(ptr + 8));
#endif
            *(GLboolean *)(&tmpBuf[0]) =            s_gl2.glExtIsProgramBinaryQCOM(*(GLuint *)(ptr + 8));
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
            fprintf(stderr,"gl2(%p): glExtGetProgramBinarySourceQCOM(%u 0x%08x %p(%u) %p(%u) )\n", stream,*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4)));
#endif
            s_gl2.glExtGetProgramBinarySourceQCOM(*(GLuint *)(ptr + 8), *(GLenum *)(ptr + 8 + 4), (GLchar*)(ptr + 8 + 4 + 4 + 4), (GLint*)(ptr + 8 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4) + 4));
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
            fprintf(stderr,"gl2(%p): glStartTilingQCOM(%u %u %u %u 0x%08x )\n", stream,*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLbitfield *)(ptr + 8 + 4 + 4 + 4 + 4));
#endif
            s_gl2.glStartTilingQCOM(*(GLuint *)(ptr + 8), *(GLuint *)(ptr + 8 + 4), *(GLuint *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4), *(GLbitfield *)(ptr + 8 + 4 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glEndTilingQCOM(0x%08x )\n", stream,*(GLbitfield *)(ptr + 8));
#endif
            s_gl2.glEndTilingQCOM(*(GLbitfield *)(ptr + 8));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glEndTilingQCOM");
#endif
            break;
            case OP_glVertexAttribPointerData:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttribPointerData(%u %d 0x%08x %d %d %p(%u) %u )\n", stream,*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLboolean *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 1), (void*)(ptr + 8 + 4 + 4 + 4 + 1 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4 + 1 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 1 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + 1 + 4)));
#endif
            this->s_glVertexAttribPointerData(this, *(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLboolean *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 1), (void*)(ptr + 8 + 4 + 4 + 4 + 1 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 1 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4 + 1 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttribPointerData");
#endif
            break;
            case OP_glVertexAttribPointerOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glVertexAttribPointerOffset(%u %d 0x%08x %d %d %u )\n", stream,*(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLboolean *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 1), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 1 + 4));
#endif
            this->s_glVertexAttribPointerOffset(this, *(GLuint *)(ptr + 8), *(GLint *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLboolean *)(ptr + 8 + 4 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + 4 + 1), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 1 + 4));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glVertexAttribPointerOffset");
#endif
            break;
            case OP_glDrawElementsOffset:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glDrawElementsOffset(0x%08x %d 0x%08x %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4));
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
            fprintf(stderr,"gl2(%p): glDrawElementsData(0x%08x %d 0x%08x %p(%u) %u )\n", stream,*(GLenum *)(ptr + 8), *(GLsizei *)(ptr + 8 + 4), *(GLenum *)(ptr + 8 + 4 + 4), (void*)(ptr + 8 + 4 + 4 + 4 + 4), *(unsigned int *)(ptr + 8 + 4 + 4 + 4), *(GLuint *)(ptr + 8 + 4 + 4 + 4 + 4 + *(tsize_t *)(ptr +8 + 4 + 4 + 4)));
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
            fprintf(stderr,"gl2(%p): glGetCompressedTextureFormats(%d %p(%u) )\n", stream,*(int *)(ptr + 8), (GLint*)(tmpPtr1), *(unsigned int *)(ptr + 8 + 4));
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
            case OP_glShaderString:
            {
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glShaderString(%u %p(%u) %d )\n", stream,*(GLuint *)(ptr + 8), (const GLchar*)(ptr + 8 + 4 + 4), *(unsigned int *)(ptr + 8 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)));
#endif
            this->s_glShaderString(this, *(GLuint *)(ptr + 8), (const GLchar*)(ptr + 8 + 4 + 4), *(GLsizei *)(ptr + 8 + 4 + 4 + *(tsize_t *)(ptr +8 + 4)));
            pos += *(int *)(ptr + 4);
            ptr += *(int *)(ptr + 4);
            }
#ifdef CHECK_GL_ERROR
            sprintf(lastCall, "glShaderString");
#endif
            break;
            case OP_glFinishRoundTrip:
            {
            size_t totalTmpSize = sizeof(int);
            unsigned char *tmpBuf = stream->alloc(totalTmpSize);
#ifdef DEBUG_PRINTOUT
            fprintf(stderr,"gl2(%p): glFinishRoundTrip()\n", stream);
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
            default:
                unknownOpcode = true;
        } //switch
#ifdef CHECK_GL_ERROR
    int err = lastCall[0] ? s_gl2.glGetError() : GL_NO_ERROR;
    if (err) fprintf(stderr, "gl2 Error: 0x%X in %s\n", err, lastCall);
#endif
    } // while
    return pos;
}

#endif

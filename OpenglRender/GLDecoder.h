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
#ifndef _GL_DECODER_H_
#define _GL_DECODER_H_

#include "IOStream.h"
#include "gl_base_types.h"
#include "FixedBuffer.h"
#include "GLDecoderContextData.h"

class GLDecoder
{
public:
    GLDecoder();
    ~GLDecoder();
    void setContextData(GLDecoderContextData *contextData) { m_contextData = contextData; }

    size_t decode(void *buf, size_t bufsize, IOStream *stream);

private:
    static void  s_glGetCompressedTextureFormats(void * self, GLint cont, GLint *data);
    static void  s_glVertexPointerData(void *self, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen);
    static void  s_glVertexPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset);

    static void  s_glColorPointerData(void *self, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen);
    static void  s_glColorPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset);

    static void  s_glTexCoordPointerData(void *self, GLint unit, GLint size, GLenum type, GLsizei stride, void *data, GLuint datalen);
    static void  s_glTexCoordPointerOffset(void *self, GLint size, GLenum type, GLsizei stride, GLuint offset);

    static void  s_glNormalPointerData(void *self, GLenum type, GLsizei stride, void *data, GLuint datalen);
    static void  s_glNormalPointerOffset(void *self, GLenum type, GLsizei stride, GLuint offset);

    static void  s_glPointSizePointerData(void *self, GLenum type, GLsizei stride, void *data, GLuint datalen);
    static void  s_glPointSizePointerOffset(void *self, GLenum type, GLsizei stride, GLuint offset);

    static void  s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset);
    static void  s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen);

    static void  s_glWeightPointerData(void * self, GLint size, GLenum type, GLsizei stride, void * data, GLuint datalen);
    static void  s_glWeightPointerOffset(void * self, GLint size, GLenum type, GLsizei stride, GLuint offset);

    static void  s_glMatrixIndexPointerData(void * self, GLint size, GLenum type, GLsizei stride, void * data, GLuint datalen);
    static void  s_glMatrixIndexPointerOffset(void * self, GLint size, GLenum type, GLsizei stride, GLuint offset);

    static int  s_glFinishRoundTrip(void *self);

    GLDecoderContextData *m_contextData;
};

#endif

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

#ifndef _GL2_DECODER_H_
#define _GL2_DECODER_H_
#ifdef WITH_GLES2
#include "IOStream.h"
#include "gl_base_types.h"
#include "GLDecoderContextData.h"

class GL2Decoder
{
public:
    GL2Decoder();
    ~GL2Decoder();
    void setContextData(GLDecoderContextData *contextData) { m_contextData = contextData; }
    size_t decode(void *buf, size_t bufsize, IOStream *stream);
private:
    GLDecoderContextData *m_contextData;

    static void  s_glGetCompressedTextureFormats(void *self, int count, GLint *formats);
    static void  s_glVertexAttribPointerData(void *self, GLuint indx, GLint size, GLenum type,
                                      GLboolean normalized, GLsizei stride,  void * data, GLuint datalen);
    static void  s_glVertexAttribPointerOffset(void *self, GLuint indx, GLint size, GLenum type,
                                        GLboolean normalized, GLsizei stride,  GLuint offset);

    static void  s_glDrawElementsOffset(void *self, GLenum mode, GLsizei count, GLenum type, GLuint offset);
    static void  s_glDrawElementsData(void *self, GLenum mode, GLsizei count, GLenum type, void * data, GLuint datalen);
    static void  s_glShaderString(void *self, GLuint shader, const GLchar* string, GLsizei len);
    static int   s_glFinishRoundTrip(void *self);
};
#endif
#endif

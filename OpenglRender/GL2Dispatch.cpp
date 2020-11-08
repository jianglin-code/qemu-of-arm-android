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

#include "GL2Dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include "osDynLibrary.h"

GL2Dispatch           s_gl2;
int                   s_gl2_enabled;

static osUtils::dynLibrary *s_gles2_lib = NULL;

//
// This function is called only once during initialiation before
// any thread has been created - hence it should NOT be thread safe.
//
// ptr = getProc\("(.*)", userData\); set_(.*);
// s_gl2.$1 = ($1_server_proc_t) s_gles2_lib->findSymbol("$1");
//

bool init_gl2_dispatch()
{
    s_gles2_lib = osUtils::dynLibrary::open("/vendor/lib64/egl/libGLESv2_mtk.so");
    if (!s_gles2_lib) return false;

    s_gl2.glActiveTexture = (glActiveTexture_server_proc_t) s_gles2_lib->findSymbol("glActiveTexture");
    s_gl2.glAttachShader = (glAttachShader_server_proc_t) s_gles2_lib->findSymbol("glAttachShader");
    s_gl2.glBindAttribLocation = (glBindAttribLocation_server_proc_t) s_gles2_lib->findSymbol("glBindAttribLocation");
    s_gl2.glBindBuffer = (glBindBuffer_server_proc_t) s_gles2_lib->findSymbol("glBindBuffer");
    s_gl2.glBindFramebuffer = (glBindFramebuffer_server_proc_t) s_gles2_lib->findSymbol("glBindFramebuffer");
    s_gl2.glBindRenderbuffer = (glBindRenderbuffer_server_proc_t) s_gles2_lib->findSymbol("glBindRenderbuffer");
    s_gl2.glBindTexture = (glBindTexture_server_proc_t) s_gles2_lib->findSymbol("glBindTexture");
    s_gl2.glBlendColor = (glBlendColor_server_proc_t) s_gles2_lib->findSymbol("glBlendColor");
    s_gl2.glBlendEquation = (glBlendEquation_server_proc_t) s_gles2_lib->findSymbol("glBlendEquation");
    s_gl2.glBlendEquationSeparate = (glBlendEquationSeparate_server_proc_t) s_gles2_lib->findSymbol("glBlendEquationSeparate");
    s_gl2.glBlendFunc = (glBlendFunc_server_proc_t) s_gles2_lib->findSymbol("glBlendFunc");
    s_gl2.glBlendFuncSeparate = (glBlendFuncSeparate_server_proc_t) s_gles2_lib->findSymbol("glBlendFuncSeparate");
    s_gl2.glBufferData = (glBufferData_server_proc_t) s_gles2_lib->findSymbol("glBufferData");
    s_gl2.glBufferSubData = (glBufferSubData_server_proc_t) s_gles2_lib->findSymbol("glBufferSubData");
    s_gl2.glCheckFramebufferStatus = (glCheckFramebufferStatus_server_proc_t) s_gles2_lib->findSymbol("glCheckFramebufferStatus");
    s_gl2.glClear = (glClear_server_proc_t) s_gles2_lib->findSymbol("glClear");
    s_gl2.glClearColor = (glClearColor_server_proc_t) s_gles2_lib->findSymbol("glClearColor");
    s_gl2.glClearDepthf = (glClearDepthf_server_proc_t) s_gles2_lib->findSymbol("glClearDepthf");
    s_gl2.glClearStencil = (glClearStencil_server_proc_t) s_gles2_lib->findSymbol("glClearStencil");
    s_gl2.glColorMask = (glColorMask_server_proc_t) s_gles2_lib->findSymbol("glColorMask");
    s_gl2.glCompileShader = (glCompileShader_server_proc_t) s_gles2_lib->findSymbol("glCompileShader");
    s_gl2.glCompressedTexImage2D = (glCompressedTexImage2D_server_proc_t) s_gles2_lib->findSymbol("glCompressedTexImage2D");
    s_gl2.glCompressedTexSubImage2D = (glCompressedTexSubImage2D_server_proc_t) s_gles2_lib->findSymbol("glCompressedTexSubImage2D");
    s_gl2.glCopyTexImage2D = (glCopyTexImage2D_server_proc_t) s_gles2_lib->findSymbol("glCopyTexImage2D");
    s_gl2.glCopyTexSubImage2D = (glCopyTexSubImage2D_server_proc_t) s_gles2_lib->findSymbol("glCopyTexSubImage2D");
    s_gl2.glCreateProgram = (glCreateProgram_server_proc_t) s_gles2_lib->findSymbol("glCreateProgram");
    s_gl2.glCreateShader = (glCreateShader_server_proc_t) s_gles2_lib->findSymbol("glCreateShader");
    s_gl2.glCullFace = (glCullFace_server_proc_t) s_gles2_lib->findSymbol("glCullFace");
    s_gl2.glDeleteBuffers = (glDeleteBuffers_server_proc_t) s_gles2_lib->findSymbol("glDeleteBuffers");
    s_gl2.glDeleteFramebuffers = (glDeleteFramebuffers_server_proc_t) s_gles2_lib->findSymbol("glDeleteFramebuffers");
    s_gl2.glDeleteProgram = (glDeleteProgram_server_proc_t) s_gles2_lib->findSymbol("glDeleteProgram");
    s_gl2.glDeleteRenderbuffers = (glDeleteRenderbuffers_server_proc_t) s_gles2_lib->findSymbol("glDeleteRenderbuffers");
    s_gl2.glDeleteShader = (glDeleteShader_server_proc_t) s_gles2_lib->findSymbol("glDeleteShader");
    s_gl2.glDeleteTextures = (glDeleteTextures_server_proc_t) s_gles2_lib->findSymbol("glDeleteTextures");
    s_gl2.glDepthFunc = (glDepthFunc_server_proc_t) s_gles2_lib->findSymbol("glDepthFunc");
    s_gl2.glDepthMask = (glDepthMask_server_proc_t) s_gles2_lib->findSymbol("glDepthMask");
    s_gl2.glDepthRangef = (glDepthRangef_server_proc_t) s_gles2_lib->findSymbol("glDepthRangef");
    s_gl2.glDetachShader = (glDetachShader_server_proc_t) s_gles2_lib->findSymbol("glDetachShader");
    s_gl2.glDisable = (glDisable_server_proc_t) s_gles2_lib->findSymbol("glDisable");
    s_gl2.glDisableVertexAttribArray = (glDisableVertexAttribArray_server_proc_t) s_gles2_lib->findSymbol("glDisableVertexAttribArray");
    s_gl2.glDrawArrays = (glDrawArrays_server_proc_t) s_gles2_lib->findSymbol("glDrawArrays");
    s_gl2.glDrawElements = (glDrawElements_server_proc_t) s_gles2_lib->findSymbol("glDrawElements");
    s_gl2.glEnable = (glEnable_server_proc_t) s_gles2_lib->findSymbol("glEnable");
    s_gl2.glEnableVertexAttribArray = (glEnableVertexAttribArray_server_proc_t) s_gles2_lib->findSymbol("glEnableVertexAttribArray");
    s_gl2.glFinish = (glFinish_server_proc_t) s_gles2_lib->findSymbol("glFinish");
    s_gl2.glFlush = (glFlush_server_proc_t) s_gles2_lib->findSymbol("glFlush");
    s_gl2.glFramebufferRenderbuffer = (glFramebufferRenderbuffer_server_proc_t) s_gles2_lib->findSymbol("glFramebufferRenderbuffer");
    s_gl2.glFramebufferTexture2D = (glFramebufferTexture2D_server_proc_t) s_gles2_lib->findSymbol("glFramebufferTexture2D");
    s_gl2.glFrontFace = (glFrontFace_server_proc_t) s_gles2_lib->findSymbol("glFrontFace");
    s_gl2.glGenBuffers = (glGenBuffers_server_proc_t) s_gles2_lib->findSymbol("glGenBuffers");
    s_gl2.glGenerateMipmap = (glGenerateMipmap_server_proc_t) s_gles2_lib->findSymbol("glGenerateMipmap");
    s_gl2.glGenFramebuffers = (glGenFramebuffers_server_proc_t) s_gles2_lib->findSymbol("glGenFramebuffers");
    s_gl2.glGenRenderbuffers = (glGenRenderbuffers_server_proc_t) s_gles2_lib->findSymbol("glGenRenderbuffers");
    s_gl2.glGenTextures = (glGenTextures_server_proc_t) s_gles2_lib->findSymbol("glGenTextures");
    s_gl2.glGetActiveAttrib = (glGetActiveAttrib_server_proc_t) s_gles2_lib->findSymbol("glGetActiveAttrib");
    s_gl2.glGetActiveUniform = (glGetActiveUniform_server_proc_t) s_gles2_lib->findSymbol("glGetActiveUniform");
    s_gl2.glGetAttachedShaders = (glGetAttachedShaders_server_proc_t) s_gles2_lib->findSymbol("glGetAttachedShaders");
    s_gl2.glGetAttribLocation = (glGetAttribLocation_server_proc_t) s_gles2_lib->findSymbol("glGetAttribLocation");
    s_gl2.glGetBooleanv = (glGetBooleanv_server_proc_t) s_gles2_lib->findSymbol("glGetBooleanv");
    s_gl2.glGetBufferParameteriv = (glGetBufferParameteriv_server_proc_t) s_gles2_lib->findSymbol("glGetBufferParameteriv");
    s_gl2.glGetError = (glGetError_server_proc_t) s_gles2_lib->findSymbol("glGetError");
    s_gl2.glGetFloatv = (glGetFloatv_server_proc_t) s_gles2_lib->findSymbol("glGetFloatv");
    s_gl2.glGetFramebufferAttachmentParameteriv = (glGetFramebufferAttachmentParameteriv_server_proc_t) s_gles2_lib->findSymbol("glGetFramebufferAttachmentParameteriv");
    s_gl2.glGetIntegerv = (glGetIntegerv_server_proc_t) s_gles2_lib->findSymbol("glGetIntegerv");
    s_gl2.glGetProgramiv = (glGetProgramiv_server_proc_t) s_gles2_lib->findSymbol("glGetProgramiv");
    s_gl2.glGetProgramInfoLog = (glGetProgramInfoLog_server_proc_t) s_gles2_lib->findSymbol("glGetProgramInfoLog");
    s_gl2.glGetRenderbufferParameteriv = (glGetRenderbufferParameteriv_server_proc_t) s_gles2_lib->findSymbol("glGetRenderbufferParameteriv");
    s_gl2.glGetShaderiv = (glGetShaderiv_server_proc_t) s_gles2_lib->findSymbol("glGetShaderiv");
    s_gl2.glGetShaderInfoLog = (glGetShaderInfoLog_server_proc_t) s_gles2_lib->findSymbol("glGetShaderInfoLog");
    s_gl2.glGetShaderPrecisionFormat = (glGetShaderPrecisionFormat_server_proc_t) s_gles2_lib->findSymbol("glGetShaderPrecisionFormat");
    s_gl2.glGetShaderSource = (glGetShaderSource_server_proc_t) s_gles2_lib->findSymbol("glGetShaderSource");
    s_gl2.glGetString = (glGetString_server_proc_t) s_gles2_lib->findSymbol("glGetString");
    s_gl2.glGetTexParameterfv = (glGetTexParameterfv_server_proc_t) s_gles2_lib->findSymbol("glGetTexParameterfv");
    s_gl2.glGetTexParameteriv = (glGetTexParameteriv_server_proc_t) s_gles2_lib->findSymbol("glGetTexParameteriv");
    s_gl2.glGetUniformfv = (glGetUniformfv_server_proc_t) s_gles2_lib->findSymbol("glGetUniformfv");
    s_gl2.glGetUniformiv = (glGetUniformiv_server_proc_t) s_gles2_lib->findSymbol("glGetUniformiv");
    s_gl2.glGetUniformLocation = (glGetUniformLocation_server_proc_t) s_gles2_lib->findSymbol("glGetUniformLocation");
    s_gl2.glGetVertexAttribfv = (glGetVertexAttribfv_server_proc_t) s_gles2_lib->findSymbol("glGetVertexAttribfv");
    s_gl2.glGetVertexAttribiv = (glGetVertexAttribiv_server_proc_t) s_gles2_lib->findSymbol("glGetVertexAttribiv");
    s_gl2.glGetVertexAttribPointerv = (glGetVertexAttribPointerv_server_proc_t) s_gles2_lib->findSymbol("glGetVertexAttribPointerv");
    s_gl2.glHint = (glHint_server_proc_t) s_gles2_lib->findSymbol("glHint");
    s_gl2.glIsBuffer = (glIsBuffer_server_proc_t) s_gles2_lib->findSymbol("glIsBuffer");
    s_gl2.glIsEnabled = (glIsEnabled_server_proc_t) s_gles2_lib->findSymbol("glIsEnabled");
    s_gl2.glIsFramebuffer = (glIsFramebuffer_server_proc_t) s_gles2_lib->findSymbol("glIsFramebuffer");
    s_gl2.glIsProgram = (glIsProgram_server_proc_t) s_gles2_lib->findSymbol("glIsProgram");
    s_gl2.glIsRenderbuffer = (glIsRenderbuffer_server_proc_t) s_gles2_lib->findSymbol("glIsRenderbuffer");
    s_gl2.glIsShader = (glIsShader_server_proc_t) s_gles2_lib->findSymbol("glIsShader");
    s_gl2.glIsTexture = (glIsTexture_server_proc_t) s_gles2_lib->findSymbol("glIsTexture");
    s_gl2.glLineWidth = (glLineWidth_server_proc_t) s_gles2_lib->findSymbol("glLineWidth");
    s_gl2.glLinkProgram = (glLinkProgram_server_proc_t) s_gles2_lib->findSymbol("glLinkProgram");
    s_gl2.glPixelStorei = (glPixelStorei_server_proc_t) s_gles2_lib->findSymbol("glPixelStorei");
    s_gl2.glPolygonOffset = (glPolygonOffset_server_proc_t) s_gles2_lib->findSymbol("glPolygonOffset");
    s_gl2.glReadPixels = (glReadPixels_server_proc_t) s_gles2_lib->findSymbol("glReadPixels");
    s_gl2.glReleaseShaderCompiler = (glReleaseShaderCompiler_server_proc_t) s_gles2_lib->findSymbol("glReleaseShaderCompiler");
    s_gl2.glRenderbufferStorage = (glRenderbufferStorage_server_proc_t) s_gles2_lib->findSymbol("glRenderbufferStorage");
    s_gl2.glSampleCoverage = (glSampleCoverage_server_proc_t) s_gles2_lib->findSymbol("glSampleCoverage");
    s_gl2.glScissor = (glScissor_server_proc_t) s_gles2_lib->findSymbol("glScissor");
    s_gl2.glShaderBinary = (glShaderBinary_server_proc_t) s_gles2_lib->findSymbol("glShaderBinary");
    s_gl2.glShaderSource = (glShaderSource_server_proc_t) s_gles2_lib->findSymbol("glShaderSource");
    s_gl2.glStencilFunc = (glStencilFunc_server_proc_t) s_gles2_lib->findSymbol("glStencilFunc");
    s_gl2.glStencilFuncSeparate = (glStencilFuncSeparate_server_proc_t) s_gles2_lib->findSymbol("glStencilFuncSeparate");
    s_gl2.glStencilMask = (glStencilMask_server_proc_t) s_gles2_lib->findSymbol("glStencilMask");
    s_gl2.glStencilMaskSeparate = (glStencilMaskSeparate_server_proc_t) s_gles2_lib->findSymbol("glStencilMaskSeparate");
    s_gl2.glStencilOp = (glStencilOp_server_proc_t) s_gles2_lib->findSymbol("glStencilOp");
    s_gl2.glStencilOpSeparate = (glStencilOpSeparate_server_proc_t) s_gles2_lib->findSymbol("glStencilOpSeparate");
    s_gl2.glTexImage2D = (glTexImage2D_server_proc_t) s_gles2_lib->findSymbol("glTexImage2D");
    s_gl2.glTexParameterf = (glTexParameterf_server_proc_t) s_gles2_lib->findSymbol("glTexParameterf");
    s_gl2.glTexParameterfv = (glTexParameterfv_server_proc_t) s_gles2_lib->findSymbol("glTexParameterfv");
    s_gl2.glTexParameteri = (glTexParameteri_server_proc_t) s_gles2_lib->findSymbol("glTexParameteri");
    s_gl2.glTexParameteriv = (glTexParameteriv_server_proc_t) s_gles2_lib->findSymbol("glTexParameteriv");
    s_gl2.glTexSubImage2D = (glTexSubImage2D_server_proc_t) s_gles2_lib->findSymbol("glTexSubImage2D");
    s_gl2.glUniform1f = (glUniform1f_server_proc_t) s_gles2_lib->findSymbol("glUniform1f");
    s_gl2.glUniform1fv = (glUniform1fv_server_proc_t) s_gles2_lib->findSymbol("glUniform1fv");
    s_gl2.glUniform1i = (glUniform1i_server_proc_t) s_gles2_lib->findSymbol("glUniform1i");
    s_gl2.glUniform1iv = (glUniform1iv_server_proc_t) s_gles2_lib->findSymbol("glUniform1iv");
    s_gl2.glUniform2f = (glUniform2f_server_proc_t) s_gles2_lib->findSymbol("glUniform2f");
    s_gl2.glUniform2fv = (glUniform2fv_server_proc_t) s_gles2_lib->findSymbol("glUniform2fv");
    s_gl2.glUniform2i = (glUniform2i_server_proc_t) s_gles2_lib->findSymbol("glUniform2i");
    s_gl2.glUniform2iv = (glUniform2iv_server_proc_t) s_gles2_lib->findSymbol("glUniform2iv");
    s_gl2.glUniform3f = (glUniform3f_server_proc_t) s_gles2_lib->findSymbol("glUniform3f");
    s_gl2.glUniform3fv = (glUniform3fv_server_proc_t) s_gles2_lib->findSymbol("glUniform3fv");
    s_gl2.glUniform3i = (glUniform3i_server_proc_t) s_gles2_lib->findSymbol("glUniform3i");
    s_gl2.glUniform3iv = (glUniform3iv_server_proc_t) s_gles2_lib->findSymbol("glUniform3iv");
    s_gl2.glUniform4f = (glUniform4f_server_proc_t) s_gles2_lib->findSymbol("glUniform4f");
    s_gl2.glUniform4fv = (glUniform4fv_server_proc_t) s_gles2_lib->findSymbol("glUniform4fv");
    s_gl2.glUniform4i = (glUniform4i_server_proc_t) s_gles2_lib->findSymbol("glUniform4i");
    s_gl2.glUniform4iv = (glUniform4iv_server_proc_t) s_gles2_lib->findSymbol("glUniform4iv");
    s_gl2.glUniformMatrix2fv = (glUniformMatrix2fv_server_proc_t) s_gles2_lib->findSymbol("glUniformMatrix2fv");
    s_gl2.glUniformMatrix3fv = (glUniformMatrix3fv_server_proc_t) s_gles2_lib->findSymbol("glUniformMatrix3fv");
    s_gl2.glUniformMatrix4fv = (glUniformMatrix4fv_server_proc_t) s_gles2_lib->findSymbol("glUniformMatrix4fv");
    s_gl2.glUseProgram = (glUseProgram_server_proc_t) s_gles2_lib->findSymbol("glUseProgram");
    s_gl2.glValidateProgram = (glValidateProgram_server_proc_t) s_gles2_lib->findSymbol("glValidateProgram");
    s_gl2.glVertexAttrib1f = (glVertexAttrib1f_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib1f");
    s_gl2.glVertexAttrib1fv = (glVertexAttrib1fv_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib1fv");
    s_gl2.glVertexAttrib2f = (glVertexAttrib2f_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib2f");
    s_gl2.glVertexAttrib2fv = (glVertexAttrib2fv_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib2fv");
    s_gl2.glVertexAttrib3f = (glVertexAttrib3f_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib3f");
    s_gl2.glVertexAttrib3fv = (glVertexAttrib3fv_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib3fv");
    s_gl2.glVertexAttrib4f = (glVertexAttrib4f_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib4f");
    s_gl2.glVertexAttrib4fv = (glVertexAttrib4fv_server_proc_t) s_gles2_lib->findSymbol("glVertexAttrib4fv");
    s_gl2.glVertexAttribPointer = (glVertexAttribPointer_server_proc_t) s_gles2_lib->findSymbol("glVertexAttribPointer");
    s_gl2.glViewport = (glViewport_server_proc_t) s_gles2_lib->findSymbol("glViewport");
    s_gl2.glEGLImageTargetTexture2DOES = (glEGLImageTargetTexture2DOES_server_proc_t) s_gles2_lib->findSymbol("glEGLImageTargetTexture2DOES");
    s_gl2.glEGLImageTargetRenderbufferStorageOES = (glEGLImageTargetRenderbufferStorageOES_server_proc_t) s_gles2_lib->findSymbol("glEGLImageTargetRenderbufferStorageOES");
    s_gl2.glGetProgramBinaryOES = (glGetProgramBinaryOES_server_proc_t) s_gles2_lib->findSymbol("glGetProgramBinaryOES");
    s_gl2.glProgramBinaryOES = (glProgramBinaryOES_server_proc_t) s_gles2_lib->findSymbol("glProgramBinaryOES");
    s_gl2.glMapBufferOES = (glMapBufferOES_server_proc_t) s_gles2_lib->findSymbol("glMapBufferOES");
    s_gl2.glUnmapBufferOES = (glUnmapBufferOES_server_proc_t) s_gles2_lib->findSymbol("glUnmapBufferOES");
    s_gl2.glTexImage3DOES = (glTexImage3DOES_server_proc_t) s_gles2_lib->findSymbol("glTexImage3DOES");
    s_gl2.glTexSubImage3DOES = (glTexSubImage3DOES_server_proc_t) s_gles2_lib->findSymbol("glTexSubImage3DOES");
    s_gl2.glCopyTexSubImage3DOES = (glCopyTexSubImage3DOES_server_proc_t) s_gles2_lib->findSymbol("glCopyTexSubImage3DOES");
    s_gl2.glCompressedTexImage3DOES = (glCompressedTexImage3DOES_server_proc_t) s_gles2_lib->findSymbol("glCompressedTexImage3DOES");
    s_gl2.glCompressedTexSubImage3DOES = (glCompressedTexSubImage3DOES_server_proc_t) s_gles2_lib->findSymbol("glCompressedTexSubImage3DOES");
    s_gl2.glFramebufferTexture3DOES = (glFramebufferTexture3DOES_server_proc_t) s_gles2_lib->findSymbol("glFramebufferTexture3DOES");
    s_gl2.glBindVertexArrayOES = (glBindVertexArrayOES_server_proc_t) s_gles2_lib->findSymbol("glBindVertexArrayOES");
    s_gl2.glDeleteVertexArraysOES = (glDeleteVertexArraysOES_server_proc_t) s_gles2_lib->findSymbol("glDeleteVertexArraysOES");
    s_gl2.glGenVertexArraysOES = (glGenVertexArraysOES_server_proc_t) s_gles2_lib->findSymbol("glGenVertexArraysOES");
    s_gl2.glIsVertexArrayOES = (glIsVertexArrayOES_server_proc_t) s_gles2_lib->findSymbol("glIsVertexArrayOES");
    s_gl2.glDiscardFramebufferEXT = (glDiscardFramebufferEXT_server_proc_t) s_gles2_lib->findSymbol("glDiscardFramebufferEXT");
    s_gl2.glMultiDrawArraysEXT = (glMultiDrawArraysEXT_server_proc_t) s_gles2_lib->findSymbol("glMultiDrawArraysEXT");
    s_gl2.glMultiDrawElementsEXT = (glMultiDrawElementsEXT_server_proc_t) s_gles2_lib->findSymbol("glMultiDrawElementsEXT");
    s_gl2.glGetPerfMonitorGroupsAMD = (glGetPerfMonitorGroupsAMD_server_proc_t) s_gles2_lib->findSymbol("glGetPerfMonitorGroupsAMD");
    s_gl2.glGetPerfMonitorCountersAMD = (glGetPerfMonitorCountersAMD_server_proc_t) s_gles2_lib->findSymbol("glGetPerfMonitorCountersAMD");
    s_gl2.glGetPerfMonitorGroupStringAMD = (glGetPerfMonitorGroupStringAMD_server_proc_t) s_gles2_lib->findSymbol("glGetPerfMonitorGroupStringAMD");
    s_gl2.glGetPerfMonitorCounterStringAMD = (glGetPerfMonitorCounterStringAMD_server_proc_t) s_gles2_lib->findSymbol("glGetPerfMonitorCounterStringAMD");
    s_gl2.glGetPerfMonitorCounterInfoAMD = (glGetPerfMonitorCounterInfoAMD_server_proc_t) s_gles2_lib->findSymbol("glGetPerfMonitorCounterInfoAMD");
    s_gl2.glGenPerfMonitorsAMD = (glGenPerfMonitorsAMD_server_proc_t) s_gles2_lib->findSymbol("glGenPerfMonitorsAMD");
    s_gl2.glDeletePerfMonitorsAMD = (glDeletePerfMonitorsAMD_server_proc_t) s_gles2_lib->findSymbol("glDeletePerfMonitorsAMD");
    s_gl2.glSelectPerfMonitorCountersAMD = (glSelectPerfMonitorCountersAMD_server_proc_t) s_gles2_lib->findSymbol("glSelectPerfMonitorCountersAMD");
    s_gl2.glBeginPerfMonitorAMD = (glBeginPerfMonitorAMD_server_proc_t) s_gles2_lib->findSymbol("glBeginPerfMonitorAMD");
    s_gl2.glEndPerfMonitorAMD = (glEndPerfMonitorAMD_server_proc_t) s_gles2_lib->findSymbol("glEndPerfMonitorAMD");
    s_gl2.glGetPerfMonitorCounterDataAMD = (glGetPerfMonitorCounterDataAMD_server_proc_t) s_gles2_lib->findSymbol("glGetPerfMonitorCounterDataAMD");
    s_gl2.glRenderbufferStorageMultisampleIMG = (glRenderbufferStorageMultisampleIMG_server_proc_t) s_gles2_lib->findSymbol("glRenderbufferStorageMultisampleIMG");
    s_gl2.glFramebufferTexture2DMultisampleIMG = (glFramebufferTexture2DMultisampleIMG_server_proc_t) s_gles2_lib->findSymbol("glFramebufferTexture2DMultisampleIMG");
    s_gl2.glDeleteFencesNV = (glDeleteFencesNV_server_proc_t) s_gles2_lib->findSymbol("glDeleteFencesNV");
    s_gl2.glGenFencesNV = (glGenFencesNV_server_proc_t) s_gles2_lib->findSymbol("glGenFencesNV");
    s_gl2.glIsFenceNV = (glIsFenceNV_server_proc_t) s_gles2_lib->findSymbol("glIsFenceNV");
    s_gl2.glTestFenceNV = (glTestFenceNV_server_proc_t) s_gles2_lib->findSymbol("glTestFenceNV");
    s_gl2.glGetFenceivNV = (glGetFenceivNV_server_proc_t) s_gles2_lib->findSymbol("glGetFenceivNV");
    s_gl2.glFinishFenceNV = (glFinishFenceNV_server_proc_t) s_gles2_lib->findSymbol("glFinishFenceNV");
    s_gl2.glSetFenceNV = (glSetFenceNV_server_proc_t) s_gles2_lib->findSymbol("glSetFenceNV");
    s_gl2.glCoverageMaskNV = (glCoverageMaskNV_server_proc_t) s_gles2_lib->findSymbol("glCoverageMaskNV");
    s_gl2.glCoverageOperationNV = (glCoverageOperationNV_server_proc_t) s_gles2_lib->findSymbol("glCoverageOperationNV");
    s_gl2.glGetDriverControlsQCOM = (glGetDriverControlsQCOM_server_proc_t) s_gles2_lib->findSymbol("glGetDriverControlsQCOM");
    s_gl2.glGetDriverControlStringQCOM = (glGetDriverControlStringQCOM_server_proc_t) s_gles2_lib->findSymbol("glGetDriverControlStringQCOM");
    s_gl2.glEnableDriverControlQCOM = (glEnableDriverControlQCOM_server_proc_t) s_gles2_lib->findSymbol("glEnableDriverControlQCOM");
    s_gl2.glDisableDriverControlQCOM = (glDisableDriverControlQCOM_server_proc_t) s_gles2_lib->findSymbol("glDisableDriverControlQCOM");
    s_gl2.glExtGetTexturesQCOM = (glExtGetTexturesQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetTexturesQCOM");
    s_gl2.glExtGetBuffersQCOM = (glExtGetBuffersQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetBuffersQCOM");
    s_gl2.glExtGetRenderbuffersQCOM = (glExtGetRenderbuffersQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetRenderbuffersQCOM");
    s_gl2.glExtGetFramebuffersQCOM = (glExtGetFramebuffersQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetFramebuffersQCOM");
    s_gl2.glExtGetTexLevelParameterivQCOM = (glExtGetTexLevelParameterivQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetTexLevelParameterivQCOM");
    s_gl2.glExtTexObjectStateOverrideiQCOM = (glExtTexObjectStateOverrideiQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtTexObjectStateOverrideiQCOM");
    s_gl2.glExtGetTexSubImageQCOM = (glExtGetTexSubImageQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetTexSubImageQCOM");
    s_gl2.glExtGetBufferPointervQCOM = (glExtGetBufferPointervQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetBufferPointervQCOM");
    s_gl2.glExtGetShadersQCOM = (glExtGetShadersQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetShadersQCOM");
    s_gl2.glExtGetProgramsQCOM = (glExtGetProgramsQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetProgramsQCOM");
    s_gl2.glExtIsProgramBinaryQCOM = (glExtIsProgramBinaryQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtIsProgramBinaryQCOM");
    s_gl2.glExtGetProgramBinarySourceQCOM = (glExtGetProgramBinarySourceQCOM_server_proc_t) s_gles2_lib->findSymbol("glExtGetProgramBinarySourceQCOM");
    s_gl2.glStartTilingQCOM = (glStartTilingQCOM_server_proc_t) s_gles2_lib->findSymbol("glStartTilingQCOM");
    s_gl2.glEndTilingQCOM = (glEndTilingQCOM_server_proc_t) s_gles2_lib->findSymbol("glEndTilingQCOM");
    s_gl2.glVertexAttribPointerData = (glVertexAttribPointerData_server_proc_t) s_gles2_lib->findSymbol("glVertexAttribPointerData");
    s_gl2.glVertexAttribPointerOffset = (glVertexAttribPointerOffset_server_proc_t) s_gles2_lib->findSymbol("glVertexAttribPointerOffset");
    s_gl2.glDrawElementsOffset = (glDrawElementsOffset_server_proc_t) s_gles2_lib->findSymbol("glDrawElementsOffset");
    s_gl2.glDrawElementsData = (glDrawElementsData_server_proc_t) s_gles2_lib->findSymbol("glDrawElementsData");
    s_gl2.glGetCompressedTextureFormats = (glGetCompressedTextureFormats_server_proc_t) s_gles2_lib->findSymbol("glGetCompressedTextureFormats");
    s_gl2.glShaderString = (glShaderString_server_proc_t) s_gles2_lib->findSymbol("glShaderString");
    s_gl2.glFinishRoundTrip = (glFinishRoundTrip_server_proc_t) s_gles2_lib->findSymbol("glFinishRoundTrip");

    s_gl2_enabled = true;
    return true;
}

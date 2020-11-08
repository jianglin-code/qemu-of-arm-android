#ifndef _EGL_DISPATCH_H
#define _EGL_DISPATCH_H

#include "egl_proc.h"

struct EGLDispatch {
    eglGetError_t eglGetError;
    eglGetDisplay_t eglGetDisplay;
    eglInitialize_t eglInitialize;
    eglTerminate_t eglTerminate;
    eglQueryString_t eglQueryString;
    eglGetConfigs_t eglGetConfigs;
    eglChooseConfig_t eglChooseConfig;
    eglGetConfigAttrib_t eglGetConfigAttrib;
    eglCreateWindowSurface_t eglCreateWindowSurface;
    eglCreatePbufferSurface_t eglCreatePbufferSurface;
    eglCreatePixmapSurface_t eglCreatePixmapSurface;
    eglDestroySurface_t eglDestroySurface;
    eglQuerySurface_t eglQuerySurface;
    eglBindAPI_t eglBindAPI;
    eglQueryAPI_t eglQueryAPI;
    eglWaitClient_t eglWaitClient;
    eglReleaseThread_t eglReleaseThread;
    eglCreatePbufferFromClientBuffer_t eglCreatePbufferFromClientBuffer;
    eglSurfaceAttrib_t eglSurfaceAttrib;
    eglBindTexImage_t eglBindTexImage;
    eglReleaseTexImage_t eglReleaseTexImage;
    eglSwapInterval_t eglSwapInterval;
    eglCreateContext_t eglCreateContext;
    eglDestroyContext_t eglDestroyContext;
    eglMakeCurrent_t eglMakeCurrent;
    eglGetCurrentContext_t eglGetCurrentContext;
    eglGetCurrentSurface_t eglGetCurrentSurface;
    eglGetCurrentDisplay_t eglGetCurrentDisplay;
    eglQueryContext_t eglQueryContext;
    eglWaitGL_t eglWaitGL;
    eglWaitNative_t eglWaitNative;
    eglSwapBuffers_t eglSwapBuffers;
    eglCopyBuffers_t eglCopyBuffers;
    eglGetProcAddress_t eglGetProcAddress;
    eglLockSurfaceKHR_t eglLockSurfaceKHR;
    eglUnlockSurfaceKHR_t eglUnlockSurfaceKHR;
    eglCreateImageKHR_t eglCreateImageKHR;
    eglDestroyImageKHR_t eglDestroyImageKHR;
    eglCreateSyncKHR_t eglCreateSyncKHR;
    eglDestroySyncKHR_t eglDestroySyncKHR;
    eglClientWaitSyncKHR_t eglClientWaitSyncKHR;
    eglSignalSyncKHR_t eglSignalSyncKHR;
    eglGetSyncAttribKHR_t eglGetSyncAttribKHR;
    eglSetSwapRectangleANDROID_t eglSetSwapRectangleANDROID;
};

bool init_egl_dispatch();

extern EGLDispatch s_egl;

#endif

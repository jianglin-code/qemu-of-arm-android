LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=ColorBuffer.cpp \
                 EGLDispatch.cpp \
                 FBConfig.cpp \
                 FrameBuffer.cpp \
                 GL2Decoder.cpp \
                 GL2Dispatch.cpp \
                 GLDecoder.cpp \
                 GLDispatch.cpp \
                 glUtils.cpp \
                 lazy_instance.cpp \
                 main.cpp \
                 NativeLinuxSubWindow.cpp \
                 osDynLibrary.cpp \
                 osProcessUnix.cpp \
                 osThreadUnix.cpp \
                 ReadBuffer.cpp \
                 render_api.cpp \
                 RenderContext.cpp \
                 RenderControl.cpp \
                 renderControl_dec.cpp \
                 renderControl_server_context.cpp \
                 RenderServer.cpp \
                 RenderThread.cpp \
                 smart_ptr.cpp \
                 sockets.cpp \
                 SocketStream.cpp \
                 TcpStream.cpp \
                 ThreadInfo.cpp \
                 thread_store.cpp \
                 TimeUtils.cpp \
                 UnixStream.cpp \
                 WindowSurface.cpp \
                 FramebufferNativeWindow.cpp \
                 AndroidWindowSurface.cpp 

LOCAL_SHARED_LIBRARIES += libdl libutils libcutils libui libgui libhardware
LOCAL_C_INCLUDES += bionic/libc/private \
                    external/libcxx/include

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DWITH_GLES2 -fPIC 
#LOCAL_CFLAGS += -fvisibility=hidden -DDEBUG_PRINTOUT -DCHECK_GL_ERROR -DEMUGL_DEBUG

LOCAL_MODULE:= openglApi

include $(BUILD_EXECUTABLE)
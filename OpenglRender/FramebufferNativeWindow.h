
#ifndef ANDROID_FRAMEBUFFER_NATIVE_WINDOW_H
#define ANDROID_FRAMEBUFFER_NATIVE_WINDOW_H

#include <stdint.h>
#include <sys/types.h>

#include "egl.h"

#include <utils/threads.h>
#include <utils/String8.h>

#include <ui/ANativeObjectBase.h>
#include <ui/Rect.h>
#include <ui/GraphicBuffer.h>

#define MIN_NUM_FRAME_BUFFERS  2
#define MAX_NUM_FRAME_BUFFERS  1

class FramebufferNativeWindow 
    : public android::ANativeObjectBase<
        ANativeWindow, 
        FramebufferNativeWindow, 
        android::LightRefBase<FramebufferNativeWindow> >
{
public:
    FramebufferNativeWindow();
    ~FramebufferNativeWindow();
    friend class LightRefBase<FramebufferNativeWindow>;
    static int setSwapInterval(ANativeWindow* window, int interval);
    static int dequeueBuffer(ANativeWindow* window, ANativeWindowBuffer** buffer, int* fenceFd);
    static int queueBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd);
    static int query(const ANativeWindow* window, int what, int* value);
    static int perform(ANativeWindow* window, int operation, ...);
    static int dequeueBuffer_DEPRECATED(ANativeWindow* window, ANativeWindowBuffer** buffer);
    static int queueBuffer_DEPRECATED(ANativeWindow* window, ANativeWindowBuffer* buffer);
    static int lockBuffer_DEPRECATED(ANativeWindow* window, ANativeWindowBuffer* buffer);
};

#endif // ANDROID_FRAMEBUFFER_NATIVE_WINDOW_H


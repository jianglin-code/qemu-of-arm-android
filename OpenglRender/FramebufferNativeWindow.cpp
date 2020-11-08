#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ui/GraphicBufferMapper.h>
#include "FramebufferNativeWindow.h"

android::GraphicBuffer* buffers[MAX_NUM_FRAME_BUFFERS];

FramebufferNativeWindow::FramebufferNativeWindow() 
    : BASE()
{
    for (int i = 0; i < MAX_NUM_FRAME_BUFFERS; i++)
    {
        buffers[i] = new android::GraphicBuffer(1080, 1920, 2, GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE);

        // ANativeWindowBuffer* buffer = buffers[i]->getNativeBuffer();
        // printf("usage:%d stride:%d format:%d width:%d height:%d\n", buffer->usage, buffer->stride, buffer->format, buffer->width, buffer->height);
        // private_handle_t* hnd= (private_handle_t*)buffer->handle;
        // printf("fd:%d magic:%d flags:%d base:%d size:%d offset:%d\n",hnd->fd,hnd->magic,hnd->flags,hnd->base,hnd->size,hnd->offset); 
        // printf("initCheck:%d registerBuffer:%d\n",buffers[i]->initCheck(),android::GraphicBufferMapper::get().registerBuffer(buffers[i]->getNativeBuffer()->handle));
        printf("initCheck:%d \n",buffers[i]->initCheck());
        // size_t size = buffers[i]->getFlattenedSize();
        // size_t count = buffers[i]->getFdCount();

        // printf("size:%d count:%d \n",size,count);
        // int* buf = new int[size/sizeof(int)];
        // int* fds = new int[count];

        // printf("flatten \n");
        // size_t tempsize = size;
        // size_t tempcount = count;
        // void* tempbuf = static_cast<void*>(buf);
        // int * tempfds = fds;
        // int err = buffers[i]->flatten((void*&)tempbuf,tempsize,tempfds,tempcount);
        // printf("size:%d count:%d err:%d \n",size,count,err);

        // if (size < 8*sizeof(int))
        //     printf("size < 8*sizeof(int) \n");

        // int* buf1 = static_cast<int*>(buf);
        // if (buf1[0] != 'GBFR')
        //     printf("buf[0] != 'GBFR' \n");

        // const size_t numFds  = buf1[8];
        // const size_t numInts = buf1[9];

        // const size_t sizeNeeded = (10 + numInts) * sizeof(int);
        // if (size < sizeNeeded)
        //     printf("size < sizeNeeded numFds:%d numInts:%d \n",numFds,numInts);

        // size_t fdCountNeeded = 0;
        // if (count < fdCountNeeded)
        //     printf("count < fdCountNeeded \n");

        // err = buffers[i]->unflatten((const void*&)(buf),size,(const int*&)(fds),count);
        // printf("size:%d count:%d err:%d \n",size,count,err);
        //delete[] buf; delete[] fds;
    }

    // const_cast<uint32_t&>(ANativeWindow::flags) = fbDev->flags; 
    // const_cast<float&>(ANativeWindow::xdpi) = fbDev->xdpi;
    // const_cast<float&>(ANativeWindow::ydpi) = fbDev->ydpi;
    const_cast<int&>(ANativeWindow::minSwapInterval) = 0;
    const_cast<int&>(ANativeWindow::maxSwapInterval) = 1;

    ANativeWindow::setSwapInterval = setSwapInterval;
    ANativeWindow::dequeueBuffer = dequeueBuffer;
    ANativeWindow::queueBuffer = queueBuffer;
    ANativeWindow::query = query;
    ANativeWindow::perform = perform;

    ANativeWindow::dequeueBuffer_DEPRECATED = dequeueBuffer_DEPRECATED;
    ANativeWindow::lockBuffer_DEPRECATED = lockBuffer_DEPRECATED;
    ANativeWindow::queueBuffer_DEPRECATED = queueBuffer_DEPRECATED;
}

FramebufferNativeWindow::~FramebufferNativeWindow() 
{

}

int FramebufferNativeWindow::setSwapInterval(
        ANativeWindow* window, int interval) 
{
    printf("setSwapInterval \n ");
    return android::NO_ERROR;
}

int FramebufferNativeWindow::dequeueBuffer_DEPRECATED(ANativeWindow* window, 
        ANativeWindowBuffer** buffer)
{
    printf("dequeueBuffer_DEPRECATED \n ");
    int fenceFd = -1;
    int result = dequeueBuffer(window, buffer, &fenceFd);
    return result;
}

int FramebufferNativeWindow::lockBuffer_DEPRECATED(ANativeWindow* /*window*/, 
        ANativeWindowBuffer* /*buffer*/)
{
    printf("lockBuffer_DEPRECATED \n ");
    return android::NO_ERROR;
}

int FramebufferNativeWindow::queueBuffer_DEPRECATED(ANativeWindow* window, 
        ANativeWindowBuffer* buffer)
{
    printf("queueBuffer_DEPRECATED \n ");
    return queueBuffer(window, buffer, -1);
}

int FramebufferNativeWindow::dequeueBuffer(ANativeWindow* window, 
        ANativeWindowBuffer** buffer, int* fenceFd)
{
    printf("dequeueBuffer \n ");
    FramebufferNativeWindow* self = getSelf(window);
    *buffer = buffers[0];
    *fenceFd = -1;
    printf("dequeueBuffer end \n ");
    return android::NO_ERROR;
}

int FramebufferNativeWindow::queueBuffer(ANativeWindow* window, 
        ANativeWindowBuffer* buffer, int fenceFd)
{
    printf("queueBuffer \n ");
    return android::NO_ERROR;
}

int FramebufferNativeWindow::query(const ANativeWindow* window,
        int what, int* value) 
{
    printf("query what:%d\n ",what);
    const FramebufferNativeWindow* self = getSelf(window);
    switch (what) {
        case NATIVE_WINDOW_WIDTH:
            *value = 1080;
            return android::NO_ERROR;
        case NATIVE_WINDOW_HEIGHT:
            *value = 1920;
            return android::NO_ERROR;
        case NATIVE_WINDOW_FORMAT:
            *value = 1;
            return android::NO_ERROR;
        case NATIVE_WINDOW_CONCRETE_TYPE:
            *value = NATIVE_WINDOW_SURFACE;
            return android::NO_ERROR;
        case NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER:
            *value = 0;
            return android::NO_ERROR;
        case NATIVE_WINDOW_DEFAULT_WIDTH:
            *value = 0;
            return android::NO_ERROR;
        case NATIVE_WINDOW_DEFAULT_HEIGHT:
            *value = 0;
            return android::NO_ERROR;
        case NATIVE_WINDOW_TRANSFORM_HINT:
            *value = 0;
            return android::NO_ERROR;
    }
    *value = 0;
    return android::BAD_VALUE;
}

int FramebufferNativeWindow::perform(ANativeWindow* /*window*/,
        int operation, ...)
{
    printf("perform \n ");
    switch (operation) {
        case NATIVE_WINDOW_CONNECT:
        case NATIVE_WINDOW_DISCONNECT:
        case NATIVE_WINDOW_SET_USAGE:
        case NATIVE_WINDOW_SET_BUFFERS_GEOMETRY:
        case NATIVE_WINDOW_SET_BUFFERS_DIMENSIONS:
        case NATIVE_WINDOW_SET_BUFFERS_FORMAT:
        case NATIVE_WINDOW_SET_BUFFERS_TRANSFORM:
        case NATIVE_WINDOW_API_CONNECT:
        case NATIVE_WINDOW_API_DISCONNECT:
            // TODO: we should implement these
            return android::NO_ERROR;

        case NATIVE_WINDOW_LOCK:
        case NATIVE_WINDOW_UNLOCK_AND_POST:
        case NATIVE_WINDOW_SET_CROP:
        case NATIVE_WINDOW_SET_BUFFER_COUNT:
        case NATIVE_WINDOW_SET_BUFFERS_TIMESTAMP:
        case NATIVE_WINDOW_SET_SCALING_MODE:
            return android::INVALID_OPERATION;
    }
    return android::NAME_NOT_FOUND;
}

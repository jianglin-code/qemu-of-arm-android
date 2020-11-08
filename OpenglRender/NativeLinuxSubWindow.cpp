#include "NativeSubWindow.h"
#include "FramebufferNativeWindow.h"
//#include <ui/FramebufferNativeWindow.h>
#include "AndroidWindowSurface.h"
#include <stdio.h>
#include "gralloc_priv.h"

static FramebufferNativeWindow* m_NativeWindow = 0;

EGLNativeWindowType createSubWindow(FBNativeWindowType p_window,
                                    EGLNativeDisplayType* display_out,
                                    int x, int y,int width, int height)
{

    static AndroidWindowSurface m_windowSurface;
    ANativeWindow* win = m_windowSurface.getSurface();
    // int w,h,f,qw,qh,qhi;
    // win->query(win,NATIVE_WINDOW_WIDTH,&w);
    // win->query(win,NATIVE_WINDOW_HEIGHT,&h);
    // win->query(win,NATIVE_WINDOW_FORMAT,&f);
    // win->query(win,NATIVE_WINDOW_DEFAULT_WIDTH,&qw);
    // win->query(win,NATIVE_WINDOW_DEFAULT_HEIGHT,&qh);
    // win->query(win,NATIVE_WINDOW_TRANSFORM_HINT,&qhi);
    // printf("w:%d h:%d f:%d qw:%d qh:%d qhi:%d \n ",w,h,f,qw,qh,qhi);
    //printf("flags:%d xdpi:%5.5f ydpi:%5.5f \n", win->flags, win->xdpi, win->ydpi);
    // ANativeWindowBuffer* buffer=NULL;
    // int fenceFd=0;
    // int result = win->dequeueBuffer(win,&buffer,&fenceFd);
    // if (buffer)
    // {
    // 	printf("usage:%d stride:%d format:%d width:%d height:%d\n", buffer->usage, buffer->stride, buffer->format, buffer->width, buffer->height);
    // 	private_handle_t* hnd= (private_handle_t*)buffer->handle;

    // 	printf("fd:%d magic:%d flags:%d base:%d size:%d offset:%d\n",hnd->fd,hnd->magic,hnd->flags,hnd->base,hnd->size,hnd->offset); 

    // 	printf("fenceFd:%d result:%d \n",fenceFd,result); 
    // }
    return win;
    
    // ANativeWindow* win = m_windowSurface.getSurface();
    // ANativeWindowBuffer* buffer=NULL;
    // int fenceFd=0;
    // win->dequeueBuffer(win,&buffer,&fenceFd);
    // if (buffer)
    // {
    // 	printf("usage:%d stride:%d format:%d width:%d height:%d\n", buffer->usage, buffer->stride, buffer->format, buffer->width, buffer->height);
    // 	private_handle_t* hnd= (private_handle_t*)buffer->handle;

    // 	printf("fd:%d magic:%d flags:%d base:%d size:%d offset:%d\n",hnd->fd,hnd->magic,hnd->flags,hnd->base,hnd->size,hnd->offset); 
    // }
   
    //return m_windowSurface.getSurface();

	if (NULL == m_NativeWindow)
	{	
		printf("createSubWindow begin\n");
		m_NativeWindow = new FramebufferNativeWindow();	
		printf("createSubWindow end\n");

		//ANativeWindowBuffer* buffer=NULL;
	    // int fenceFd=0;
	    // m_NativeWindow->dequeueBuffer(m_NativeWindow,&buffer,&fenceFd);
	    // if (buffer)
	    // {
	    // 	printf("usage:%d stride:%d format:%d width:%d height:%d\n", buffer->usage, buffer->stride, buffer->format, buffer->width, buffer->height);
	    // 	private_handle_t* hnd= (private_handle_t*)buffer->handle;

	    // 	printf("fd:%d magic:%d flags:%d base:%d size:%d offset:%d\n",hnd->fd,hnd->magic,hnd->flags,hnd->base,hnd->size,hnd->offset); 
	    // }
	}

	return (EGLNativeWindowType)m_NativeWindow;
}

void destroySubWindow(EGLNativeDisplayType dis,EGLNativeWindowType win)
{
	// if (NULL != m_NativeWindow)
	// {
	// 	delete m_NativeWindow;
	// 	m_NativeWindow = NULL;
	// }
}

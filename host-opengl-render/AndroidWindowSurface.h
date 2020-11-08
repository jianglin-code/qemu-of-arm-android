#ifndef OPENGL_WINDOWSURFACE_H
#define OPENGL_WINDOWSURFACE_H

#include <gui/SurfaceControl.h>

#include "egl.h"

using namespace android;

class AndroidWindowSurface {
public:
    AndroidWindowSurface();

    EGLNativeWindowType getSurface() const;

private:
    AndroidWindowSurface(const AndroidWindowSurface&);
    AndroidWindowSurface& operator=(const AndroidWindowSurface&);

    sp<SurfaceControl> mSurfaceControl;
};

#endif /* OPENGL_WINDOWSURFACE_H */

#include "AndroidWindowSurface.h"
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>

AndroidWindowSurface::AndroidWindowSurface() {
    status_t err;

    sp<SurfaceComposerClient> surfaceComposerClient = new SurfaceComposerClient;
    err = surfaceComposerClient->initCheck();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposerClient::initCheck error: %#x\n", err);
        return;
    }

    sp<IBinder> mainDpy = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    DisplayInfo mainDpyInfo;
    err = SurfaceComposerClient::getDisplayInfo(mainDpy, &mainDpyInfo);
    if (err != NO_ERROR) {
        fprintf(stderr, "ERROR: unable to get display characteristics\n");
        return;
    }

    uint32_t width, height;
    if (mainDpyInfo.orientation != DISPLAY_ORIENTATION_0 &&
            mainDpyInfo.orientation != DISPLAY_ORIENTATION_180) {
        width = mainDpyInfo.h;
        height = mainDpyInfo.w;
    } else {
        width = mainDpyInfo.w;
        height = mainDpyInfo.h;
    }

    sp<SurfaceControl> sc = surfaceComposerClient->createSurface(
            String8("Benchmark"), width, height,
            PIXEL_FORMAT_RGBX_8888, ISurfaceComposerClient::eOpaque);
    if (sc == NULL || !sc->isValid()) {
        fprintf(stderr, "Failed to create SurfaceControl\n");
        return;
    }

    SurfaceComposerClient::openGlobalTransaction();
    err = sc->setLayer(0x7FFFFFFF);
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::setLayer error: %#x\n", err);
        return;
    }

    err = sc->show();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::show error: %#x\n", err);
        return;
    }
    SurfaceComposerClient::closeGlobalTransaction();

    mSurfaceControl = sc;
}

EGLNativeWindowType AndroidWindowSurface::getSurface() const {
    sp<ANativeWindow> anw = mSurfaceControl->getSurface();
    return (EGLNativeWindowType) anw.get();
}


#include "config-host.h"
#include "opengles.h"
#include "hw-pipe-net.h"
#include <assert.h>
//#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint32_t FBNativeWindowType;

typedef struct ADynamicLibrary   ADynamicLibrary;

//static char*
//append_string( const char* str1, const char* str2 )
//{
//    int   len1   = strlen(str1);
//    int   len2   = strlen(str2);
//    char* result = malloc(len1+len2+1);
//
//    if (result != NULL) {
//        memcpy(result, str1, len1);
//        memcpy(result + len1, str2, len2);
//        result[len1+len2] = '\0';
//    }
//    return result;
//}

static ADynamicLibrary*
adynamicLibrary_open( const char*  libraryName,
                      char**       pError)
{
    void*  result;result =NULL;

   /* char*  libName = (char*) libraryName;
    if (strchr(libraryName,'.') == NULL) {
        libName = append_string(libraryName, ".so");
    }

    *pError = NULL;
    result  = dlopen( libName, RTLD_LAZY );
    if (result == NULL) {
        *pError = strdup(dlerror());
    }

    if (libName != (char*)libraryName) {
        free(libName);
    }*/

    return (ADynamicLibrary*) result;
}

static void*
adynamicLibrary_findSymbol( ADynamicLibrary*  lib,
                            const char*       symbolName,
                            char**            pError)
{
    void*  result;result =NULL;

    /**pError = NULL;

    if (lib == NULL) {
        *pError = strdup("NULL library pointer");
        return NULL;
    }
    if (symbolName == NULL || symbolName[0] == '\0') {
        *pError = strdup("NULL or empty symbolName");
        return NULL;
    }
    result = dlsym(lib, symbolName);
    if (result == NULL) {
        *pError = strdup(dlerror());
    }*/
    return result;
}

static void
adynamicLibrary_close( ADynamicLibrary*  lib )
{
    if (lib != NULL) {
       // dlclose(lib);
    }
}

int  android_gles_fast_pipes = 1;

#define STREAM_MODE_DEFAULT   0
#define STREAM_MODE_TCP       1
#define STREAM_MODE_UNIX      2
#define STREAM_MODE_PIPE      3

#define RENDERER_FUNCTIONS_LIST \
  FUNCTION_(int, initLibrary, (void), ()) \
  FUNCTION_(int, setStreamMode, (int mode), (mode)) \
  FUNCTION_(int, initOpenGLRenderer, (int width, int height, char* addr, size_t addrLen), (width, height, addr, addrLen)) \
  FUNCTION_VOID_(getHardwareStrings, (const char** vendors, const char** renderer, const char** version), (vendors, renderer, version)) \
  FUNCTION_VOID_(setPostCallback, (OnPostFunc onPost, void* onPostContext), (onPost, onPostContext)) \
  FUNCTION_(int, createOpenGLSubwindow, (FBNativeWindowType window, int x, int y, int width, int height, float zRot), (window, x, y, width, height, zRot)) \
  FUNCTION_(int, destroyOpenGLSubwindow, (void), ()) \
  FUNCTION_VOID_(setOpenGLDisplayRotation, (float zRot), (zRot)) \
  FUNCTION_VOID_(repaintOpenGLDisplay, (void), ()) \
  FUNCTION_(int, stopOpenGLRenderer, (void), ()) \

#define RENDERER_LIB_NAME  "lib64OpenglRender"

#define FUNCTION_(ret, name, sig, params) \
        static ret (*name) sig = NULL;
#define FUNCTION_VOID_(name, sig, params) \
        static void (*name) sig = NULL;
RENDERER_FUNCTIONS_LIST
#undef FUNCTION_
#undef FUNCTION_VOID_

static int
initOpenglesEmulationFuncs(ADynamicLibrary* rendererLib)
{
    void*  symbol;
    char*  error;

#define FUNCTION_(ret, name, sig, params) \
    symbol = adynamicLibrary_findSymbol(rendererLib, #name, &error); \
    if (symbol != NULL) { \
        name = symbol; \
    } else { \
        printf("GLES emulation: Could not find required symbol (%s): %s", #name, error); \
        free(error); \
        return -1; \
    }
#define FUNCTION_VOID_(name, sig, params) FUNCTION_(void, name, sig, params)
RENDERER_FUNCTIONS_LIST
#undef FUNCTION_VOID_
#undef FUNCTION_

    return 0;
}

static ADynamicLibrary*  rendererLib;
static int               rendererStarted;
static char              rendererAddress[256]="/data/qemu-gles";

int
android_initOpenglesEmulation(void)
{
    char* error = NULL;

    if (rendererLib != NULL)
        return 0;

    printf("Initializing hardware OpenGLES emulation support");

    rendererLib = adynamicLibrary_open(RENDERER_LIB_NAME, &error);
    if (rendererLib == NULL) {
        printf("Could not load OpenGLES emulation library: %s", error);
        return -1;
    }

    android_init_opengles_pipes();

    /* Resolve the functions */
    if (initOpenglesEmulationFuncs(rendererLib) < 0) {
        printf("OpenGLES emulation library mismatch. Be sure to use the correct version!");
        goto BAD_EXIT;
    }

    if (!initLibrary()) {
        printf("OpenGLES initialization failed!");
        goto BAD_EXIT;
    }

    if (android_gles_fast_pipes) {
        setStreamMode(STREAM_MODE_UNIX);
    } else {
	    setStreamMode(STREAM_MODE_TCP);
    }
    return 0;

BAD_EXIT:
    printf("OpenGLES emulation library could not be initialized!");
    adynamicLibrary_close(rendererLib);
    rendererLib = NULL;
    return -1;
}

int
android_startOpenglesRenderer(int width, int height)
{
    if (!rendererLib) {
        printf("Can't start OpenGLES renderer without support libraries");
        return -1;
    }

    if (rendererStarted) {
        return 0;
    }

    if (!initOpenGLRenderer(width, height, rendererAddress, sizeof(rendererAddress))) {
        printf("Can't start OpenGLES renderer?");
        return -1;
    }

    rendererStarted = 1;
    return 0;
}

void
android_setPostCallback(OnPostFunc onPost, void* onPostContext)
{
    if (rendererLib) {
        setPostCallback(onPost, onPostContext);
    }
}

static void strncpy_safe(char* dst, const char* src, size_t n)
{
    strncpy(dst, src, n);
    dst[n-1] = '\0';
}

static void extractBaseString(char* dst, const char* src, size_t dstSize)
{
    const char* begin = strchr(src, '(');
    const char* end = strrchr(src, ')');

    if (!begin || !end) {
        strncpy_safe(dst, src, dstSize);
        return;
    }
    begin += 1;

    if (end - begin + 1 > dstSize) {
        end = begin + dstSize - 1;
    }

    strncpy_safe(dst, begin, end - begin + 1);
}

void
android_getOpenglesHardwareStrings(char* vendor, size_t vendorBufSize,
                                   char* renderer, size_t rendererBufSize,
                                   char* version, size_t versionBufSize)
{
    const char *vendorSrc, *rendererSrc, *versionSrc;

    assert(vendorBufSize > 0 && rendererBufSize > 0 && versionBufSize > 0);
    assert(vendor != NULL && renderer != NULL && version != NULL);

    if (!rendererStarted) {
        printf("Can't get OpenGL ES hardware strings when renderer not started");
        vendor[0] = renderer[0] = version[0] = '\0';
        return;
    }

    getHardwareStrings(&vendorSrc, &rendererSrc, &versionSrc);
    if (!vendorSrc) vendorSrc = "";
    if (!rendererSrc) rendererSrc = "";
    if (!versionSrc) versionSrc = "";

    if (strncmp(vendorSrc, "Google", 6) == 0 &&
            strncmp(rendererSrc, "Android Emulator OpenGL ES Translator", 37) == 0) {
        extractBaseString(vendor, vendorSrc, vendorBufSize);
        extractBaseString(renderer, rendererSrc, rendererBufSize);
        extractBaseString(version, versionSrc, versionBufSize);
    } else {
        strncpy_safe(vendor, vendorSrc, vendorBufSize);
        strncpy_safe(renderer, rendererSrc, rendererBufSize);
        strncpy_safe(version, versionSrc, versionBufSize);
    }
}

void
android_stopOpenglesRenderer(void)
{
    if (rendererStarted) {
        stopOpenGLRenderer();
        rendererStarted = 0;
    }
}

int
android_showOpenglesWindow(void* window, int x, int y, int width, int height, float rotation)
{
    if (rendererStarted) {
        int success = createOpenGLSubwindow((FBNativeWindowType)(uintptr_t)window, x, y, width, height, rotation);
        return success ? 0 : -1;
    } else {
        return -1;
    }
}

int
android_hideOpenglesWindow(void)
{
    if (rendererStarted) {
        int success = destroyOpenGLSubwindow();
        return success ? 0 : -1;
    } else {
        return -1;
    }
}

void
android_redrawOpenglesWindow(void)
{
    if (rendererStarted) {
        repaintOpenGLDisplay();
    }
}

void
android_gles_server_path(char* buff, size_t buffsize)
{
    strncpy_safe(buff, rendererAddress, buffsize);
}

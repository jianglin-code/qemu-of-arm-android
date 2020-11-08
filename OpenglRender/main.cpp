#include "render_api.h"
#include <stdio.h>
#include <unistd.h>

static char              rendererAddress[256];

int main(int argc, char** argv)
{
    initLibrary();

    initOpenGLRenderer(1080,1920,rendererAddress,sizeof(rendererAddress));
    createOpenGLSubwindow(NULL,0,0,1080,1920,0);

    printf("initOpenGLRenderer:%s \n",rendererAddress);

    while(1) {
        sleep(60*60*24);
    }
    return 0;
}

#include <linux/unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>  
#include "virtio-msg.h"
#include "fb_backend.h"
#include <errno.h>
#define DEBUG_LOG

#ifdef DEBUG_LOG
#define LOG printf
#else
#define LOG
#endif

int fb_open(void)
{
    int fb = -1;

    LOG("fb_open...\n");
    fb = open("/dev/graphics/fb0", 2);  
    if (0 == fb)
    {
        printf("open /dev/graphics/fb0 fail!\n");
        return -1;
    }
   
	printf("[fb_open] fd=%d\n", fb);
    return fb;
}

int fb_close(MSG_FUNC_INFO *pInfo)
{
    return close(pInfo->fd);
}


int ioctl_get_fsinfo(int fd, unsigned int cmd,  MSG_FUNC_INFO *pinfo)
{
    int ret = 0;
    
    ret = ioctl(fd, cmd, &pinfo->fix);
    if (ret == -1)
    {
        printf("[ioctl_get_fsinfo] call ioctl fail! fd[%d] cmd[0x%x] err[%s]\n", fd, cmd, strerror(errno));
        return -1;
    }
    printf("[ioctl_get_fsinfo] fix->line_length[%d]\n", pinfo->fix.line_length);
    return 0;
}

int ioctl_get_vsinfo(int fd, unsigned int cmd, MSG_FUNC_INFO *pinfo)
{
    int ret = 0;

    ret = ioctl(fd, cmd, &pinfo->var);
    if (ret == -1)
    {
        printf("[ioctl_get_vsinfo] call ioctl fail!\n");
        return -1;
    }

    printf("[ioctl_get_vsinfo] end\n");
    return 0;
}

extern void *g_fbaddr;
int ioctl_get_put_vsinfo(int fd, unsigned int cmd, MSG_FUNC_INFO *pinfo)
{
    int ret = 0;
    unsigned int idx = pinfo->var.yoffset;
//    unsigned int *pTmp =  (unsigned int *)g_fbaddr;

    idx = idx * 4352 / 4;
    ret = ioctl(fd, cmd, &pinfo->var);
    if (ret == -1)
    {
        printf("[ioctl_get_put_vsinfo] call ioctl fail! fd[%d] cmd[0x%x] errno[%s] yoff[%d]\n", fd, cmd, strerror(errno), pinfo->var.yoffset);
        return -1;
    }

//    printf("[ioctl_get_put_vsinfo] yoffset[%d] magic[0x%x] off[%d]\n", pinfo->var.yoffset, pTmp[idx], idx);
    return 0;
}

int fb_ioctl(MSG_FUNC_INFO *pinfo)
{
    int fd = pinfo->fd;
    unsigned long cmd = pinfo->cmd;

    switch(cmd)
    {
        case FBIOGET_FSCREENINFO :
            return ioctl_get_fsinfo(fd, cmd, pinfo);
        case FBIOGET_VSCREENINFO :
            return ioctl_get_vsinfo(fd, cmd, pinfo);
        case FBIOPAN_DISPLAY :
        case FBIOPUT_VSCREENINFO :
            /* 需要先获取传入的vinfo，然后写入的ioctl */
            return ioctl_get_put_vsinfo(fd, cmd, pinfo);
            break; 
        default :
            printf("[fb_ioctl] unsupport ioctl!\n");
            return -1;    
    }
    
    return 0;
}

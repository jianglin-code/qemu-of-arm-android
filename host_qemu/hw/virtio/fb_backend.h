
#ifndef _FB_BACKEND_H_
#define _FB_BACKEND_H_

extern int fb_open(void);
extern int fb_close(MSG_FUNC_INFO *pInfo);
extern int ioctl_get_fsinfo(int fd, unsigned int cmd,  MSG_FUNC_INFO *pinfo);
extern int ioctl_get_vsinfo(int fd, unsigned int cmd, MSG_FUNC_INFO *pinfo);
extern int ioctl_get_put_vsinfo(int fd, unsigned int cmd, MSG_FUNC_INFO *pinfo);
extern int fb_ioctl(MSG_FUNC_INFO *pinfo);
    
#endif


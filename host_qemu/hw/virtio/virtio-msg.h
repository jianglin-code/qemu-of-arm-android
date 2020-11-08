/*
 * =====================================================================================
 *
 *       Filename:  virtio_msg.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015Äê11ÔÂ2ÈÕ
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  GuoWeibin, gwb@xdja.com
 *        Company:  xdja
 *
 * =====================================================================================
 */
#ifndef _VIRTIO_MSG_H_
#define _VIRTIO_MSG_H_
#include <linux/fb.h>
#include "hw/virtio/virtio.h"
#include "sysemu/iothread.h"


#define TYPE_VIRTIO_MSG "virtio-msg-device"
#define VIRTIO_MSG(obj) \
        OBJECT_CHECK(VirtIOMsg, (obj), TYPE_VIRTIO_MSG)

enum msgrq_cmd_type_bits {
    MSG_REQ_TYPE_FUNC         = 1,
};

enum msgrq_func_idx {
    MSG_FUNC_FB_OPEN = 1,
    MSG_FUNC_FB_IOCTL = 2,
    MSG_FUNC_FB_CLOSE = 3,
};

/* And this is the final byte of the write scatter-gather list. */
#define VIRTIO_MSG_S_OK		0
#define VIRTIO_MSG_S_IOERR	    1
#define VIRTIO_MSG_S_UNSUPP	2

/* Feature bits */
#define VIRTIO_MSG_F_SIZE_MAX	0	/* Indicates maximum segment size */
#define VIRTIO_MSG_F_SEG_MAX	1	/* Indicates maximum # of segments */
#define MSG_FUNC_PARA_NUM   (4)

typedef struct VirtIOMsgConf
{
    IOThread *iothread;
}VirtIOMsgConf;

struct fb_cmap_user {
	__u32 start;			/* First entry	*/
	__u32 len;			/* Number of entries */
	__u16  *red;		/* Red values	*/
	__u16  *green;
	__u16  *blue;
	__u16  *transp;		/* transparency, can be NULL */
};

typedef struct _MSG_FUNC_INFO {
    int flags;
    unsigned char status;
    int ret;
    int cmd_type;
    int func_idx;
    int fd;
    int cmd;
    union {
        struct fb_ops *fb;
        struct fb_fix_screeninfo fix;
        struct fb_var_screeninfo var;
        struct fb_con2fbmap con2fb;
        struct fb_cmap cmap_from;
        struct fb_cmap_user cmap;
    };
}MSG_FUNC_INFO, *PMSG_FUNC_INFO;


struct MsgReq
{
    struct VirtIOMsg *vmsg;
    MSG_FUNC_INFO finfo;
};

struct VirtIOMsgReq
{
    struct VirtIOMsg *dev;
    VirtQueueElement elem; 
    size_t in_len;
};

typedef struct VirtIOMsgReq VirtIOMsgReq;

struct VirtIOMsg {
    VirtIODevice parent_obj;
    VirtQueue *vq;
    void *rq;
    QEMUBH *bh;
    VirtIOMsgConf conf;

    VMChangeStateEntry *change;
    /* Function to push to vq and notify guest */
    void (*complete_func_call)(VirtIOMsgReq *req);
    Notifier migration_state_notifier;
};

typedef struct VirtIOMsg VirtIOMsg;

#endif


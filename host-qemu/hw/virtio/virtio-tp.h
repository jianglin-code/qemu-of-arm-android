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
#ifndef _VIRTIO_TP_H_
#define _VIRTIO_TP_H_
#include "hw/virtio/virtio.h"
#include "sysemu/iothread.h"

struct VirtIOTpMsg
{
    struct VirtIOTp *dev;
    VirtQueueElement elem; 
    size_t len;
};

typedef struct VirtIOTpMsg VirtIOTpMsg;

typedef struct VirtIOTpConf
{
    IOThread *iothread;
}VirtIOTpConf;

struct VirtIOTp {
    VirtIODevice parent_obj;
    VirtQueue *vq;
    void *rq;
    QEMUBH *bh;
    VirtIOTpConf conf;

    VMChangeStateEntry *change;
    /* Function to push to vq and notify guest */
    void (*complete_func_call)(VirtIOTpMsg *msg);
    Notifier migration_state_notifier;
    QemuThread thread;
    bool queue_state;
};

typedef struct VirtIOTp VirtIOTp;

#define TYPE_VIRTIO_TP "virtio-tp-device"
#define VIRTIO_TP(obj) \
        OBJECT_CHECK(VirtIOTp, (obj), TYPE_VIRTIO_TP)
    /* Feature bits */
#define VIRTIO_TP_F_SIZE_MAX	0	/* Indicates maximum segment size */
#define VIRTIO_TP_F_SEG_MAX	1	/* Indicates maximum # of segments */


#endif


/*
 * =====================================================================================
 *
 *       Filename:  virtio_msg.c
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
#include "qemu-common.h"
#include "qemu/iov.h"
#include "qemu/error-report.h"
#include "trace.h"
#include "migration/migration.h"
#include "block/scsi.h"
#ifdef __linux__
# include <scsi/sg.h>
#endif
#include "hw/virtio/virtio-bus.h"
#include "hw/virtio/virtio-access.h"
#include "hw/virtio/virtio-tp.h"
#include <linux/input.h>     

#include "standard-headers/linux/virtio_ids.h"

//#define MSG_DEBUG
#ifdef MSG_DEBUG
static FILE *fd;
#define LOG_FILE(fmt, ...) do { \
	fprintf(fd, fmt, ##__VA_ARGS__); \
	fflush(fd); \
}while(0)

static int debug_init(void)
{
    fd = fopen("/data/virt/qemu_msglog", "w+");
    if (fd < 0)
    {
        printf("create log file fail!\n");
        return -1;
    }
	return 0;
}
#else
#define LOG_FILE printf
#endif

#define VIRTIO_TP_F_SIZE_MAX	0	/* Indicates maximum segment size */
#define VIRTIO_TP_F_SEG_MAX	1	/* Indicates maximum # of segments */

static VirtIOTpMsg *virtio_tp_get_request(VirtIOTp *s)
{
    VirtIOTpMsg *pReq = g_slice_new(VirtIOTpMsg);

    pReq->dev = s;
    if (!virtqueue_pop(s->vq, &pReq->elem)) {
        g_slice_free(VirtIOTpMsg, pReq);
        return NULL;
    }

    return pReq;
}

static void virtio_tp_req_complete(VirtIOTpMsg *req)
{
    req->dev->complete_func_call(req);
}

static void virtio_tp_handle_output(VirtIODevice *vdev, VirtQueue *vq)
{
#if 0
    VirtIOTp *s = VIRTIO_TP(vdev);
    VirtIOTpMsg *req;

    while ((req = virtio_msg_get_request(s))) {
        virtio_msg_handle_request(req);
    }
#endif
    VirtIOTp *s = VIRTIO_TP(vdev);

    s->queue_state = true;

}

static void virtio_tp_complete_request(VirtIOTpMsg *req)
{
    VirtIOTp *s = req->dev;
    VirtIODevice *vdev = VIRTIO_DEVICE(s);

    virtqueue_push(s->vq, &req->elem, req->len);
    virtio_notify(vdev, s->vq);
}

static void virtio_tp_save(QEMUFile *f, void *opaque)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(opaque);

    virtio_save(vdev, f);
}

static int virtio_tp_load(QEMUFile *f, void *opaque, int version_id)
{
    VirtIOTp *s = opaque;
    VirtIODevice *vdev = VIRTIO_DEVICE(s);

    if (version_id != 2)
        return -EINVAL;

    return virtio_load(vdev, f, version_id);
}

static void tp_dbg_show(VirtQueueElement *pElement)
{
    printf("[tp_dbg_show] index[%d] out_num[%d] in_num[%d]\n", 
        pElement->index, pElement->out_num, pElement->in_num);
}

static void tp_input_report(VirtIOTpMsg *pMsg, struct input_event *pEvent, unsigned int len)
{    
    struct iovec *iov = pMsg->elem.in_sg;

    *((unsigned int *)iov->iov_base) = len;
    memcpy(((unsigned char *)iov->iov_base + 4), pEvent, len);
    pMsg->len = iov->iov_len;
    virtio_tp_req_complete(pMsg);
}

static void* tp_poll_thread_fn(void *arg)
{
    VirtIOTp *s = (VirtIOTp *)arg;
    VirtIOTpMsg *pMsg = NULL;
    int fd = -1;
    int count = 0;
    struct input_event ev[35];   

    /* wait queue init finish */
    while(s->queue_state == false) {
        sleep(2);
    }

    fd = open("/dev/input/event2", O_RDWR);
    if (fd < 0) {
        printf("[tp_poll_thread_fn] open file fail!\n");
        return NULL;
    }

    while(1) {
        count = read(fd, &ev, sizeof(ev));
        if (0 != count % sizeof(struct input_event)) {
            printf("[tp_poll_thread_fn] read error!\n");    
        }

        pMsg = virtio_tp_get_request(s);
        //tp_dbg_show(&pMsg->elem);
        tp_input_report(pMsg, ev, count);
        
    }

    return NULL;
}


static void virtio_tp_device_realize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOTp *s = VIRTIO_TP(dev);
    static int virtio_msg_id;

    virtio_init(vdev, "virtio-tp", VIRTIO_ID_INPUT,
                sizeof(struct VirtIOTpConf));
    s->queue_state = false;
    s->rq = NULL;
    s->vq = virtio_add_queue(vdev, 1024, virtio_tp_handle_output);
    s->complete_func_call = virtio_tp_complete_request;
    s->migration_state_notifier.notify = NULL; //virtio_msg_migration_state_changed;
    add_migration_state_change_notifier(&s->migration_state_notifier);

    //s->change = qemu_add_vm_change_state_handler(virtio_blk_dma_restart_cb, s);
    register_savevm(dev, "virtio-tp", virtio_msg_id++, 2,
                    virtio_tp_save, virtio_tp_load, s);


    qemu_thread_create(&s->thread, "tp_poll", tp_poll_thread_fn,
                   s, QEMU_THREAD_JOINABLE);
}



static void virtio_tp_instance_init(Object *obj)
{
    VirtIOTp *s = VIRTIO_TP(obj);

    object_property_add_link(obj, "iothread", TYPE_IOTHREAD,
                          (Object **)&s->conf.iothread,
                          qdev_prop_allow_set_link_before_realize,
                          OBJ_PROP_LINK_UNREF_ON_RELEASE, NULL);
#if 0    
    device_add_bootindex_property(obj, &s->conf.conf.bootindex,
                               "bootindex", "/disk@0,0",
                               DEVICE(obj), NULL);
#endif
}

static void virtio_tp_device_unrealize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOTp *s = VIRTIO_TP(dev);

    remove_migration_state_change_notifier(&s->migration_state_notifier);
    qemu_del_vm_change_state_handler(s->change);
    unregister_savevm(dev, "virtio-tp", s);
    virtio_cleanup(vdev);
}

static void virtio_tp_update_config(VirtIODevice *vdev, uint8_t *config)
{
    VirtIOTp *s = VIRTIO_TP(vdev);
    struct VirtIOTpConf tp_cfg;

    printf("[virtio_msg_update_config] .....\n");


    memset(&tp_cfg, 0, sizeof(tp_cfg));
    tp_cfg.iothread = s->conf.iothread;
    memcpy(config, &tp_cfg, sizeof(struct VirtIOTpConf));
}

static void virtio_tp_set_config(VirtIODevice *vdev, const uint8_t *config)
{
    VirtIOTp *s = VIRTIO_TP(vdev);
    struct VirtIOTpConf tp_cfg;

    memcpy(&tp_cfg, config, sizeof(tp_cfg));
    s->conf.iothread = tp_cfg.iothread;
}

static uint64_t virtio_tp_get_features(VirtIODevice *vdev, uint64_t features,
                                        Error **errp)
{
    //VirtIOMsg *s = VIRTIO_MSG(vdev);

    virtio_add_feature(&features, VIRTIO_TP_F_SIZE_MAX);
    virtio_add_feature(&features, VIRTIO_TP_F_SEG_MAX);


    return features;
}

static void virtio_tp_set_status(VirtIODevice *vdev, uint8_t status)
{
    //VirtIOMsg *s = VIRTIO_MSG(vdev);

    if (!(status & VIRTIO_CONFIG_S_DRIVER_OK)) {
        return;
    }
}

static void virtio_tp_reset(VirtIODevice *vdev)
{
    VirtIOTp *s = VIRTIO_TP(vdev);
    AioContext *ctx = NULL;

    s = s;
    ctx = ctx;
}

static void virtio_tp_save_device(VirtIODevice *vdev, QEMUFile *f)
{
    VirtIOTp *s = VIRTIO_TP(vdev);
    VirtIOTpMsg *msg = s->rq;

    if (msg) {
        qemu_put_sbyte(f, 1);
        qemu_put_buffer(f, (unsigned char *)&msg->elem,
                        sizeof(VirtQueueElement));
    }
    qemu_put_sbyte(f, 0);
}


static int virtio_tp_load_device(VirtIODevice *vdev, QEMUFile *f,
                                  int version_id)
{
    VirtIOTp *s = VIRTIO_TP(vdev);

    if (qemu_get_sbyte(f)) {
        VirtIOTpMsg *msg = g_slice_new(VirtIOTpMsg);
        qemu_get_buffer(f, (unsigned char *)&msg->elem,
                        sizeof(VirtQueueElement));
        s->rq = msg;

        virtqueue_map_sg(msg->elem.in_sg, msg->elem.in_addr,
            msg->elem.in_num, 1);
        virtqueue_map_sg(msg->elem.out_sg, msg->elem.out_addr,
            msg->elem.out_num, 0);
    }

    return 0;
}


static Property virtio_tp_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

static void virtio_tp_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(klass);
#ifdef MSG_DEBUG
		debug_init();
#endif
    dc->props = virtio_tp_properties;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    vdc->realize = virtio_tp_device_realize;
    vdc->unrealize = virtio_tp_device_unrealize;
    vdc->get_config = virtio_tp_update_config;
    vdc->set_config = virtio_tp_set_config;
    vdc->get_features = virtio_tp_get_features;
    vdc->set_status = virtio_tp_set_status;
    vdc->reset = virtio_tp_reset;
    vdc->save = virtio_tp_save_device;
    vdc->load = virtio_tp_load_device;
}
  
static const TypeInfo virtio_tp_info = {
    .name = TYPE_VIRTIO_TP,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOTp),
    .instance_init = virtio_tp_instance_init,
    .class_init = virtio_tp_class_init,
};
 
static void virtiotp_register_types(void)
{
    type_register_static(&virtio_tp_info);
}

type_init(virtiotp_register_types)




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
#include "hw/virtio/virtio-msg.h"
#include "hw/virtio/fb_backend.h"
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

static VirtIOMsgReq *virtio_msg_get_request(VirtIOMsg *s)
{
    VirtIOMsgReq *pReq = g_slice_new(VirtIOMsgReq);

    pReq->dev = s;
    if (!virtqueue_pop(s->vq, &pReq->elem)) {
        g_slice_free(VirtIOMsgReq, pReq);
        return NULL;
    }

    return pReq;
}

static void virtio_msg_req_complete(VirtIOMsgReq *req)
{
    req->dev->complete_func_call(req);
}

static int virtio_msg_func_dispatch(struct MsgReq *pMsgReq)
{
    MSG_FUNC_INFO *pInfo = &pMsgReq->finfo;
    int func_idx = pInfo->func_idx;

    switch (func_idx)
    {
        case MSG_FUNC_FB_OPEN :
            pInfo->fd = pInfo->ret = fb_open();
            break;
        case MSG_FUNC_FB_IOCTL :
            pInfo->ret = fb_ioctl(pInfo);
            break;
        case MSG_FUNC_FB_CLOSE :
            pInfo->ret = fb_close(pInfo);
            break;  
        default :
            printf("[virtio_msg_func_dispatch] unsupport func[%d]\n", func_idx);
            return -1;
    }

    return 0;
}

static void virtio_msg_handle_request(VirtIOMsgReq *req)
{
    uint32_t type;
    struct iovec *in_iov = req->elem.in_sg;
    struct iovec *iov = req->elem.out_sg;
    unsigned in_num = req->elem.in_num;
    unsigned out_num = req->elem.out_num;
    struct MsgReq *pMsgReq = (struct MsgReq *)(iov->iov_base);

    in_num = in_num;
    in_iov = in_iov;
    req->in_len = iov_size(iov, out_num);
    type = virtio_ldl_p(VIRTIO_DEVICE(req->dev), &(pMsgReq->finfo.cmd_type));
    if (type != MSG_REQ_TYPE_FUNC)
    {
        LOG_FILE("[virtio_msg_handle_request] type[%d] error\n", type);
        goto END;
    }
    virtio_msg_func_dispatch(pMsgReq);
    pMsgReq->finfo.status = VIRTIO_MSG_S_OK;
    
END:
    virtio_msg_req_complete(req);
    g_slice_free(VirtIOMsgReq, req);
}


static void virtio_msg_handle_output(VirtIODevice *vdev, VirtQueue *vq)
{
    VirtIOMsg *s = VIRTIO_MSG(vdev);
    VirtIOMsgReq *req;

    while ((req = virtio_msg_get_request(s))) {
        virtio_msg_handle_request(req);
    }
}

static void virtio_msg_complete_request(VirtIOMsgReq *req)
{
    VirtIOMsg *s = req->dev;
    VirtIODevice *vdev = VIRTIO_DEVICE(s);

    virtqueue_push(s->vq, &req->elem, req->in_len);
    virtio_notify(vdev, s->vq);
}

static void virtio_msg_save(QEMUFile *f, void *opaque)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(opaque);

    virtio_save(vdev, f);
}

static int virtio_msg_load(QEMUFile *f, void *opaque, int version_id)
{
    VirtIOMsg *s = opaque;
    VirtIODevice *vdev = VIRTIO_DEVICE(s);

    if (version_id != 2)
        return -EINVAL;

    return virtio_load(vdev, f, version_id);
}

static void virtio_msg_device_realize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOMsg *s = VIRTIO_MSG(dev);
    static int virtio_msg_id;

    virtio_init(vdev, "virtio-msg", VIRTIO_ID_MSG,
                sizeof(struct VirtIOMsgConf));

    s->rq = NULL;
    s->vq = virtio_add_queue(vdev, 128, virtio_msg_handle_output);
    s->complete_func_call = virtio_msg_complete_request;
    s->migration_state_notifier.notify = NULL; //virtio_msg_migration_state_changed;
    add_migration_state_change_notifier(&s->migration_state_notifier);

    //s->change = qemu_add_vm_change_state_handler(virtio_blk_dma_restart_cb, s);
    register_savevm(dev, "virtio-msg", virtio_msg_id++, 2,
                    virtio_msg_save, virtio_msg_load, s);
}



static void virtio_msg_instance_init(Object *obj)
{
    VirtIOMsg *s = VIRTIO_MSG(obj);

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

static void virtio_msg_device_unrealize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOMsg *s = VIRTIO_MSG(dev);

    remove_migration_state_change_notifier(&s->migration_state_notifier);
    qemu_del_vm_change_state_handler(s->change);
    unregister_savevm(dev, "virtio-msg", s);
    virtio_cleanup(vdev);
}

static void virtio_msg_update_config(VirtIODevice *vdev, uint8_t *config)
{
    VirtIOMsg *s = VIRTIO_MSG(vdev);
    struct VirtIOMsgConf msgcfg;

    printf("[virtio_msg_update_config] .....\n");


    memset(&msgcfg, 0, sizeof(msgcfg));
    msgcfg.iothread = s->conf.iothread;
    memcpy(config, &msgcfg, sizeof(struct VirtIOMsgConf));
}

static void virtio_msg_set_config(VirtIODevice *vdev, const uint8_t *config)
{
    VirtIOMsg *s = VIRTIO_MSG(vdev);
    struct VirtIOMsgConf msgcfg;

    memcpy(&msgcfg, config, sizeof(msgcfg));
    s->conf.iothread = msgcfg.iothread;
}

static uint64_t virtio_msg_get_features(VirtIODevice *vdev, uint64_t features,
                                        Error **errp)
{
    //VirtIOMsg *s = VIRTIO_MSG(vdev);

    virtio_add_feature(&features, VIRTIO_MSG_F_SIZE_MAX);
    virtio_add_feature(&features, VIRTIO_MSG_F_SEG_MAX);

    return features;
}

static void virtio_msg_set_status(VirtIODevice *vdev, uint8_t status)
{
    //VirtIOMsg *s = VIRTIO_MSG(vdev);

    if (!(status & VIRTIO_CONFIG_S_DRIVER_OK)) {
        return;
    }
}

static void virtio_msg_reset(VirtIODevice *vdev)
{
    VirtIOMsg *s = VIRTIO_MSG(vdev);
    AioContext *ctx = NULL;

    s = s;
    ctx = ctx;
}

static void virtio_msg_save_device(VirtIODevice *vdev, QEMUFile *f)
{
    VirtIOMsg *s = VIRTIO_MSG(vdev);
    VirtIOMsgReq *req = s->rq;

    if (req) {
        qemu_put_sbyte(f, 1);
        qemu_put_buffer(f, (unsigned char *)&req->elem,
                        sizeof(VirtQueueElement));
    }
    qemu_put_sbyte(f, 0);
}


static int virtio_msg_load_device(VirtIODevice *vdev, QEMUFile *f,
                                  int version_id)
{
    VirtIOMsg *s = VIRTIO_MSG(vdev);

    if (qemu_get_sbyte(f)) {
        VirtIOMsgReq *req = g_slice_new(VirtIOMsgReq);
        qemu_get_buffer(f, (unsigned char *)&req->elem,
                        sizeof(VirtQueueElement));
        s->rq = req;

        virtqueue_map_sg(req->elem.in_sg, req->elem.in_addr,
            req->elem.in_num, 1);
        virtqueue_map_sg(req->elem.out_sg, req->elem.out_addr,
            req->elem.out_num, 0);
    }

    return 0;
}


static Property virtio_msg_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

static void virtio_msg_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(klass);
#ifdef MSG_DEBUG
		debug_init();
#endif
    dc->props = virtio_msg_properties;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    vdc->realize = virtio_msg_device_realize;
    vdc->unrealize = virtio_msg_device_unrealize;
    vdc->get_config = virtio_msg_update_config;
    vdc->set_config = virtio_msg_set_config;
    vdc->get_features = virtio_msg_get_features;
    vdc->set_status = virtio_msg_set_status;
    vdc->reset = virtio_msg_reset;
    vdc->save = virtio_msg_save_device;
    vdc->load = virtio_msg_load_device;
}
  
static const TypeInfo virtio_msg_info = {
    .name = TYPE_VIRTIO_MSG,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOMsg),
    .instance_init = virtio_msg_instance_init,
    .class_init = virtio_msg_class_init,
};
 
static void virtiomsg_register_types(void)
{
    type_register_static(&virtio_msg_info);
}

type_init(virtiomsg_register_types)



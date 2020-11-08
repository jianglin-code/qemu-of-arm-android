/*
 * Virtio MMIO Bindings
 *
 * Copyright XDJA, Corp. 2015
 * Copyright (c) 2015 CodeSourcery
 *
 * Authors:
 *  Anthony GuoWeibin   <gwb@xdja.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#ifndef QEMU_VIRTIO_MMIO_H
#define QEMU_VIRTIO_MMIO_H

#include "hw/virtio/virtio-bus.h"
#include "hw/virtio/virtio-blk.h"
#include "hw/virtio/virtio-msg.h"
//#include "hw/virtio/virtio-net.h"
//#include "hw/virtio/virtio-rng.h"
//#include "hw/virtio/virtio-serial.h"
//#include "hw/virtio/virtio-scsi.h"
//#include "hw/virtio/virtio-balloon.h"
//#include "hw/virtio/virtio-bus.h"
//#include "hw/virtio/virtio-9p.h"
//#include "hw/virtio/virtio-input.h"
//#include "hw/virtio/virtio-gpu.h"
#ifdef CONFIG_VIRTFS
#include "hw/9pfs/virtio-9p.h"
#endif
#ifdef CONFIG_VHOST_SCSI
#include "hw/virtio/vhost-scsi.h"
#endif

/* QOM macros */
/* virtio-mmio-bus */
#define TYPE_VIRTIO_MMIO_BUS "virtio-mmio-bus"
#define VIRTIO_MMIO_BUS(obj) \
        OBJECT_CHECK(VirtioBusState, (obj), TYPE_VIRTIO_MMIO_BUS)
#define VIRTIO_MMIO_BUS_GET_CLASS(obj) \
        OBJECT_GET_CLASS(VirtioBusClass, (obj), TYPE_VIRTIO_MMIO_BUS)
#define VIRTIO_MMIO_BUS_CLASS(klass) \
        OBJECT_CLASS_CHECK(VirtioBusClass, (klass), TYPE_VIRTIO_MMIO_BUS)

/* virtio-mmio */
#define TYPE_VIRTIO_MMIO "virtio-mmio"
#define VIRTIO_MMIO(obj) \
        OBJECT_CHECK(VirtIOMMIOProxy, (obj), TYPE_VIRTIO_MMIO)

/* Memory mapped register offsets */
#define VIRTIO_MMIO_MAGIC 0x0
#define VIRTIO_MMIO_VERSION 0x4
#define VIRTIO_MMIO_DEVICEID 0x8
#define VIRTIO_MMIO_VENDORID 0xc
#define VIRTIO_MMIO_HOSTFEATURES 0x10
#define VIRTIO_MMIO_HOSTFEATURESSEL 0x14
#define VIRTIO_MMIO_GUESTFEATURES 0x20
#define VIRTIO_MMIO_GUESTFEATURESSEL 0x24
#define VIRTIO_MMIO_GUESTPAGESIZE 0x28
#define VIRTIO_MMIO_QUEUESEL 0x30
#define VIRTIO_MMIO_QUEUENUMMAX 0x34
#define VIRTIO_MMIO_QUEUENUM 0x38
#define VIRTIO_MMIO_QUEUEALIGN 0x3c
#define VIRTIO_MMIO_QUEUEPFN 0x40
#define VIRTIO_MMIO_QUEUENOTIFY 0x50
#define VIRTIO_MMIO_INTERRUPTSTATUS 0x60
#define VIRTIO_MMIO_INTERRUPTACK 0x64
#define VIRTIO_MMIO_STATUS 0x70
/* Device specific config space starts here */
#define VIRTIO_MMIO_CONFIG 0x100

#define VIRT_MAGIC 0x74726976 /* 'virt' */
#define VIRT_VERSION 1
#define VIRT_VENDOR 0x554D4551 /* 'QEMU' */

#define TYPE_VIRTIO_BLK_MMIO "virtio-blk-mmio"
#define TYPE_VIRTIO_MSG_MMIO	"virtio-msg-mmio"

typedef struct {
    /* Generic */
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    qemu_irq irq;
    /* Guest accessible state needing migration and reset */
    uint32_t host_features_sel;
    uint32_t guest_features_sel;
    uint32_t guest_page_shift;
    /* virtio-bus */
    VirtioBusState bus;
    bool ioeventfd_disabled;
    bool ioeventfd_started;
} VirtIOMMIOProxy;


struct VirtIOBlkMMIO {
    VirtIOMMIOProxy parent_obj;
    VirtIOBlock vdev;
};

typedef struct VirtIOBlkMMIO VirtIOBlkMMIO;  
 
#define VIRTIO_BLK_MMIO(obj) \
        OBJECT_CHECK(VirtIOBlkMMIO, (obj), TYPE_VIRTIO_BLK_MMIO)



struct VirtIOMsgMMIO {
    VirtIOMMIOProxy parent_obj;
    VirtIOMsg vdev;
};

typedef struct VirtIOMsgMMIO VirtIOMsgMMIO;

#define VIRTIO_MSG_MMIO(obj) \
        OBJECT_CHECK(VirtIOMsgMMIO, (obj), TYPE_VIRTIO_MSG_MMIO)

#endif

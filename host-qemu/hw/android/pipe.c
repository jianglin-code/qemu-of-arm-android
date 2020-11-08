#include "hw/sysbus.h"
#include "exec/ram_addr.h"
#include "hw/android/pipe.h"
#include "qemu/timer.h"
#include "exec/address-spaces.h"
#include <sys/time.h>
#include <unistd.h>
/****************************************************************************/

static inline void uint64_set_low(uint64_t *addr, uint32 value)
{
    *addr = (*addr & ~(0xFFFFFFFFULL)) | value;
}

static inline void uint64_set_high(uint64_t *addr, uint32 value)
{
    *addr = (*addr & 0xFFFFFFFFULL) | ((uint64_t)value << 32);
}

static int qemu_guest_is_64bit(void)
{
    int result = 0;
    return result;
}

//static hwaddr safe_get_phys_page_debug(CPUState *cpu, target_ulong addr)
//{
//     CPUArchState *env = cpu->env_ptr;

// //#ifdef TARGET_I386
//     if (kvm_enabled()) {
//        kvm_get_sregs(cpu);
//     }
// //#endif
//     return cpu_get_phys_page_debug(cpu, addr);
//}

/****************************************************************************/

#define DEBUG 0

#define DEBUG_REGS  0

#if DEBUG >= 1
#  define D(...)  fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
#else
#  define D(...)  (void)0
#endif

#if DEBUG >= 2
#  define DD(...)  fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
#else
#  define DD(...)  (void)0
#endif

#if DEBUG_REGS >= 1
#  define DR(...)   D(__VA_ARGS__)
#else
#  define DR(...)   (void)0
#endif

#define E(...)  fprintf(stderr, "ERROR:" __VA_ARGS__), fprintf(stderr, "\n")

/*****************************************************************************/

#define TYPE_QEMU_PIPE "qemu_pipe"
#define QEMU_PIPE(obj) OBJECT_CHECK(PipeDevice, (obj), TYPE_QEMU_PIPE)
#define MAX_PIPE_SERVICE_NAME_SIZE  255
#define MAX_PIPE_SERVICES  8

typedef struct {
    const char*        name;
    void*              opaque;
    GoldfishPipeFuncs  funcs;
} PipeService;

typedef struct {
    int          count;
    PipeService  services[MAX_PIPE_SERVICES];
} PipeServices;

static PipeServices  _pipeServices[1];

typedef struct PipeDevice  PipeDevice;

typedef struct Pipe {
    struct Pipe*               next;
    struct Pipe*               next_waked;
    PipeDevice*                device;
    uint64_t                   channel;
    void*                      opaque;
    const GoldfishPipeFuncs*   funcs;
    const PipeService*         service;
    char*                      args;
    unsigned char              wanted;
    char                       closed;
} Pipe;

typedef struct PipeDevice {
    SysBusDevice               parent_obj;
    MemoryRegion               iomem;
    qemu_irq                   irq;

    Pipe*                      pipes;
    Pipe*                      signaled_pipes;
    uint64_t                   address;
    uint32_t                   size;
    uint32_t                   status;
    uint64_t                   channel;
    uint32_t                   wakes;
    uint64_t                   params_addr;
} PipeDevice;

/***********************************************************************/

void qemu_pipe_add_type(const char*               pipeName,
                       void*                     pipeOpaque,
                       const GoldfishPipeFuncs*  pipeFuncs )
{
    PipeServices* list = _pipeServices;
    int           count = list->count;

    if (count >= MAX_PIPE_SERVICES) {
        return;
    }

    if (strlen(pipeName) > MAX_PIPE_SERVICE_NAME_SIZE) {
        return;
    }

    list->services[count].name   = pipeName;
    list->services[count].opaque = pipeOpaque;
    list->services[count].funcs  = pipeFuncs[0];

    list->count++;
}

static const PipeService* qemu_pipe_find_type(const char*  pipeName)
{
    PipeServices* list = _pipeServices;
    int           count = list->count;
    int           nn;

    for (nn = 0; nn < count; nn++) {
        if (!strcmp(list->services[nn].name, pipeName)) {
            return &list->services[nn];
        }
    }
    return NULL;
}

static void*  pipeConnector_new(Pipe*  pipe);

static Pipe*
pipe_new0(PipeDevice* dev)
{
    Pipe*  pipe;
    pipe=g_malloc0(sizeof(*pipe));
    pipe->device = dev;
    return pipe;
}

static Pipe*
pipe_new(uint64_t channel, PipeDevice* dev)
{
    Pipe*  pipe   = pipe_new0(dev);
    pipe->channel = channel;
    pipe->opaque  = pipeConnector_new(pipe);
    return pipe;
}

static Pipe**
pipe_list_findp_channel( Pipe** list, uint64_t channel )
{
    Pipe** pnode = list;
    for (;;) {
        Pipe* node = *pnode;
        if (node == NULL || node->channel == channel) {
            break;
        }
        pnode = &node->next;
    }
    return pnode;
}

static Pipe**
pipe_list_findp_waked( Pipe** list, Pipe* pipe )
{
    Pipe** pnode = list;
    for (;;) {
        Pipe* node = *pnode;
        if (node == NULL || node == pipe) {
            break;
        }
        pnode = &node->next_waked;
    }
    return pnode;
}


static void
pipe_list_remove_waked( Pipe** list, Pipe*  pipe )
{
    Pipe** lookup = pipe_list_findp_waked(list, pipe);
    Pipe*  node   = *lookup;

    if (node != NULL) {
        (*lookup) = node->next_waked;
        node->next_waked = NULL;
    }
}

static void
pipe_free( Pipe* pipe )
{
    if (pipe->funcs->close) {
        pipe->funcs->close(pipe->opaque);
    }

    g_free(pipe->args);
    g_free(pipe);
}

void
qemu_pipe_wake( void* hwpipe, unsigned flags )
{
    Pipe*  pipe = hwpipe;
    Pipe** lookup = NULL;
    PipeDevice*  dev = pipe->device;

    DD("%s: channel=0x%llx flags=%d", __FUNCTION__, (unsigned long long)pipe->channel, flags);

    /* If not already there, add to the list of signaled pipes */
    lookup = pipe_list_findp_waked(&dev->signaled_pipes, pipe);
    if (!*lookup) {
        pipe->next_waked = dev->signaled_pipes;
        dev->signaled_pipes = pipe;
    }
    pipe->wanted |= (unsigned)flags;

    /* Raise IRQ to indicate there are items on our list ! */
    //goldfish_device_set_irq(&dev->dev, 0, 1);
    qemu_set_irq(dev->irq,1);
    DD("%s: raising IRQ", __FUNCTION__);
}

void
qemu_pipe_close( void* hwpipe )
{
    Pipe* pipe = hwpipe;

    D("%s: channel=0x%llx (closed=%d)", __FUNCTION__, (unsigned long long)pipe->channel, pipe->closed);

    if (!pipe->closed) {
        pipe->closed = 1;
        qemu_pipe_wake( hwpipe, PIPE_WAKE_CLOSED );
    }
}


typedef struct {
    Pipe*  pipe;
    char   buffer[128];
    int    buffpos;
} PipeConnector;

static const GoldfishPipeFuncs  pipeConnector_funcs;

void*
pipeConnector_new(Pipe*  pipe)
{
    PipeConnector*  pcon;

    pcon=g_malloc0(sizeof(*pcon));
    pcon->pipe  = pipe;
    pipe->funcs = &pipeConnector_funcs;
    return pcon;
}

static void
pipeConnector_close( void* opaque )
{
    PipeConnector*  pcon = opaque;
    g_free(pcon);
}

static int
pipeConnector_sendBuffers( void* opaque, const GoldfishPipeBuffer* buffers, int numBuffers )
{
    PipeConnector* pcon = opaque;
    const GoldfishPipeBuffer*  buffers_limit = buffers + numBuffers;
    int ret = 0;

    DD("%s: channel=0x%llx numBuffers=%d", __FUNCTION__,
       (unsigned long long)pcon->pipe->channel,
       numBuffers);

    while (buffers < buffers_limit) {
        int  avail;

        DD("%s: buffer data (%3d bytes): '%.*s'", __FUNCTION__,
           (int)buffers[0].size, (int)buffers[0].size, buffers[0].data);

        if (buffers[0].size == 0) {
            buffers++;
            continue;
        }

        avail = sizeof(pcon->buffer) - pcon->buffpos;
        if (avail > buffers[0].size)
            avail = buffers[0].size;

        if (avail > 0) {
            memcpy(pcon->buffer + pcon->buffpos, buffers[0].data, avail);
            pcon->buffpos += avail;
            ret += avail;
        }
        buffers++;
    }

    if (memchr(pcon->buffer, '\0', pcon->buffpos) != NULL) {
        char* pipeName;
        char* pipeArgs;

        D("%s: connector: '%s' \n ", __FUNCTION__, pcon->buffer);

        if (memcmp(pcon->buffer, "pipe:", 5) != 0) {
            D("%s: Unknown pipe connection: '%s'", __FUNCTION__, pcon->buffer);
            return PIPE_ERROR_INVAL;
        }

        pipeName = pcon->buffer + 5;
        pipeArgs = strchr(pipeName, ':');

        if (pipeArgs != NULL) {
            *pipeArgs++ = '\0';
            if (!*pipeArgs)
                pipeArgs = NULL;
        }

        Pipe* pipe = pcon->pipe;
        const PipeService* svc = qemu_pipe_find_type(pipeName);
        if (svc == NULL) {
            D("%s: Unknown server! %s", __FUNCTION__,pipeName);
            return PIPE_ERROR_INVAL;
        }

        void*  peer = svc->funcs.init(pipe, svc->opaque, pipeArgs);
        if (peer == NULL) {
            D("%s: Initialization failed!", __FUNCTION__);
            return PIPE_ERROR_INVAL;
        }

        pipe->opaque = peer;
        pipe->service = svc;
        pipe->funcs  = &svc->funcs;
        pipe->args   = g_strdup(pipeArgs);
        g_free(pcon);
    }

    return ret;
}

static int
pipeConnector_recvBuffers( void* opaque, GoldfishPipeBuffer* buffers, int numBuffers )
{
    return PIPE_ERROR_IO;
}

static unsigned
pipeConnector_poll( void* opaque )
{
    return PIPE_POLL_OUT;
}

static void
pipeConnector_wakeOn( void* opaque, int flags )
{
    
}

static const GoldfishPipeFuncs  pipeConnector_funcs = {
    NULL,  
    pipeConnector_close,        
    pipeConnector_sendBuffers,  
    pipeConnector_recvBuffers,  
    pipeConnector_poll,         
    pipeConnector_wakeOn,       
    NULL,
    NULL,
};


static uint64_t pipe_dev_read(void *opaque, hwaddr offset, uint32_t value)
{
    PipeDevice *dev = (PipeDevice *)opaque;
    switch (offset) {
    case PIPE_REG_STATUS:
        DR("%s: REG_STATUS status=%d (0x%x)", __FUNCTION__, dev->status, dev->status);
        return (uint64_t)dev->status;

    case PIPE_REG_CHANNEL:
        if (dev->signaled_pipes != NULL) {
            Pipe* pipe = dev->signaled_pipes;
            DR("%s: channel=0x%llx wanted=%d", __FUNCTION__,
               (unsigned long long)pipe->channel, pipe->wanted);
            dev->wakes = pipe->wanted;
            pipe->wanted = 0;
            dev->signaled_pipes = pipe->next_waked;
            pipe->next_waked = NULL;
            if (dev->signaled_pipes == NULL) {
                //goldfish_device_set_irq(&dev->dev, 0, 0);
                qemu_set_irq(dev->irq,0);
                DD("%s: lowering IRQ", __FUNCTION__);
            }
            return (uint64_t)(pipe->channel & 0xFFFFFFFFUL);
        }
        DR("%s: no signaled channels", __FUNCTION__);
        return 0;

    case PIPE_REG_CHANNEL_HIGH:
        if (dev->signaled_pipes != NULL) {
            Pipe* pipe = dev->signaled_pipes;
            //DR("%s: channel=0x%llx wanted=%d", __FUNCTION__,
            //   (unsigned long long)pipe->channel, pipe->wanted);
            //dev->wakes = pipe->wanted;
            //pipe->wanted = 0;
            //dev->signaled_pipes = pipe->next_waked;
            //pipe->next_waked = NULL;
            //if (dev->signaled_pipes == NULL) {
                //goldfish_device_set_irq(&dev->dev, 0, 0);
            //    qemu_set_irq(dev->irq,0);
            //    DD("%s: lowering IRQ", __FUNCTION__);
            //}
            return (uint64_t)(pipe->channel >> 32);
        }
        DR("%s: no signaled channels", __FUNCTION__);
        return 0;

    case PIPE_REG_WAKES:
        DR("%s: wakes %d", __FUNCTION__, dev->wakes);
        return (uint64_t)dev->wakes;

    case PIPE_REG_PARAMS_ADDR_HIGH:
        return (uint64_t)(dev->params_addr >> 32);

    case PIPE_REG_PARAMS_ADDR_LOW:
        return (uint64_t)(dev->params_addr & 0xFFFFFFFFUL);

    case PIPE_REG_VERSION:
        return (uint64_t)1;

    default:
        D("%s: offset=%d (0x%x)\n", __FUNCTION__, (int)offset, (int)offset);
    }
    return 0;
}

long long GetCurrentTimeMS()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    long long iDiff = (now.tv_sec * 1000LL * 1000LL) + now.tv_usec;
    return iDiff;
}

static void pipeDevice_doCommand( PipeDevice* dev, uint32_t command )
{
    Pipe** lookup = pipe_list_findp_channel(&dev->pipes, dev->channel);
    Pipe*  pipe   = *lookup;
    //CPUArchState* env = ((CPUArchState*)current_cpu->env_ptr);

    if (command != PIPE_CMD_OPEN && pipe == NULL) {
        dev->status = PIPE_ERROR_INVAL;
        return;
    }

    if (pipe != NULL && pipe->closed && command != PIPE_CMD_CLOSE) {
        dev->status = PIPE_ERROR_IO;
        return;
    }

    switch (command) {
    case PIPE_CMD_OPEN:
        DD("%s: CMD_OPEN channel=%p", __FUNCTION__, (unsigned long long)dev->channel);
        if (pipe != NULL) {
            dev->status = PIPE_ERROR_INVAL;
            break;
        }
        pipe = pipe_new(dev->channel, dev);
        pipe->next = dev->pipes;
        dev->pipes = pipe;
        dev->status = 0;
        break;

    case PIPE_CMD_CLOSE:
        DD("%s: CMD_CLOSE channel=0x%llx", __FUNCTION__, (unsigned long long)dev->channel);
        *lookup = pipe->next;
        pipe->next = NULL;
        pipe_list_remove_waked(&dev->signaled_pipes, pipe);
        pipe_free(pipe);
        break;

    case PIPE_CMD_POLL:
        dev->status = pipe->funcs->poll(pipe->opaque);
        DD("%s: CMD_POLL > status=%d", __FUNCTION__, dev->status);
        break;

    case PIPE_CMD_READ_BUFFER: {
        GoldfishPipeBuffer  buffer;
        hwaddr addr1;
        //long long temptime0 = GetCurrentTimeMS();
        buffer.size = dev->size;
        MemoryRegion *mr = address_space_translate(&address_space_memory,dev->address,&addr1,&buffer.size,true);
        addr1 += memory_region_get_ram_addr(mr);
        buffer.data = qemu_get_ram_ptr(addr1);
        buffer.size = dev->size;
        //long long temptime1 = GetCurrentTimeMS();
        dev->status = pipe->funcs->recvBuffers(pipe->opaque, &buffer, 1);
        DD("%s: CMD_READ_BUFFER channel=0x%llx address=0x%16llx size=%d > status=%d",
           __FUNCTION__, (unsigned long long)dev->channel, (unsigned long long)dev->address,
           dev->size, dev->status);
        //long long temptime2 = GetCurrentTimeMS();
        //printf("%d\t%lld\t%lld\t%lld\n",dev->size,temptime1-temptime0,temptime2-temptime1,temptime2-temptime0);
        /*static int stats_totalBytes = 0;
        long long temptime = GetCurrentTimeMS();
        static long long stats_t0 = 0;
        stats_totalBytes += dev->size;
        long long dt = temptime - stats_t0;
        if (dt > 1000) {
            float dts = (float)dt / 1000.0f;
            printf("Used Read Bandwidth %5.5f MB/s\n", ((float)stats_totalBytes / dts) / (1024.0f*1024.0f));
            stats_totalBytes = 0;
            stats_t0 = temptime;
        }*/

        break;
    }

    case PIPE_CMD_WRITE_BUFFER: {
        GoldfishPipeBuffer  buffer;
        hwaddr addr1;
        //long long temptime0 = GetCurrentTimeMS();
        buffer.size = dev->size;
        MemoryRegion *mr = address_space_translate(&address_space_memory,dev->address,&addr1,&buffer.size,false);
        addr1 += memory_region_get_ram_addr(mr);
        buffer.data = qemu_get_ram_ptr(addr1);
        buffer.size = dev->size;
	//long long temptime1 = GetCurrentTimeMS();
        dev->status = pipe->funcs->sendBuffers(pipe->opaque, &buffer, 1);
        DD("%s: CMD_WRITE_BUFFER channel=0x%llx address=0x%16llx size=%d > status=%d",
           __FUNCTION__, (unsigned long long)dev->channel, (unsigned long long)dev->address,
           dev->size, dev->status);
        //long long temptime2 = GetCurrentTimeMS();
        //float temptime = ((float)(temptime1-temptime0)/1000);
        
        //if(dev->size == 4096)
        //   printf("%d\t%lld\t%lld\t%lld\n",dev->size,temptime1-temptime0,temptime2-temptime1,temptime2-temptime0);
	/*static int stats_totalBytes = 0;
        long long temptime = GetCurrentTimeMS();
        static long long stats_t0 = 0;
        stats_totalBytes += dev->size;
        long long dt = temptime - stats_t0;
        if (dt > 1000) {
            float dts = (float)dt / 1000.0f;
            printf("Used Send Bandwidth %5.5f MB/s\n", ((float)stats_totalBytes / dts) / (1024.0f*1024.0f));
            stats_totalBytes = 0;
            stats_t0 = temptime;
        }*/

        break;
    }

    case PIPE_CMD_WAKE_ON_READ:
        DD("%s: CMD_WAKE_ON_READ channel=0x%llx", __FUNCTION__, (unsigned long long)dev->channel);
        if ((pipe->wanted & PIPE_WAKE_READ) == 0) {
            pipe->wanted |= PIPE_WAKE_READ;
            pipe->funcs->wakeOn(pipe->opaque, pipe->wanted);
        }
        dev->status = 0;
        break;

    case PIPE_CMD_WAKE_ON_WRITE:
        DD("%s: CMD_WAKE_ON_WRITE channel=0x%llx", __FUNCTION__, (unsigned long long)dev->channel);
        if ((pipe->wanted & PIPE_WAKE_WRITE) == 0) {
            pipe->wanted |= PIPE_WAKE_WRITE;
            pipe->funcs->wakeOn(pipe->opaque, pipe->wanted);
        }
        dev->status = 0;
        break;

    default:
        D("%s: command=%d (0x%x)\n", __FUNCTION__, command, command);
    }
}

static void pipe_dev_write(void *opaque, hwaddr offset, uint64_t value, unsigned size)
{
    PipeDevice *s = (PipeDevice *)opaque;

    switch (offset) {
    case PIPE_REG_COMMAND:
        DR("%s: command=%d (0x%x)", __FUNCTION__, (int)value, (unsigned int)value);
        pipeDevice_doCommand(s, value);
        break;

    case PIPE_REG_SIZE:
        DR("%s: size=%d (0x%x)", __FUNCTION__, (int)value, (unsigned int)value);
        s->size = value;
        break;

    case PIPE_REG_ADDRESS:
        DR("%s: address=%d (0x%x)", __FUNCTION__, (int)value, (unsigned int)value);
        uint64_set_low(&s->address, value);
        break;

    case PIPE_REG_ADDRESS_HIGH:
        DR("%s: address_high=%d (0x%x)", __FUNCTION__, (int)value, (unsigned int)value);
        uint64_set_high(&s->address, value);
        break;

    case PIPE_REG_CHANNEL:
        DR("%s: channel=%p %d %d \n ", __FUNCTION__, value, size,sizeof(long));
        uint64_set_low(&s->channel, value);
        break;

    case PIPE_REG_CHANNEL_HIGH:
        DR("%s: channel_high=%d (0x%x) \n ", __FUNCTION__, (int)value, (unsigned int)value);
        uint64_set_high(&s->channel, value);
        break;

    case PIPE_REG_PARAMS_ADDR_HIGH:
        s->params_addr = (s->params_addr & ~(0xFFFFFFFFULL << 32) ) |
                          ((uint64_t)value << 32);
        break;

    case PIPE_REG_PARAMS_ADDR_LOW:
        s->params_addr = (s->params_addr & ~(0xFFFFFFFFULL) ) | value;
        break;

    case PIPE_REG_ACCESS_PARAMS:
        {
            struct access_params aps;
            struct access_params_64 aps64;
            uint32_t cmd;

            if (s->params_addr == 0)
                break;

            if (qemu_guest_is_64bit()) {
                cpu_physical_memory_read(s->params_addr, (void*)&aps64,
                                         sizeof(aps64));
            } else {
                cpu_physical_memory_read(s->params_addr, (void*)&aps,
                                         sizeof(aps));
            }
            
            if (qemu_guest_is_64bit()) {
                s->channel = aps64.channel;
                s->size = aps64.size;
                s->address = aps64.address;
                cmd = aps64.cmd;
            } else {
                s->channel = aps.channel;
                s->size = aps.size;
                s->address = aps.address;
                cmd = aps.cmd;
            }
            if ((cmd != PIPE_CMD_READ_BUFFER) && (cmd != PIPE_CMD_WRITE_BUFFER))
                break;

            pipeDevice_doCommand(s, cmd);
            if (qemu_guest_is_64bit()) {
                aps64.result = s->status;
                cpu_physical_memory_write(s->params_addr, (void*)&aps64,
                                          sizeof(aps64));
            } else {
                aps.result = s->status;
                cpu_physical_memory_write(s->params_addr, (void*)&aps,
                                          sizeof(aps));
            }
        }
        break;

    default:
        D("%s: offset=%d (0x%x) value=%d (0x%x)\n", __FUNCTION__, (int)offset,
            (int)offset, (int)value, (int)value);
        break;
    }
}


static const MemoryRegionOps qemu_pipe_ops = {
    .read = pipe_dev_read,
    .write = pipe_dev_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static int qemu_pipe_initfn(SysBusDevice *sbd)
{
    PipeDevice *s = QEMU_PIPE(sbd);
    memory_region_init_io(&s->iomem, OBJECT(s), &qemu_pipe_ops, s, TYPE_QEMU_PIPE, 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);
    return 0;
}

static void qemu_pipe_class_init(ObjectClass *klass, void *data)
{
    SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);
    k->init = qemu_pipe_initfn;
}

static const TypeInfo qemu_pipe_info = {
    .name          = TYPE_QEMU_PIPE,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(PipeDevice),
    .class_init    = qemu_pipe_class_init,
};

static void qemu_pipe_register_types(void)
{
    type_register_static(&qemu_pipe_info);
}

type_init(qemu_pipe_register_types)


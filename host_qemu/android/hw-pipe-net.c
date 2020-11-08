// #include "android/sockets.h"
// #include "android/utils/assert.h"
// #include "android/utils/panic.h"
// #include "android/utils/system.h"
#include "async-utils.h"
#include "looper.h"
#include "opengles.h"
#include "hw-pipe-net.h"
#include "hw/android/pipe.h"
#include "sockets.h"

enum {
    STATE_INIT,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_CLOSING_GUEST,
    STATE_CLOSING_SOCKET
};

typedef struct {
    void*           hwpipe;
    int             state;
    int             wakeWanted;
    LoopIo          io[1];
    AsyncConnector  connector[1];
} NetPipe;

static void
netPipe_free( NetPipe*  pipe )
{
    int  fd;

    fd = pipe->io->fd;
    loopIo_done(pipe->io);
    socket_close(fd);

    g_free(pipe);
}


static void
netPipe_resetState( NetPipe* pipe )
{
    if ((pipe->wakeWanted & PIPE_WAKE_WRITE) != 0) {
        loopIo_wantWrite(pipe->io);
    } else {
        loopIo_dontWantWrite(pipe->io);
    }

   if (pipe->state == STATE_CONNECTED && (pipe->wakeWanted & PIPE_WAKE_READ) != 0) {
        loopIo_wantRead(pipe->io);
    } else {
        loopIo_dontWantRead(pipe->io);
    }
}

static void
netPipe_closeFromSocket( void* opaque )
{
    NetPipe*  pipe = opaque;

    printf("%s", __FUNCTION__);

    if (pipe->state == STATE_CLOSING_GUEST) {
        netPipe_free(pipe);
        return;
    }

    if (pipe->hwpipe != NULL) {
        qemu_pipe_close(pipe->hwpipe);
        pipe->hwpipe = NULL;
    }

    pipe->state = STATE_CLOSING_SOCKET;
    netPipe_resetState(pipe);
}

static void
netPipe_io_func( void* opaque, int fd, unsigned events )
{
    NetPipe*  pipe = opaque;
    int         wakeFlags = 0;

    if (pipe->state == STATE_CONNECTING) {
        AsyncStatus  status = asyncConnector_run(pipe->connector);
        if (status == ASYNC_NEED_MORE) {
            return;
        }
        else if (status == ASYNC_ERROR) {

            netPipe_closeFromSocket(pipe);
            return;
        }
        pipe->state = STATE_CONNECTED;
        netPipe_resetState(pipe);
        return;
    }

    if ((events & LOOP_IO_READ) != 0) {
        if ((pipe->wakeWanted & PIPE_WAKE_READ) != 0) {
            wakeFlags |= PIPE_WAKE_READ;
        }
    }

    if ((events & LOOP_IO_WRITE) != 0) {
        if ((pipe->wakeWanted & PIPE_WAKE_WRITE) != 0) {
            wakeFlags |= PIPE_WAKE_WRITE;
        }
    }

    if (wakeFlags != 0) {
        qemu_pipe_wake(pipe->hwpipe, wakeFlags);
        pipe->wakeWanted &= ~wakeFlags;
    }

    netPipe_resetState(pipe);
}


static void*
netPipe_initFromAddress( void* hwpipe, const SockAddress*  address, Looper* looper )
{
    NetPipe*     pipe;
    pipe=g_malloc0(sizeof(*pipe));

    pipe->hwpipe = hwpipe;
    pipe->state  = STATE_INIT;

    {
        AsyncStatus  status;

        int  fd = socket_create( sock_address_get_family(address), SOCKET_STREAM );
        if (fd < 0) {
            printf("%s: Could create socket from address family!", __FUNCTION__);
            netPipe_free(pipe);
            return NULL;
        }

        loopIo_init(pipe->io, looper, fd, netPipe_io_func, pipe);
        status = asyncConnector_init(pipe->connector, address, pipe->io);
        pipe->state = STATE_CONNECTING;

        if (status == ASYNC_ERROR) {
            printf("%s: Could not connect to socket: %s",
              __FUNCTION__, errno_str);
            netPipe_free(pipe);
            return NULL;
        }
        if (status == ASYNC_COMPLETE) {
            pipe->state = STATE_CONNECTED;
            netPipe_resetState(pipe);
        }
    }

    return pipe;
}

static void
netPipe_closeFromGuest( void* opaque )
{
    NetPipe*  pipe = opaque;
    netPipe_free(pipe);
}

static int netPipeReadySend(NetPipe *pipe)
{
    if (pipe->state == STATE_CONNECTED)
        return 0;
    else if (pipe->state == STATE_CONNECTING)
        return PIPE_ERROR_AGAIN;
    else if (pipe->hwpipe == NULL)
        return PIPE_ERROR_INVAL;
    else
        return PIPE_ERROR_IO;
}

static int
netPipe_sendBuffers( void* opaque, const GoldfishPipeBuffer* buffers, int numBuffers )
{
    NetPipe*  pipe = opaque;
    int       count = 0;
    int       ret   = 0;
    int       buffStart = 0;
    const GoldfishPipeBuffer* buff = buffers;
    const GoldfishPipeBuffer* buffEnd = buff + numBuffers;

    ret = netPipeReadySend(pipe);
    if (ret != 0)
        return ret;

    for (; buff < buffEnd; buff++)
        count += buff->size;

    buff = buffers;
    while (count > 0) {
        int  avail = buff->size - buffStart;
        int  len = socket_send(pipe->io->fd, buff->data + buffStart, avail);
        if (len > 0) {
            buffStart += len;
            if (buffStart >= buff->size) {
                buff++;
                buffStart = 0;
            }
            count -= len;
            ret   += len;
            continue;
        }

        if (len == 0) {
            if (ret == 0)
                ret = PIPE_ERROR_IO;
            break;
        }

        if (ret > 0) {
            break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //printf("**************netPipe_sendBuffers:PIPE_ERROR_AGAIN \n");
            ret = PIPE_ERROR_AGAIN;
        } else {
            ret = PIPE_ERROR_IO;
        }
        break;
    }

    return ret;
}

static int
netPipe_recvBuffers( void* opaque, GoldfishPipeBuffer*  buffers, int  numBuffers )
{
    NetPipe*  pipe = opaque;
    int       count = 0;
    int       ret   = 0;
    int       buffStart = 0;
    GoldfishPipeBuffer* buff = buffers;
    GoldfishPipeBuffer* buffEnd = buff + numBuffers;

    for (; buff < buffEnd; buff++)
        count += buff->size;

    buff = buffers;
    while (count > 0) {
        int  avail = buff->size - buffStart;
        int  len = socket_recv(pipe->io->fd, buff->data + buffStart, avail);

        if (len > 0) {
            buffStart += len;
            if (buffStart >= buff->size) {
                buff++;
                buffStart = 0;
            }
            count -= len;
            ret   += len;
            continue;
        }

        if (len == 0) {
            if (ret == 0)
                ret = PIPE_ERROR_IO;
            break;
        }

        if (ret > 0) {
            break;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            //printf("**************netPipe_recvBuffers:PIPE_ERROR_AGAIN \n");
            ret = PIPE_ERROR_AGAIN;
        } else {
            ret = PIPE_ERROR_IO;
        }
        break;
    }
    return ret;
}

static unsigned
netPipe_poll( void* opaque )
{
    NetPipe*  pipe = opaque;
    unsigned  mask = loopIo_poll(pipe->io);
    unsigned  ret  = 0;

    if (mask & LOOP_IO_READ)
        ret |= PIPE_POLL_IN;
    if (mask & LOOP_IO_WRITE)
        ret |= PIPE_POLL_OUT;

    return ret;
}

static void
netPipe_wakeOn( void* opaque, int flags )
{
    NetPipe*  pipe = opaque;

    //printf("%s: flags=%d", __FUNCTION__, flags);

    pipe->wakeWanted |= flags;
    netPipe_resetState(pipe);
}


static void*
netPipe_initTcp( void* hwpipe, void* _looper, const char* args )
{
    SockAddress  address;
    uint16_t     port;
    void*        ret;

    if (args == NULL) {
        printf("%s: Missing address!", __FUNCTION__);
        return NULL;
    }
    printf("%s: Port is '%s'", __FUNCTION__, args);
    {
        char* end;
        long  val = strtol(args, &end, 10);
        if (end == NULL || *end != '\0' || val <= 0 || val > 65535) {
            printf("%s: Invalid port number: '%s'", __FUNCTION__, args);
        }
        port = (uint16_t)val;
    }
    sock_address_init_inet(&address, SOCK_ADDRESS_INET_LOOPBACK, port);

    ret = netPipe_initFromAddress(hwpipe, &address, _looper);

    sock_address_done(&address);
    return ret;
}

static void*
netPipe_initUnix( void* hwpipe, void* _looper, const char* args )
{
    SockAddress  address;
    void*        ret;

    if (args == NULL || args[0] == '\0') {
        printf("%s: Missing address!", __FUNCTION__);
        return NULL;
    }
    //printf("%s: Address is '%s'", __FUNCTION__, args);

    sock_address_init_unix(&address, args);

    ret = netPipe_initFromAddress(hwpipe, &address, _looper);

    sock_address_done(&address);
    return ret;
}

static const GoldfishPipeFuncs  netPipeTcp_funcs = {
    netPipe_initTcp,
    netPipe_closeFromGuest,
    netPipe_sendBuffers,
    netPipe_recvBuffers,
    netPipe_poll,
    netPipe_wakeOn,
    NULL,  /* we can't save these */
    NULL,  /* we can't load these */
};

static const GoldfishPipeFuncs  netPipeUnix_funcs = {
    netPipe_initUnix,
    netPipe_closeFromGuest,
    netPipe_sendBuffers,
    netPipe_recvBuffers,
    netPipe_poll,
    netPipe_wakeOn,
    NULL,  /* we can't save these */
    NULL,  /* we can't load these */
};

static int  _opengles_init = 0;

static void*
openglesPipe_init( void* hwpipe, void* _looper, const char* args )
{
    NetPipe *pipe;

    if (!_opengles_init) {
        printf("Trying to open the OpenGLES pipe without GPU emulation! \n");
        return NULL;
    }

    char server_addr[PATH_MAX];
    android_gles_server_path(server_addr, sizeof(server_addr));
    if (android_gles_fast_pipes) {
        pipe = (NetPipe *)netPipe_initUnix(hwpipe, _looper, server_addr);
        //printf("Creating Unix OpenGLES pipe for GPU emulation: %s \n", server_addr);
    } else {
        pipe = (NetPipe *)netPipe_initTcp(hwpipe, _looper, server_addr);
        printf("Creating TCP OpenGLES pipe for GPU emulation! \n");
    }

    if (pipe != NULL) {
       socket_set_nodelay1(pipe->io->fd);
    }

    return pipe;
}

static const GoldfishPipeFuncs  openglesPipe_funcs = {
    openglesPipe_init,
    netPipe_closeFromGuest,
    netPipe_sendBuffers,
    netPipe_recvBuffers,
    netPipe_poll,
    netPipe_wakeOn,
    NULL,  /* we can't save these */
    NULL,  /* we can't load these */
};

void
android_net_pipes_init(void)
{
    Looper*  looper = looper_newCore();

    qemu_pipe_add_type( "tcp", looper, &netPipeTcp_funcs );

    qemu_pipe_add_type( "unix", looper, &netPipeUnix_funcs );

    qemu_pipe_add_type( "opengles", looper, &openglesPipe_funcs );
}

int
android_init_opengles_pipes(void)
{
    _opengles_init = 1;
    return 0;
}

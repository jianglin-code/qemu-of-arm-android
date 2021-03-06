# Automatically generated by configure - do not modify

all:
prefix=/usr/local
bindir=${prefix}/bin
libdir=${prefix}/lib
libexecdir=${prefix}/libexec
includedir=${prefix}/include
mandir=${prefix}/share/man
sysconfdir=${prefix}/etc
qemu_confdir=${prefix}/etc/qemu
qemu_datadir=${prefix}/share/qemu
qemu_docdir=${prefix}/share/doc/qemu
qemu_moddir=${prefix}/lib/qemu
qemu_localstatedir=${prefix}/var
qemu_helperdir=${prefix}/libexec
extra_cflags= 
extra_ldflags=
qemu_localedir=${prefix}/share/locale
libs_softmmu=-lpixman-1   -lutil  -L$(BUILD_DIR)/dtc/libfdt -lfdt
ARCH=aarch64
STRIP=aarch64-linux-gnu-strip
CONFIG_POSIX=y
CONFIG_LINUX=y
CONFIG_STATIC=y
CONFIG_SLIRP=y
CONFIG_SMBD_COMMAND="/usr/sbin/smbd"
CONFIG_L2TPV3=y
CONFIG_AUDIO_DRIVERS=
CONFIG_BDRV_RW_WHITELIST=
CONFIG_BDRV_RO_WHITELIST=
CONFIG_VNC=y
CONFIG_FNMATCH=y
VERSION=2.4.0.1
PKGVERSION=
SRC_PATH=/home/zp/share/qemu
TARGET_DIRS=aarch64-softmmu
CONFIG_UTIMENSAT=y
CONFIG_PIPE2=y
CONFIG_ACCEPT4=y
CONFIG_SPLICE=y
CONFIG_EVENTFD=y
CONFIG_FALLOCATE=y
CONFIG_FALLOCATE_PUNCH_HOLE=y
CONFIG_POSIX_FALLOCATE=y
CONFIG_SYNC_FILE_RANGE=y
CONFIG_FIEMAP=y
CONFIG_DUP3=y
CONFIG_PPOLL=y
CONFIG_PRCTL_PR_SET_TIMERSLACK=y
CONFIG_EPOLL=y
CONFIG_EPOLL_CREATE1=y
CONFIG_EPOLL_PWAIT=y
CONFIG_SENDFILE=y
CONFIG_TIMERFD=y
CONFIG_SETNS=y
CONFIG_INOTIFY=y
CONFIG_INOTIFY1=y
CONFIG_BYTESWAP_H=y
CONFIG_HAS_GLIB_SUBPROCESS_TESTS=y
GLIB_CFLAGS=-pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include  
CONFIG_ATTR=y
CONFIG_VHOST_SCSI=y
CONFIG_VHOST_NET_USED=y
INSTALL_BLOBS=yes
CONFIG_IOVEC=y
CONFIG_PREADV=y
CONFIG_FDT=y
CONFIG_FDATASYNC=y
CONFIG_MADVISE=y
CONFIG_POSIX_MADVISE=y
CONFIG_SIGEV_THREAD_ID=y
CONFIG_ZERO_MALLOC=y
CONFIG_QOM_CAST_DEBUG=y
CONFIG_COROUTINE_BACKEND=gthread
CONFIG_COROUTINE_POOL=0
CONFIG_OPEN_BY_HANDLE=y
CONFIG_LINUX_MAGIC_H=y
CONFIG_PRAGMA_DIAGNOSTIC_AVAILABLE=y
CONFIG_HAS_ENVIRON=y
CONFIG_INT128=y
CONFIG_GETAUXVAL=y
HOST_USB=stub
CONFIG_TPM=$(CONFIG_SOFTMMU)
TRACE_BACKENDS=nop
CONFIG_TRACE_NOP=y
CONFIG_TRACE_FILE=trace
CONFIG_THREAD_SETNAME_BYTHREAD=y
CONFIG_PTHREAD_SETNAME_NP=y
TOOLS=qemu-ga$(EXESUF) qemu-nbd$(EXESUF) qemu-img$(EXESUF) qemu-io$(EXESUF) 
ROMS=
MAKE=make
INSTALL=install
INSTALL_DIR=install -d -m 0755
INSTALL_DATA=install -c -m 0644
INSTALL_PROG=install -c -m 0755
INSTALL_LIB=install -c -m 0644
PYTHON=python -B
CC=aarch64-linux-gnu-gcc
CC_I386=i386-pc-linux-gnu-gcc
HOST_CC=cc
CXX=
OBJCC=aarch64-linux-gnu-gcc
AR=aarch64-linux-gnu-ar
ARFLAGS=rv
AS=aarch64-linux-gnu-as
CPP=aarch64-linux-gnu-gcc -E
OBJCOPY=aarch64-linux-gnu-objcopy
LD=aarch64-linux-gnu-ld
NM=aarch64-linux-gnu-nm
WINDRES=aarch64-linux-gnu-windres
LIBTOOL=
CFLAGS=-O2 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include   -g 
CFLAGS_NOPIE=
QEMU_CFLAGS=-I/usr/include/pixman-1   -I$(SRC_PATH)/dtc/libfdt  -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wstrict-prototypes -Wredundant-decls -Wall -Wundef -Wwrite-strings -Wmissing-prototypes -fno-strict-aliasing -fno-common  -Wendif-labels -Wmissing-include-dirs -Wempty-body -Wnested-externs -Wformat-security -Wformat-y2k -Winit-self -Wignored-qualifiers -Wold-style-declaration -Wold-style-definition -Wtype-limits -fstack-protector-all
QEMU_INCLUDES=-I$(SRC_PATH)/tcg -I$(SRC_PATH)/tcg/$(ARCH) -I$(SRC_PATH)/linux-headers -I/home/zp/share/qemu/linux-headers -I. -I$(SRC_PATH) -I$(SRC_PATH)/include
AUTOCONF_HOST := --host=aarch64-linux-gnu
LDFLAGS=-Wl,--warn-common -static -g 
LDFLAGS_NOPIE=
LIBTOOLFLAGS= -Wc,-fstack-protector-all
LIBS+=-lm -pthread -lgthread-2.0 -lglib-2.0 -lpcre    -lz -lrt
LIBS_TOOLS+=
EXESUF=
DSOSUF=.so
LDFLAGS_SHARED=-shared
LIBS_QGA+=-lm -pthread -lgthread-2.0 -lglib-2.0 -lpcre    -lrt
POD2MAN=pod2man --utf8
TRANSLATE_OPT_CFLAGS=
config-host.h: subdir-dtc

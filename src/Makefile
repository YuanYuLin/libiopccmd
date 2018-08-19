
TARGET = libiopccmd.so

CFLAGS += -I$(SDKSTAGE)/usr/include/libiopcmisc
CFLAGS += -I$(SDKSTAGE)/usr/include/libiopcdao
CFLAGS += -I$(SDKSTAGE)/usr/include/libiopcnet
CFLAGS += -I$(SDKSTAGE)/usr/include/
CFLAGS += -I./include/

LDFLAGS += -L$(SDKSTAGE)/lib/ 
LDFLAGS += -L$(SDKSTAGE)/usr/lib/ 
LDFLAGS += -lc -lpthread -ldl -liopcnet

ifeq ($(SUPPORT_LXC), y)
CFLAGS += -I$(SDKSTAGE)/usr/include/lxc
CFLAGS += -DSUPPORT_LXC
LDFLAGS += -llxc -lselinux -lseccomp -lcap -lapparmor
LDFLAGS += -lnettle -lhogweed -lgmp -lidn -ltasn1 -lp11-kit -lffi -lpcre -lgnutls
endif
ifeq ($(SUPPORT_RFB), y)
#CFLAGS += -I$(SDKSTAGE)/usr/include/libiopcrfb
#CFLAGS += -DSUPPORT_RFB
#LDFLAGS += -liopcrfb
endif

SRC = 
SRC += ops_cmd.c
SRC += cmd_processor1.c
SRC += cmd_processor2.c
SRC += cmd_processor_shell.c

SRC += shell_common.c
SRC += shell_lxc.c
SRC += shell_qemu.c
SRC += shell_ssh.c
SRC += shell_drbd.c
SRC += shell_mount.c
SRC += shell_netifc.c
SRC += shell_misc.c

include Makefile.include.lib


TARGET = libiopccmd.so

CFLAGS += -I$(SDKSTAGE)/usr/include/libiopcmisc
CFLAGS += -I$(SDKSTAGE)/usr/include/libiopcdao
CFLAGS += -I$(SDKSTAGE)/usr/include/
CFLAGS += -I$(SDKSTAGE)/usr/include/lxc
CFLAGS += -I./include/
CFLAGS += 

LDFLAGS += -L$(SDKSTAGE)/lib/ 
LDFLAGS += -L$(SDKSTAGE)/usr/lib/ 
LDFLAGS += -lc -lpthread -ldl

SRC = 
SRC += ops_cmd.c
SRC += cmd_processor1.c
SRC += cmd_processor2.c
SRC += cmd_processor_shell.c

include Makefile.include.lib

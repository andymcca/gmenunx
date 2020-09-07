PLATFORM = retrofw

export CROSS_COMPILE = /opt/mipsel-RetroFW-linux-uclibc/usr/bin/mipsel-linux-

CFLAGS += -DTARGET_RETROFW
CFLAGS += -Os
CFLAGS += -mhard-float -mips32 -mno-mips16

LDFLAGS = -lintl

include gmenunx.mk

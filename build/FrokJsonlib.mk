###########################################################
# mk for project FrokJsonlib
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FrokJsonlib

# Target settings
LANG := c++
TARGET := libFrokJsonlib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			                    \
	-L/usr/lib/x86_64-linux-gnu -lrt

CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/		                    \

include makefile.actions

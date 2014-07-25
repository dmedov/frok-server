###########################################################
# mk for project FaceCommonLib
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceCommonLib

# Target settings
LANG := c++
TARGET := libFaceCommonLib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			\
	-L/usr/lib/x86_64-linux-gnu	-lrt -lpthread \
	$(OPENCV_LIB)


CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/		\
	$(OPENCV_INCLUDES)

include makefile.actions

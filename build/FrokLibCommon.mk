###########################################################
# mk for project FrokLibCommon
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FrokLibCommon

# Target settings
LANG := c
TARGET := libFrokLib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			\
	-L/usr/lib/x86_64-linux-gnu	-lrt -lpthread \
	$(OPENCV_LIB)


CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/		\
	../$(SRCDIR)/linux/
	
HDRDIRS :=  ../$(SRCDIR)/		\
	../$(SRCDIR)/linux/		\
	$(OPENCV_INCLUDES)

include makefile.actions

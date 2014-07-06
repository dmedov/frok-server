###########################################################
# mk for project FaceDetectionApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceDetectionLib

# Target settings
LANG := c++
TARGET := libFaceDetectionLib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			\
	-L/usr/lib/x86_64-linux-gnu	\
	-lFaceDetectionLib -lrt -lpthread \
	$(OPENCV_LIB)


CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/		\
	$(OPENCV_INCLUDES)

include makefile.actions

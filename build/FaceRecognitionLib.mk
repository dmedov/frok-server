###########################################################
# mk for project FaceRecognitionLib
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceRecognitionLib

# Target settings
LANG := c++
TARGET := libFaceRecognitionLib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			                            \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread               \
    -lFaceCommonLib                                         \
    $(OPENCV_LIB)                                           \


DEPENDENCIES :=                             \
    $(BINOUTDIR)/libFaceCommonLib.a         \

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	\
	../FaceCommonLib/	    \
	$(OPENCV_INCLUDES)

include makefile.actions

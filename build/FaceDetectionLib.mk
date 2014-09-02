###########################################################
# mk for project FaceDetectionApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceDetectionLib

# Target settings
LANG := c++
TARGET := libFrokLib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			                    \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread       \
    -lFaceCommonLib                                         \
    $(OPENCV_LIB)                                           \

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/		\
	../FrokLibCommon/	    	\
	../FrokLibCommon/linux/		\
	$(OPENCV_INCLUDES)

include makefile.actions

###########################################################
# mk for project FrokAPILib
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FrokAPILib

# Target settings
LANG := c++
TARGET := libFrokLib.a
TARGETTYPE := staticlib

LIBS := -L$(BINOUTDIR)			                    \
	-L/usr/lib/x86_64-linux-gnu	-lrt -lpthread      \
    -lFrokJsonlib                                   \
    -lFaceCommonLib                                 \
	-lFaceDetectionLib                              \
    -lFaceRecognitionLib                            \
    -lFrokLib                                       \
	$(OPENCV_LIB)

CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/		                    \
    ../FrokJsonlib/	                                \
	../FaceDetectionLib/	                        \
	../FaceRecognitionLib/	                        \
    ../FaceCommonLib/	                            \
	$(OPENCV_INCLUDES)

include makefile.actions

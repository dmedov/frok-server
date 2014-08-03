###########################################################
# mk for project FrokServerApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FrokServerApp

# Target settings
LANG := c++
TARGET := FrokServerApp
TARGETTYPE := executable

CFLAGS += -Wno-unknown-pragmas
LIBS := -L$(BINOUTDIR)			                            \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread               \
    -lFaceCommonLib                                         \

DEPENDENCIES :=                             \
    $(BINOUTDIR)/libFaceCommonLib.a         \

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	\
	../FaceDetectionLib/	\
	../FaceRecognitionLib/	\
	../FaceCommonLib/	    \
	$(OPENCV_INCLUDES)

make_dependencies:
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceCommonLib.mk build
include makefile.actions

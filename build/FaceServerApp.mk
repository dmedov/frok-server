###########################################################
# mk for project FaceServerApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceServerApp

# Target settings
LANG := c++
TARGET := FaceServerApp
TARGETTYPE := executable

CFLAGS += -Wno-unknown-pragmas
LIBS := -L$(BINOUTDIR)			                            \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread               \
    -lFaceCommonLib                                         \
#	-lFaceDetectionLib                                      \
#    -lFaceRecognitionLib                                    \
#    $(OPENCV_LIB)                                           \


DEPENDENCIES :=                             \
    $(BINOUTDIR)/libFaceCommonLib.a         \
#    $(BINOUTDIR)/libFaceDetectionLib.a      \
#    $(BINOUTDIR)/libFaceRecognitionLib.a    \

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	\
	../FaceDetectionLib/	\
	../FaceRecognitionLib/	\
	../FaceCommonLib/	    \
	$(OPENCV_INCLUDES)

make_dependencies:
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceCommonLib.mk build
#    @$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceDetectionLib.mk build
#    @$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceRecognition.mk build
include makefile.actions

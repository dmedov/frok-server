###########################################################
# mk for project FrokAgentApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FrokAgentApp

# Target settings
LANG := c++
TARGET := FrokAgentApp
TARGETTYPE := executable

CFLAGS += -Wno-unknown-pragmas
LIBS := -L$(BINOUTDIR)			                    \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread       \
    -lFaceCommonLib                                 \
	-lFaceDetectionLib                              \
    -lFaceRecognitionLib                            \
	-lFrokAPILib                                    \
    $(OPENCV_LIB)                                   \

DEPENDENCIES :=                                     \
    $(BINOUTDIR)/libFaceCommonLib.a                 \
    $(BINOUTDIR)/libFaceDetectionLib.a              \
    $(BINOUTDIR)/libFaceRecognitionLib.a            \
    $(BINOUTDIR)/libFrokAPILib.a                    \

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	                        \
	../FaceCommonLib/	                            \
	../FaceDetectionLib/	                        \
	../FaceRecognitionLib/	                        \
	../FrokAPILib/	                                \
	$(OPENCV_INCLUDES)

make_dependencies:
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceCommonLib.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FrokAPILib.mk build

include makefile.actions
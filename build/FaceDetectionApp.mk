###########################################################
# mk for project FaceDetectionApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceDetectionApp

# Target settings
LANG := c++
TARGET := FaceDetectionApp
TARGETTYPE := executable

CFLAGS += -Wno-unknown-pragmas
LIBS := -L/usr/lib/i386-linux-gnu -L$(BINOUTDIR) -lFaceDetectionLib -lrt -lpthread

DEPENDENCIES := $(BINOUTDIR)/libFaceDetectionLib.a

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	\
	../FaceDetectionLib/	\
	$(OPENCV_INCLUDES)

make_dependencies:
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceDetectionLib.mk build
include makefile.actions

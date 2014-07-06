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
LIBS := -L$(BINOUTDIR)			\
	-L/usr/lib/x86_64-linux-gnu	\
	-lFaceDetectionLib -lrt -lpthread \
	$(OPENCV_LIB)


DEPENDENCIES := $(BINOUTDIR)/libFaceDetectionLib.a

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	\
	../FaceDetectionLib/	\
	$(OPENCV_INCLUDES)

make_dependencies:
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceDetectionLib.mk build
include makefile.actions

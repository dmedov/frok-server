###########################################################
# mk for project FrokTestApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FrokTestApp

# Target settings
LANG := c++
TARGET := FrokTestApp
TARGETTYPE := executable

CFLAGS += -Wno-unknown-pragmas
LIBS := -L$(BINOUTDIR)			                    \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread       \
    -lFrokLib                                       \
    $(OPENCV_LIB)                                   \

DEPENDENCIES :=                                     \
    $(BINOUTDIR)/FrokLib.a                          \

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
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceCommonLib.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceDetectionLib.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceRecognitionLib.mk build

include makefile.actions

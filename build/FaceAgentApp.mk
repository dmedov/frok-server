###########################################################
# mk for project FaceAgentApp
###########################################################

include makefile.consts
include opencv.include
SRCDIR = FaceAgentApp

# Target settings
LANG := c++
TARGET := FaceAgentApp
TARGETTYPE := executable

CFLAGS += -Wno-unknown-pragmas
LIBS := -L$(BINOUTDIR)			                    \
	-L/usr/lib/x86_64-linux-gnu	-lrt -pthread       \
    -lFaceCommonLib                                 \
	-lFrokAPILib                                    \
    $(OPENCV_LIB)                                   \

DEPENDENCIES :=                                     \
    $(BINOUTDIR)/libFaceCommonLib.a                 \
    $(BINOUTDIR)/libFrokAPILib.a                    \

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/	                        \
	../FaceCommonLib/	                            \
	../FrokAPILib/	                                \
	$(OPENCV_INCLUDES)

make_dependencies:
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FaceCommonLib.mk build
	@$(MAKE) --no-print-directory CCFLAG="$(CCFLAG)" LDFLAG="$(LDFLAG)" -f FrokAPILib.mk build

include makefile.actions

# Initialize make tools
include makefile.consts
include opencv.include
SRCDIR = FaceDetectionLib

# Target settings
LANG := c++
TARGET := libFaceDetectionLib.a
TARGETTYPE := staticlib

CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/		\
	$(OPENCV_INCLUDES)

# Automated target make procedures: designed to fit any product
include makefile.actions

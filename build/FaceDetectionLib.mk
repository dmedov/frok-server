# Initialize make tools
include makefile.consts

SRCDIR = lib-common

# Target settings
LANG := c++
TARGET := FaceDetectionLib
TARGETTYPE := staticlib

CFLAGS += -Wno-unknown-pragmas

SRCDIRS :=  ../$(SRCDIR)/

HDRDIRS :=  ../$(SRCDIR)/

# Automated target make procedures: designed to fit any product
include makefile.actions

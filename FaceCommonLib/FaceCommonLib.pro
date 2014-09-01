QT -= core gui
CONFIG -= qt

TARGET = FaceCommonLib
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=  \
            TRACE_DEBUG

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

INCLUDEPATH +=              ./linux

HEADERS +=                  \
    json.h                  \
    commonThread.h          \
    faceCommonLib.h \
    commonMath.h \
    linux/io.h \
    linux/commonSched.h \
    linux/linuxDefines.h

SOURCES +=                  \
    json.cpp                \
    commonThread.cpp        \
    faceCommonLib.c \
    io.c \
    linux/io.c \
    linux/commonSched.c


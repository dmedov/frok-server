QT -= core gui
CONFIG -= qt

TARGET = lib-common
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=  \
            NETWORK_SYSTEM_DEBUG_PRINT_ENABLED

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

INCLUDEPATH +=                                          \
    /opt/opencv-2.4.9/install/include/opencv/           \
    /opt/opencv-2.4.9/install/include/opencv2/          \

HEADERS +=                  \
    ViolaJonesDetection.h   \
    stdafx.h                \
    network.h               \
    LibInclude.h            \
    json.h                  \
    DescriptorDetection.h   \
    common.h                \
    activities.h            \
    EigenDetector.h \
    commonThread.h

SOURCES +=                  \
    ViolaJonesDetection.cpp \
    network.cpp             \
    json.cpp                \
    DescriptorDetection.cpp \
    common.cpp              \
    activities.cpp          \
    EigenDetector.cpp \
    commonThread.cpp
    /opt/opencv-2.4.9/include/opencv2/                  \

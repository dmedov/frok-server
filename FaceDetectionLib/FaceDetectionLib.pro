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
    /opt/opencv-2.4.9/include/opencv/                   \
    /opt/opencv-2.4.9/include/opencv2/                  \
    /opt/opencv-2.4.9/modules/core/include/             \
    /opt/opencv-2.4.9/modules/imgproc/include/          \
    /opt/opencv-2.4.9/modules/imgproc/include/          \
    /opt/opencv-2.4.9/modules/video/include             \
    /opt/opencv-2.4.9/modules/features2d/include        \
    /opt/opencv-2.4.9/modules/flann/include             \
    /opt/opencv-2.4.9/modules/androidcamera/include     \
    /opt/opencv-2.4.9/modules/calib3d/include           \
    /opt/opencv-2.4.9/modules/contrib/include           \
    /opt/opencv-2.4.9/modules/dynamicuda/include        \
    /opt/opencv-2.4.9/modules/gpu/include               \
    /opt/opencv-2.4.9/modules/highgui/include           \
    /opt/opencv-2.4.9/modules/java/include              \
    /opt/opencv-2.4.9/modules/legacy/include            \
    /opt/opencv-2.4.9/modules/ml/include                \
    /opt/opencv-2.4.9/modules/nonfree/include           \
    /opt/opencv-2.4.9/modules/objdetect/include         \
    /opt/opencv-2.4.9/modules/ocl/include               \
    /opt/opencv-2.4.9/modules/photo/include             \
    /opt/opencv-2.4.9/modules/python/include            \
    /opt/opencv-2.4.9/modules/stitching/include         \
    /opt/opencv-2.4.9/modules/superres/include          \
    /opt/opencv-2.4.9/modules/ts/include                \
    /opt/opencv-2.4.9/modules/videostab/include         \
    /opt/opencv-2.4.9/modules/viz/include               \
    /opt/opencv-2.4.9/modules/world/include

HEADERS +=                  \
    ViolaJonesDetection.h   \
    stdafx.h                \
    network.h               \
    LibInclude.h            \
    json.h                  \
    DescriptorDetection.h   \
    common.h                \
    activities.h            \
    EigenDetector.h

SOURCES +=                  \
    ViolaJonesDetection.cpp \
    network.cpp             \
    json.cpp                \
    DescriptorDetection.cpp \
    common.cpp              \
    activities.cpp          \
    EigenDetector.cpp

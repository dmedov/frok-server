QT -= core gui
CONFIG -= qt

TARGET = lib-common
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=  \
            TRACE_DEBUG

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

DEPENDPATH += ../FrokLibCommon/

INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv/
INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv2/
INCLUDEPATH += /opt/opencv-2.4.9/install/include
INCLUDEPATH += ../FrokLibCommon/

HEADERS +=                              \
    network.h                           \
    LibInclude.h                        \
    json.h                              \
    DescriptorDetection.h               \
    common.h                            \
    activities.h                        \
    EigenDetector.h                     \
    commonThread.h                      \
    FrokFaceDetector.h \
    FaceDetectorAbstarct.h

SOURCES +=                              \
    network.cpp                         \
    json.cpp                            \
    DescriptorDetection.cpp             \
    common.cpp                          \
    activities.cpp                      \
    EigenDetector.cpp                   \
    commonThread.cpp                    \
    FrokFaceDetector.cpp \
    FaceDetectorAbstract.cpp

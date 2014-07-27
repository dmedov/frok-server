QT -= core gui
CONFIG -= qt

TARGET = FrokAPILib
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=  \
            NETWORK_SYSTEM_DEBUG_PRINT_ENABLED

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

DEPENDPATH += ../FaceCommonLib/
DEPENDPATH += ../FaceDetectionLib/
DEPENDPATH += ../FaceRecognitionLib/

INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv/
INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv2/
INCLUDEPATH += /opt/opencv-2.4.9/install/include
INCLUDEPATH += ../FaceCommonLib/
INCLUDEPATH += ../FaceDetectionLib/
INCLUDEPATH += ../FaceRecognitionLib/

HEADERS +=                  \
    FrokAPIFunction.h \
    FrokAPI.h

SOURCES +=                  \
    FrokAPIFunction.cpp \
    FrokAPI.cpp


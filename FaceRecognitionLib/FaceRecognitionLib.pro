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

DEPENDPATH += ../FaceCommonLib/

INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv/
INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv2/
INCLUDEPATH += /opt/opencv-2.4.9/install/include
INCLUDEPATH += ../FaceCommonLib/

HEADERS +=                  \
    EigenDetector.h         \
    FaceUserModel.h         \
    FaceRecognizer.h

SOURCES +=                  \
    EigenDetector.cpp       \
    FaceUserModel.cpp


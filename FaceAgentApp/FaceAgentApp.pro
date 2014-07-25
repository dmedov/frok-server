QT -= core gui
CONFIG -= qt

TARGET = FaceDetectionApp
TEMPLATE = app

DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=

DEPENDPATH += ../FaceCommonLib/
DEPENDPATH += ../FaceDetectionLib/
DEPENDPATH += ../FaceRecognitionLib/

INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv/
INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv2/
INCLUDEPATH += /opt/opencv-2.4.9/install/include
INCLUDEPATH += ../FaceCommonLib/
INCLUDEPATH += ../FaceDetectionLib
INCLUDEPATH += ../FaceRecognitionLib

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

HEADERS +=          \
    activities.h \
    FrokAPI.h \
    FaceAgent.h \
    FrokAPI.h

SOURCES +=          \
    main.cpp        \
    activities.cpp \
    FaceAgent.cpp \
    FrokAPI.cpp

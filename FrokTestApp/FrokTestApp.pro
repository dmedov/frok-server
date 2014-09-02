QT -= core gui
CONFIG -= qt

TARGET = FaceTestApp
TEMPLATE = app

DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=

DEPENDPATH += ../FrokLibCommon/
DEPENDPATH += ../FrokAPILib/

INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv/
INCLUDEPATH += /opt/opencv-2.4.9/install/include/opencv2/
INCLUDEPATH += /opt/opencv-2.4.9/install/include
INCLUDEPATH += ../FrokLibCommon/
INCLUDEPATH += ../FaceDetectionLib/
INCLUDEPATH += ../FaceRecognitionLib/
INCLUDEPATH += ../FrokAPILib

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

HEADERS +=          \

SOURCES +=          \
    main.cpp        

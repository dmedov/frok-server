QT -= core gui
CONFIG -= qt

TARGET = FrokDetectionApp
TEMPLATE = app

DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=  \
            TRACE_DEBUG

DEPENDPATH += ../FaceCommonLib/
DEPENDPATH += ../FrokJsonlib/

INCLUDEPATH += ../FaceCommonLib/
INCLUDEPATH += ../FrokJsonlib/

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas

HEADERS +=          \
    FrokServer.h    \
    FrokAgentConnector.h

SOURCES +=          \
    main.cpp        \
    FrokServer.cpp  \
    FrokAgentConnector.cpp

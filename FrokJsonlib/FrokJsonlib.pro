QT -= core gui
CONFIG -= qt

TARGET = FrokJsonlib
TEMPLATE = lib
CONFIG += staticlib
DESTDIR = ./bin
OBJECTS_DIR = ./obj

OBJECTS_DIR = $$OBJECTS_DIR/$$TARGET

DEFINES +=  \

QMAKE_CXXFLAGS +=   -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas
QMAKE_CFLAGS_DEBUG +=     -Werror -Wall -Wno-unused-function -Wno-write-strings -Wno-unused-result -Wno-unknown-pragmas


HEADERS +=                  \
    json.h

SOURCES +=                          \
    json.cpp


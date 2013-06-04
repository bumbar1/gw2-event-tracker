#-------------------------------------------------
#
# Project created by QtCreator 2013-05-29T05:56:37
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gw2-event-tracker
TEMPLATE = app


SOURCES += \
    tracker.cpp \
    main.cpp

HEADERS  += \
    tracker.hpp \
    event.hpp \
    label.hpp

CONFIG += c++11 static

QMAKE_CXXFLAGS += -std=c++11 -static-libgcc -static-libstdc++
QMAKE_LFLAGS += -static-libgcc -static-libstdc++

RESOURCES += \
    resources.qrc

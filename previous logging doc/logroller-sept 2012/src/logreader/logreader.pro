#-------------------------------------------------
#
# Project created by QtCreator 2012-11-05T21:42:08
#
#-------------------------------------------------

QT       += core gui

TARGET = logreader
TEMPLATE = app
CONFIG += console


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

LIBS += -lelf

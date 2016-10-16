TARGET = logreader
CONFIG -= qt

win32 {
    TEMPLATE = vcapp
    CONFIG += console
} else {
    TEMPLATE = app
}

SOURCES += logreader.cpp

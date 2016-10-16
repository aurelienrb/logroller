TARGET = exp3
CONFIG -= qt

win32 {
    TEMPLATE = vcapp
    CONFIG += console
} else {
    TEMPLATE = app
}

SOURCES += exp3.cpp

TARGET = exp2
CONFIG -= qt

win32 {
    TEMPLATE = vcapp
    CONFIG += console
} else {
    TEMPLATE = app
}

SOURCES += exp2.cpp

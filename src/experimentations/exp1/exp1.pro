TARGET = exp1
CONFIG -= qt

win32 {
    TEMPLATE = vcapp
    CONFIG += console
} else {
    TEMPLATE = app
}

SOURCES += exp1.cpp

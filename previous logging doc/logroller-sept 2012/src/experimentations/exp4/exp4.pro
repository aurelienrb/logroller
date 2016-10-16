TARGET = exp4
CONFIG -= qt

win32 {
    TEMPLATE = vcapp
    CONFIG += console
} else {
    TEMPLATE = app
}

SOURCES += exp4.cpp

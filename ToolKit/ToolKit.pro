#-------------------------------------------------
#
# Project created by QtCreator 2014-04-28T08:50:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ToolKit
TEMPLATE = app

win32 {
    TEMPLATE = vcapp
}

SOURCES += Source/main.cpp\
        Source/MainWindow.cpp

HEADERS  += Source/MainWindow.h

FORMS    += Source/MainWindow.ui

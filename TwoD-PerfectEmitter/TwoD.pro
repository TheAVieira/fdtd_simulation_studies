#-------------------------------------------------
#
# Project created by QtCreator 2016-06-09T11:48:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TwoD
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

win32: LIBS += -LC:/Qwt-6.1.2/lib/ -lqwt

INCLUDEPATH += C:/Qwt-6.1.2/include
DEPENDPATH += C:/Qwt-6.1.2/include


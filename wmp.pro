#-------------------------------------------------
#
# Project created by QtCreator 2012-08-29T18:52:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wmp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    histogramwidget.cpp

HEADERS  += mainwindow.h \
    histogramwidget.h

FORMS    += mainwindow.ui

RESOURCES += \
    assets.qrc

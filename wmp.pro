#-------------------------------------------------
#
# Project created by QtCreator 2012-08-29T18:52:43
#
#-------------------------------------------------

QT       += core gui application

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wmp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    histogramwidget.cpp \
    application.cpp \
    player.cpp

HEADERS  += mainwindow.h \
    histogramwidget.h \
    application.h \
    player.h

FORMS    += mainwindow.ui

RESOURCES += \
    assets.qrc

macx {
    QMAKE_LFLAGS += -framework Cocoa
    ICON = wmp.icns
    OBJECTIVE_SOURCES += macsupport.mm
    HEADERS += macsupport.h
}

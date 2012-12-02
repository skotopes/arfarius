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
    histogramwidget.cpp \
    application.cpp \
    RtAudio.cpp \
    player.cpp \
    playlist.cpp \
    playlistitem.cpp \
    avfile.cpp \
    avthread.cpp \
    avmutex.cpp \
    avcondition.cpp \
    avring.cpp

HEADERS  += mainwindow.h \
    histogramwidget.h \
    application.h \
    RtAudio.h \
    RtError.h \
    player.h \
    playlist.h \
    playlistitem.h \
    avfile.h \
    memring.h \
    avexception.h \
    avthread.h \
    avmutex.h \
    avcondition.h \
    avring.h

FORMS    += mainwindow.ui

RESOURCES += \
    assets.qrc

LIBS += -lavformat -lavutil -lavcodec -lswresample -ltag
#INCLUDEPATH += /usr/local/include/taglib

macx {
    LIBS += -framework Cocoa -framework CoreAudio -lpthread
    ICON = wmp.icns
    OBJECTIVE_SOURCES += macsupport.mm
    HEADERS += macsupport.h
    DEFINES += __MACOSX_CORE__
}

OTHER_FILES += \
    README.md

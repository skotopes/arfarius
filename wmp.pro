QT       += core gui concurrent svg opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = wmp
CONFIG += c++11

SOURCES += RtAudio.cpp \
    avcondition.cpp \
    avfile.cpp \
    avhistogram.cpp \
    avmutex.cpp \
    avobject.cpp \
    avring.cpp \
    avspectrogram.cpp \
    avsplitter.cpp \
    avthread.cpp \
    histogramwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    player.cpp \
    playlistitem.cpp \
    playlistmodel.cpp \
    playlistview.cpp \
    collection.cpp \
    wmpapplication.cpp

HEADERS  += RtAudio.h \
    RtError.h \
    avcondition.h \
    avconf.h \
    avexception.h \
    avfile.h \
    avhistogram.h \
    avmutex.h \
    avobject.h \
    avring.h \
    avspectrogram.h \
    avsplitter.h \
    avthread.h \
    histogramwidget.h \
    mainwindow.h \
    memring.h \
    player.h \
    playlistitem.h \
    playlistmodel.h \
    playlistview.h \
    collection.h \
    wmpapplication.h

FORMS    += mainwindow.ui

RESOURCES += \
    assets.qrc

LIBS += -lavformat -lavutil -lavcodec -lswresample -ltag -ltcejdb

macx {
    LIBS += -L /usr/local/lib -framework Cocoa -framework CoreAudio -lpthread
    ICON = wmp.icns
    OBJECTIVE_SOURCES += macsupport.mm
    HEADERS += macsupport.h
    DEFINES += __MACOSX_CORE__
    INCLUDEPATH += /usr/local/include/
    QMAKE_INFO_PLIST = wmp.plist
}

linux {
}

win {
}

OTHER_FILES += \
    README.md \
    wmp.plist

cache()

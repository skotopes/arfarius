include(QtAwesome/QtAwesome.pri)

QT += core gui widgets concurrent opengl

TEMPLATE = app
TARGET = Arfarius
CONFIG += c++11

SOURCES += \
    avcondition.cpp \
    avfile.cpp \
    avhistogram.cpp \
    avmutex.cpp \
    avobject.cpp \
    avring.cpp \
    avsplitter.cpp \
    avthread.cpp \
    histogramwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    player.cpp \
    playlistitem.cpp \
    playlistmodel.cpp \
    playlistview.cpp \
    arfariusapplication.cpp \
    qcoreaudio.cpp \
    avspectrum.cpp \
    collection.cpp

HEADERS  += \
    avcondition.h \
    avconf.h \
    avexception.h \
    avfile.h \
    avhistogram.h \
    avmutex.h \
    avobject.h \
    avring.h \
    avsplitter.h \
    avthread.h \
    histogramwidget.h \
    mainwindow.h \
    memring.h \
    player.h \
    playlistitem.h \
    playlistmodel.h \
    playlistview.h \
    arfariusapplication.h \
    qcoreaudio.h \
    avspectrum.h \
    collection.h \
    macmediakeys.h

FORMS    += mainwindow.ui

RESOURCES += assets.qrc

LIBS += -lavformat -lavutil -lavcodec -lswresample -ltag -lfftw3f

macx {
    LIBS += -L /usr/local/lib -framework Cocoa -framework CoreAudio -framework AudioUnit
    ICON = Arfarius.icns
    OBJECTIVE_SOURCES +=
    HEADERS +=
    DEFINES += __STDC_CONSTANT_MACROS
    INCLUDEPATH += /usr/local/include/
    QMAKE_INFO_PLIST = Arfarius.plist
    OTHER_FILES += Arfarius.plist
}

OTHER_FILES += \
    README.md

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Os

OBJECTIVE_SOURCES += \
    macmediakeys.mm

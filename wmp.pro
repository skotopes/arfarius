QT       += core gui widgets concurrent opengl

TEMPLATE = app
TARGET = wmp
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
    wmpapplication.cpp \
    qcoreaudio.cpp \
    avspectrum.cpp

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
    wmpapplication.h \
    qcoreaudio.h \
    avspectrum.h

FORMS    += mainwindow.ui

RESOURCES += assets.qrc

LIBS += -lavformat -lavutil -lavcodec -lswresample -ltag

macx {
    LIBS += -L /usr/local/lib -framework Cocoa -framework CoreAudio -framework AudioUnit -lfftw3f
    ICON = wmp.icns
    OBJECTIVE_SOURCES += macsupport.mm
    HEADERS += macsupport.h
    DEFINES += __STDC_CONSTANT_MACROS
    INCLUDEPATH += /usr/local/include/
    QMAKE_INFO_PLIST = wmp.plist
    OTHER_FILES += wmp.plist
}

OTHER_FILES += \
    README.md

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Os

cache()

cmake_minimum_required(VERSION 3.16)
project(Arfarius VERSION 0.1 LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets Concurrent Svg)

find_library(AVFOUNDATION_LIBRARY AvFoundation REQUIRED)
find_library(FFTW3F_LIBRARY fftw3f REQUIRED)
find_library(TAG_LIBRARY tag REQUIRED)

# FFmpeg libraries
find_library(AVFORMAT_LIBRARY avformat REQUIRED)
find_library(AVUTIL_LIBRARY avutil REQUIRED)
find_library(AVCODEC_LIBRARY avcodec REQUIRED)
find_library(SWRESAMPLE_LIBRARY swresample REQUIRED)
# FFMpeg needs it
add_compile_definitions(__STDC_CONSTANT_MACROS)

# Sources
set(ARFARIUS_SOURCES
    src/avfile.cpp
    src/avhistogram.cpp
    src/avmutex.cpp
    src/avobject.cpp
    src/avsplitter.cpp
    src/histogramwidget.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/player.cpp
    src/playlistitem.cpp
    src/playlistmodel.cpp
    src/playlistview.cpp
    src/arfariusapplication.cpp
    src/qcoreaudio.cpp
    src/avspectrum.cpp
    src/macmediakeys.mm
    QtAwesome/QtAwesome.cpp
    QtAwesome/QtAwesomeAnim.cpp
)

set(ARFARIUS_HEADERS
    src/avconf.h
    src/avexception.h
    src/avfile.h
    src/avhistogram.h
    src/avmutex.h
    src/avobject.h
    src/avsplitter.h
    src/histogramwidget.h
    src/mainwindow.h
    src/memring.h
    src/player.h
    src/playlistitem.h
    src/playlistmodel.h
    src/playlistview.h
    src/arfariusapplication.h
    src/qcoreaudio.h
    src/avspectrum.h
    src/macmediakeys.h
    QtAwesome/QtAwesome.h
    QtAwesome/QtAwesomeAnim.h
)

# Resources
set(ARFARIUS_RESOURCES
    QtAwesome/QtAwesome.qrc
)

# Executable
add_executable(${PROJECT_NAME}
    ${ARFARIUS_SOURCES}
    ${ARFARIUS_HEADERS}
    ${ARFARIUS_RESOURCES}
    Arfarius.icns
)

# Link
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Concurrent
    Qt6::Svg
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${SWRESAMPLE_LIBRARY}
    ${TAG_LIBRARY}
    ${FFTW3F_LIBRARY}
)

# Mac-specific
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Arfarius.plist"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "Arfarius.icns"
    )
    set_source_files_properties(Arfarius.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AVFoundation"
        "-framework MediaPlayer"
    )
endif()

# Include directories (for FFmpeg headers)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    /opt/homebrew/include
)

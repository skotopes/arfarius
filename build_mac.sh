#!/bin/bash

set -e
set -x

PROJECT_DIR=`pwd`
PROJECT="Arfarius"
BUILD_DIRECTORY="build_mac"

if [[ -d "$BUILD_DIRECTORY" ]]
then
	rm -rf "$BUILD_DIRECTORY"
fi

mkdir "$BUILD_DIRECTORY"
cd "$BUILD_DIRECTORY"

qmake -spec macx-clang CONFIG+=release CONFIG+=x86_64 -o Makefile ../$PROJECT.pro
make -j9 > /dev/null
macdeployqt $PROJECT.app -verbose=1

# libavcodec
install_name_tool -change /usr/local/Cellar/ffmpeg/4.4_2/lib/libswresample.3.dylib @executable_path/../Frameworks/libswresample.3.dylib $PROJECT.app/Contents/Frameworks/libavcodec.58.dylib
install_name_tool -change /usr/local/Cellar/ffmpeg/4.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib $PROJECT.app/Contents/Frameworks/libavcodec.58.dylib
# libavformat
install_name_tool -change /usr/local/Cellar/ffmpeg/4.4_2/lib/libavcodec.58.dylib @executable_path/../Frameworks/libavcodec.58.dylib $PROJECT.app/Contents/Frameworks/libavformat.58.dylib
install_name_tool -change /usr/local/Cellar/ffmpeg/4.4_2/lib/libswresample.3.dylib @executable_path/../Frameworks/libswresample.3.dylib $PROJECT.app/Contents/Frameworks/libavformat.58.dylib
install_name_tool -change /usr/local/Cellar/ffmpeg/4.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib $PROJECT.app/Contents/Frameworks/libavformat.58.dylib
# libhogweed
install_name_tool -change /usr/local/Cellar/nettle/3.7.3/lib/libnettle.8.dylib @executable_path/../Frameworks/libnettle.8.dylib $PROJECT.app/Contents/Frameworks/libhogweed.6.dylib
# libswresample
install_name_tool -change /usr/local/Cellar/ffmpeg/4.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib $PROJECT.app/Contents/Frameworks/libswresample.3.dylib
# libfftw
install_name_tool -change /usr/local/lib/gcc/11/libgcc_s.1.dylib @executable_path/../Frameworks/libgcc_s.1.dylib $PROJECT.app/Contents/Frameworks/libfftw3f.3.dylib

FAILED_LIBS_COUNT=`otool -L $PROJECT.app/Contents/Frameworks/*.dylib | grep /usr/local -c || true`

if [[ $FAILED_LIBS_COUNT -gt 0 ]]
then
	echo "Not all libraries use proper paths"
	exit 255
fi

# Sign
if [ -n "$SIGNING_KEY" ]
then
	xattr -cr $PROJECT.app
	codesign --force -s "$SIGNING_KEY" --deep -v $PROJECT.app
fi

# build DMG
mkdir disk_image
ln -s /Applications disk_image/Applications
mv $PROJECT.app disk_image
hdiutil create -volname Arfarius -srcfolder disk_image -ov -format UDZO Arfarius.dmg

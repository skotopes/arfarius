# About

Arfarius - ultra-minimalistic audio player for MacOS written in C++ with Qt.

# Features

- Made by me for me
- Plays absolutely everything that can be played
- Spectrohistogram for easier track navigation
- In-line tags editing (alt+click)
- Built for MacOS: uses coreaudio, supports media keys
- Lightweight (as much as Qt and FFmpeg allows)

# ToDo

- Playlist item rearranging
- Collection data store
- Better UI
- Get rid of Qt
- Minimize FFmpeg build: only audio codecs and containers

# Download

Latest version: https://github.com/skotopes/arfarius/releases/latest

# Requirements

Install with brew: `brew install taglib ffmpeg fftw cmake qtbase qtsvg`

# Compilation

Build and execute debug version:

`make CMAKE_BUILD_TYPE=Debug && ./build/Arfarius.app/Contents/MacOS/Arfarius`


Build and execute release version:

`make && ./build/Arfarius.app/Contents/MacOS/Arfarius`

Build release and pack dmg:

`make macdeploy`

# About

Arfarius - minimalistic audio player written in c++ with Qt.

# Features

- Made by me for me
- Plays everything that can be played (backed by ffmpeg)
- Async and cache throw ffmpeg plugins
- Histograms with spectrograms
- In-line tags editing (alt+click)
- Made for Mac OS: directly uses core audio, supports media keys
- Lightweight (as much as Qt allows)

# ToDo

- Playlist item rearranging
- Random playing
- Analyzer Thread Pools
- Metadata store / Collection


# Download

Latest version: https://github.com/skotopes/arfarius/releases/latest

# Building from source

Requirements:

- ffmpeg - containers and codecs
- taglib - work with tags
- fftw - FFT for spectrum functions

Type following command if you are using brew:

```
brew install taglib ffmpeg fftw
```

And complete build with qmake.

#About

Arfarius - minimalistic audio player written in c++ with Qt.

Main features:

* plays everything that can be played
* histograms with spectrograms
* in-line tags editing (alt+click)
* made for Mac OS: directly uses core audio, supports media keys
* quite good audio pipeline

#Download

Latest version for Mac OS: http://arfarius.com/Arfarius.dmg
Requirements: Mac OS 10.9 or higher

#Building from source

Requirements:

* ffmpeg - containers and codecs
* taglib - work with tags
* fftw - FFT for spectrum function

Type following command if you are using brew:

```
brew install taglib ffmpeg fftw
```

And complete build with qmake.

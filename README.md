About
============

Arfarius - minimalistic audio player written in c++ with Qt.

Main features:

* plays almost everything that can be played
* histograms with spectrograms
* in-line tags editing

Download
============

Latest version for Mac OS: http://myau.su/Arfarius.dmg
Requirements: Mac OS 10.9 or higher

Building from source
============

Requirments:

* ffmpeg
* taglib
* ejdb
* fftw

Type following commands if you are using brew:

```
brew install taglib
brew install ffmpeg --with-libvorbis --with-tools
brew install ejdb
brew install fftw
```

And complete build with qmake.

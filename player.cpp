#include "player.h"

Player::Player(QObject *parent) :
    QObject(parent)
{
}

Player::~Player()
{
}

int Player::callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
                    double streamTime, RtAudioStreamStatus status, void *userData )
{
    Player *me = reinterpret_cast<Player *>(userData);
    double *buffer = reinterpret_cast<double *>(outputBuffer);

    return 0;
}

void Player::openStream()
{
    if ( dac.getDeviceCount() < 1 ) {
        // todo show error dialog
        return;
    }

    // TODO: load it from setting please
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 2;
    parameters.firstChannel = 0;

    sampleRate = 44100;
    bufferFrames = 256;
}

void Player::startStream()
{
    try {
      dac.openStream( &parameters, NULL, RTAUDIO_FLOAT64,
                      sampleRate, &bufferFrames, &Player::callback, reinterpret_cast<void *>(this) );
      dac.startStream();
    }
    catch ( RtError& e ) {
      e.printMessage();
      // todo show error dialog
      return;
    }

}

void Player::stopStream()
{
    try {
      // Stop the stream
      dac.stopStream();
    }
    catch (RtError& e) {
      e.printMessage();
    }
}

void Player::closeStream()
{
    if (dac.isStreamOpen())
        dac.closeStream();
}

void Player::playPause()
{
}

void Player::stop()
{
}

void Player::next()
{

}

void Player::prev()
{

}

#include "avobject.h"
#include "avexception.h"

AVObject::AVObject():
    _input(0), _output(0), _sample_rate(0), _channels(0)
{
}

AVObject::~AVObject() {
    if (_output) _output->disconnectInput(this);
    if (_input) _input->disconnectOutput(this);
}

const char * AVObject::getRepr()
{
    return getName();
}

void AVObject::connectInput(AVObject *object, bool recursive)
{
    if (_input) {
        throw AVException("connectInput: already connected");
    }

    _input = object;
    if (recursive)
        object->connectOutput(this, false);
}

void AVObject::connectOutput(AVObject *object, bool recursive)
{
    if (_output)
        throw AVException("connectOutput: already connected");

    _output = object;
    if (recursive)
        object->connectInput(this, false);
}

void AVObject::disconnectInput(AVObject *object, bool recursive)
{
    if (_input != object)
        throw AVException("disconnectInput: programming error");

    _input = 0;
    if (recursive)
        object->disconnectOutput(this, false);
}

void AVObject::disconnectOutput(AVObject *object, bool recursive)
{
    if (_output != object)
        throw AVException("disconnectOutput: programming error");

    _output = 0;
    if (recursive)
        object->disconnectInput(this, false);
}

av_sample_rate_t AVObject::getSamplerate()
{
    return _sample_rate;
}

void AVObject::setSamplerate(av_sample_rate_t sample_rate) {
    _sample_rate = sample_rate;
    if (_output) {
        _output->setSamplerate(sample_rate);
    }
}

av_channels_t AVObject::getChannels()
{
    return _channels;
}

void AVObject::setChannels(av_channels_t channels)
{
    _channels = channels;
    if (_output) {
        _output->setChannels(channels);
    }
}

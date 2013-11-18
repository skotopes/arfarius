#include "avsplitter.h"
#include "avexception.h"

AVSplitter::AVSplitter() :
    _objects()
{
}

AVSplitter::~AVSplitter() {
}

size_t AVSplitter::pull(float */*buffer_ptr*/, size_t /*buffer_size*/)
{
    throw AVException("pull: not supported operation");
}

size_t AVSplitter::push(float *buffer_ptr, size_t buffer_size) {
    std::for_each(
        _objects.begin(), _objects.end(),
        [&](AVObject* obj) {
            obj->push(buffer_ptr, buffer_size);
        }
    );
    return buffer_size;
}

void AVSplitter::connectOutput(AVObject *object, bool recursive)
{
    std::for_each(
        _objects.begin(), _objects.end(),
        [&](AVObject* obj) {
            if (obj == object)
                throw AVException("connectOutput: already connected");
        }
    );

    _objects.push_back(object);

    if (recursive)
        object->connectInput(this, false);
}

void AVSplitter::disconnectOutput(AVObject *object, bool recursive)
{
    bool found = false;
    std::for_each(
        _objects.begin(), _objects.end(),
        [&](AVObject* obj) {
            if (obj == object)
                found = true;
        }
    );

    if (!found)
        throw AVException("disconnectOutput: programming error");

    if (recursive)
        object->disconnectInput(this, false);
}

void AVSplitter::setSamplerate(av_sample_rate_t sample_rate)
{
    _sample_rate = sample_rate;
    std::for_each(
        _objects.begin(), _objects.end(),
        [&](AVObject* obj) {
            obj->setSamplerate(sample_rate);
        }
    );
}

void AVSplitter::setChannels(av_channels_t channels)
{
    _channels = channels;
    std::for_each(
        _objects.begin(), _objects.end(),
        [&](AVObject* obj) {
            obj->setChannels(channels);
        }
    );
}

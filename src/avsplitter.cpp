#include "avsplitter.h"
#include "avexception.h"

AVSplitter::AVSplitter()
    : _objects() {
}

AVSplitter::~AVSplitter() {
    _objects.clear();
}

size_t AVSplitter::pull(float* /*buffer_ptr*/, size_t /*buffer_size*/) {
    throw AVException("pull: not supported operation");
}

size_t AVSplitter::push(float* buffer_ptr, size_t buffer_size) {
    if(!buffer_ptr) return 0;

    for(AVObject* obj : _objects) {
        if(obj) {
            obj->push(buffer_ptr, buffer_size);
        }
    }
    return buffer_size;
}

void AVSplitter::connectOutput(AVObject* object, bool recursive) {
    if(!object) {
        throw AVException("connectOutput: null object");
    }

    for(AVObject* obj : _objects) {
        if(obj == object) {
            throw AVException("connectOutput: already connected");
        }
    }

    _objects.push_back(object);

    if(recursive) {
        object->connectInput(this, false);
    }
}

void AVSplitter::disconnectOutput(AVObject* object, bool recursive) {
    auto it = std::find(_objects.begin(), _objects.end(), object);
    if(it == _objects.end()) throw AVException("disconnectOutput: programming error");

    if(recursive) object->disconnectInput(this, false);

    _objects.erase(it);
}

void AVSplitter::setSamplerate(av_sample_rate_t sample_rate) {
    _sample_rate = sample_rate;
    for(AVObject* obj : _objects) {
        if(obj) {
            obj->setSamplerate(sample_rate);
        }
    }
}

void AVSplitter::setChannels(av_channels_t channels) {
    _channels = channels;
    for(AVObject* obj : _objects) {
        if(obj) {
            obj->setChannels(channels);
        }
    }
}

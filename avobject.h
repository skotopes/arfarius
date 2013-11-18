#ifndef AVOBJECT_H
#define AVOBJECT_H

#include "avconf.h"

class AVObject
{
public:
    AVObject();
    virtual ~AVObject();

    AVObject(const AVObject&) = delete;
    AVObject& operator=(const AVObject&) = delete;

    virtual const char * getName() = 0;
    virtual const char * getRepr();

    virtual void connectInput(AVObject *object, bool recursive=true);
    virtual void connectOutput(AVObject *object, bool recursive=true);

    virtual void disconnectInput(AVObject *object, bool recursive=true);
    virtual void disconnectOutput(AVObject *object, bool recursive=true);

    virtual av_sample_rate_t getSamplerate();
    virtual void setSamplerate(av_sample_rate_t samplerate);

    virtual av_channels_t getChannels();
    virtual void setChannels(av_channels_t channels);

    virtual size_t pull(av_sample_t *buffer_ptr, size_t buffer_size) = 0;
    virtual size_t push(av_sample_t *buffer_ptr, size_t buffer_size) = 0;

protected:
    AVObject *_input;
    AVObject *_output;
    av_sample_rate_t _sample_rate;
    av_channels_t _channels;
};

#endif // AVOBJECT_H

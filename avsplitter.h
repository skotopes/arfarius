#ifndef AVSPLITTER_H
#define AVSPLITTER_H

#include "avobject.h"
#include <list>

class AVSplitter: public AVObject
{
public:
    AVSplitter();
    virtual ~AVSplitter();

    virtual const char * getName() { return "AVSplitter"; }

    virtual size_t pull(float *buffer_ptr, size_t buffer_size);
    virtual size_t push(float *buffer_ptr, size_t buffer_size);

    virtual void connectOutput(AVObject *object, bool recursive=true);
    virtual void disconnectOutput(AVObject *object, bool recursive=true);

    virtual void setSamplerate(av_sample_rate_t sample_rate);
    virtual void setChannels(av_channels_t channels);

private:
    std::list<AVObject*> _objects;
};

#endif // AVSPLITTER_H

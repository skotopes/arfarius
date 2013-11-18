#ifndef AVFILE_H
#define AVFILE_H

#include "avobject.h"

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;

class AVFile: public AVObject
{
public:
    AVFile();
    virtual ~AVFile();

    virtual const char * getName();
    virtual size_t pull(av_sample_t *buffer_ptr, size_t buffer_size);
    virtual size_t push(av_sample_t *buffer_ptr, size_t buffer_size);

    void open(const char *);
    void close();

    float getDurationInSeconds();
    size_t getDurationInSamples();
    float getPositionInSeconds();
    float getPositionInPercents();
    size_t getBitrate();

    size_t getCodecBitrate();
    int getCodecSamplerate();
    int getCodecChannels();

    void seekToPercent(float p);

    void decode();
    void abort();

private:
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    SwrContext *swrCtx;
    int audioStream;

    volatile bool _abort;
    volatile int64_t _position;
    volatile int64_t _seek_to;

    void _push(av_sample_t *buffer_ptr, size_t buffer_size);
    void _allocSWR();
};

#endif // AVFILE_H

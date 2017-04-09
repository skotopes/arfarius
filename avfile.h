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

    virtual void setSamplerate(av_sample_rate_t samplerate);
    virtual void setChannels(av_channels_t channels);

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

    void seekToPercent(float percent);
    void seekToSecond(float second);
    void seekBackward(float seconds);
    void seekForward(float seconds);

    void decode();
    void cancelDecoding();

private:
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    SwrContext *swrCtx;
    int audioStream;
    bool decoding;

    volatile int64_t _position;
    volatile int64_t _seek_to;

    void _updateSWR();
};

#endif // AVFILE_H

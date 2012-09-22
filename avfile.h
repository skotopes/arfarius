#ifndef AVFILE_H
#define AVFILE_H

#include <stddef.h>
#include "avcondition.h"
#include "avthread.h"

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;

template<typename T> class MemRing;


class AVFile : private AVThread
{
public:
    AVFile();
    virtual ~AVFile();

    void open(const char *);
    void startDecoder();
    inline bool isDecoderRunning() { return isRunning(); }
    void stopDecoder();
    void close();

    size_t getDuration();
    void seekToPosition(size_t p);
    void seekToPercent(float p);

    size_t pull(float * buffer, size_t size);

protected:
    void run();

private:
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    SwrContext *swrCtx;
    int audioStream;
    MemRing<float> *ring;
    AVCondition conditon;
    volatile bool do_shutdown;
    volatile int seek_to;

    void allocRing();
    void allocSWR();
    void fillRing(float * buffer, size_t size);
};

#endif // AVFILE_H

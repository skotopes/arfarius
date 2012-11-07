#ifndef AVFILE_H
#define AVFILE_H

#include <stddef.h>
#include <stdint.h>
#include "avcondition.h"
#include "avthread.h"

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;

template<typename T> class MemRing;


class AVFile : private AVThread
{
public:
    struct Progress {
        uint64_t    s_position; // postion, samples
        uint64_t    s_duration; // duraion, samples
        float       p_position; // percent position
        float       t_duration; // sec duration
    };

    AVFile();
    virtual ~AVFile();

    void open(const char *);
    void startDecoder();
    inline bool isDecoderRunning() { return isRunning(); }
    void stopDecoder();
    void close();

    size_t getDuration();
    float getPositionPercent();
    void seekToPositionPercent(float p);

    size_t pull(float * buffer, size_t size);
    bool isEOF() { return eof; }

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
    volatile bool eof;
    volatile int64_t position;
    volatile int seek_to;

    void allocRing();
    void allocSWR();
    void fillRing(float * buffer, size_t size);
};

#endif // AVFILE_H

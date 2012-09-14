#ifndef AVFILE_H
#define AVFILE_H

#include <stddef.h>

struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;

template<typename T> class MemRing;

class AVFile
{
public:
    AVFile();
    virtual ~AVFile();

    void open(const char *);
    void runDecoder();
    void close();

    size_t pull(float * buffer, size_t size);

private:
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    SwrContext *swrCtx;
    int audioStream;
    MemRing<float> *ring;

    void allocRing();
    void allocSWR();
};

#endif // AVFILE_H

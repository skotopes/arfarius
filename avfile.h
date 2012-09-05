#ifndef AVFILE_H
#define AVFILE_H

struct AVFormatContext;
struct AVCodecContext;

class AVFile
{
public:
    AVFile();
    virtual ~AVFile();

    void open(const char *);
    bool isAudio();

private:
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    int audioStream;
};

#endif // AVFILE_H

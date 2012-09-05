#ifndef AVFILE_H
#define AVFILE_H

#include <string>

struct AVFormatContext;
struct AVCodecContext;

class AVFile
{
public:
    AVFile();
    virtual ~AVFile();

    void open(std::string url);
    bool isAudio();

private:
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    int audioStream;
};

#endif // AVFILE_H

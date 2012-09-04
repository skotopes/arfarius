#include "avfile.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

static volatile bool ffmpeginit = false;

AVFile::AVFile()
{
    if (!ffmpeginit) {
       avcodec_register_all();
       av_register_all();
    }
}

AVFile::~AVFile()
{

}

void AVFile::open(std::string url)
{

}

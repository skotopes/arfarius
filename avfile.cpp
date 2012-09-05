#include "avfile.h"
#include "avexception.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <QDebug>

static volatile bool ffmpeginit = false;

AVFile::AVFile() :
    formatCtx(0), codecCtx(0), audioStream(-1)
{
    qDebug() << "AVFile: created" << this;
    if (!ffmpeginit) {
        av_register_all();
        avcodec_register_all();
        ffmpeginit = true;
    }
}

AVFile::~AVFile()
{
    close();
    qDebug() << "AVFile: destroyed" << this;
}

void AVFile::open(const char *url)
{
    if (formatCtx)
        throw AVException("Programming error: i already did it");

    if (avformat_open_input(&formatCtx, url, 0, 0) < 0)
        throw AVException("Unable to open media");

    if (avformat_find_stream_info(formatCtx, 0) < 0)
        throw AVException("Unable to find streams in media");

    for (unsigned int i=0; i<formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
            break;
        }
    }

    if (audioStream < 0)
        throw AVException("No audio stream found");

    codecCtx = formatCtx->streams[audioStream]->codec;
    AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);

    if (!codec)
        throw AVException("Could not find codec");

    if (avcodec_open2(codecCtx, codec, 0) < 0)
        throw AVException("Could not open codec");

}

void AVFile::close()
{
    if (codecCtx) {
        avcodec_close(codecCtx);
        codecCtx = 0;
    }

    if (formatCtx) {
        avformat_close_input(&formatCtx);
    }
}

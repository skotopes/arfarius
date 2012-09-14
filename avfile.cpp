#include "avfile.h"
#include "avexception.h"
#include "memring.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <QDebug>

static volatile bool ffmpeginit = false;

AVFile::AVFile() :
    formatCtx(0), codecCtx(0), swrCtx(0), audioStream(-1), ring(0)
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

    AVCodec *codec;
    audioStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (audioStream < 0)
        throw AVException("No audio stream found");

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx)
        throw AVException("Unable to allocate context");

    if (avcodec_open2(codecCtx, codec, 0) < 0)
        throw AVException("Could not open codec");

    allocRing();
    allocSWR();
}

void AVFile::runDecoder()
{
    int len = 0;
    int got_frame = 0;

    int packet_size;
    uint8_t *packet_data;

    AVFrame frame;
    AVPacket packet;

    while (av_read_frame(formatCtx, &packet) == 0) {
        if (packet.stream_index == audioStream) {
            packet_size = packet.size;
            packet_data = packet.data;
            while (packet.size > 0) {
                avcodec_get_frame_defaults(&frame);
                len = avcodec_decode_audio4(codecCtx, &frame, &got_frame, &packet);
                if (len < 0) {
                    break;
                }

                packet.data += len;
                packet.size -= len;

                if (got_frame) {
                    got_frame = 0;
                    // do some magic
                    int decoded_size = av_samples_get_buffer_size(NULL, codecCtx->channels, frame.nb_samples, codecCtx->sample_fmt, 1);
                    uint8_t *decoded_data = frame.data[0];
                    qDebug() << "Got frame:" << frame.nb_samples << " with size: " << decoded_size;
                }
            }
            packet.size = packet_size;
            packet.data = packet_data;
        }
        av_free_packet(&packet);
    }
}

void AVFile::close()
{
    // Free codec context
    if (codecCtx) {
        avcodec_close(codecCtx);
        av_freep(&codecCtx); // todo: check that context freed properly
        codecCtx = 0;
    }
    // Free format context and close file and so on
    if (formatCtx) {
        avformat_close_input(&formatCtx);
    }
    // free swr context
    if (swrCtx) {
        swr_free(&swrCtx);
    }
    // reset audio stream
    audioStream = -1;
    // free memory ring
    if (ring) {
        delete ring;
    }
    // object now ready to be reused
}

size_t AVFile::pull(float * buffer, size_t size)
{
    return ring->pull(buffer, size);
}

void AVFile::allocRing()
{
    ring = new MemRing<float>(44100 * 8 * codecCtx->channels);
    if (!ring)
        throw AVException("Unable to allocate ring");
}

void AVFile::allocSWR()
{
    swrCtx = swr_alloc_set_opts(0, av_get_default_channel_layout(2), AV_SAMPLE_FMT_FLT, 44100,
                       codecCtx->channel_layout, codecCtx->sample_fmt, codecCtx->sample_rate,
                       0, 0);
    if (!swrCtx)
        throw AVException("Unable to allocate swresample context");
    swr_init(swrCtx);
}

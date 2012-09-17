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
    formatCtx(0), codecCtx(0), swrCtx(0), audioStream(-1), ring(0), conditon()
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
    join();
    qDebug() << "AVFile: destroyed" << this;
}

void AVFile::open(const char *url)
{
    qDebug() << "AVFile: trying to open" << url;

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
}

void AVFile::startDecoder()
{
    create();
}

void AVFile::run()
{
    AVFrame frame;
    int got_frame;

    AVPacket packet;
    int packet_size;
    uint8_t *packet_data;

    DECLARE_ALIGNED(16,uint8_t,shadow)[AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];

    while (av_read_frame(formatCtx, &packet) == 0) {
        if (packet.stream_index == audioStream) {
            // make shure that we will be able to free it later
            packet_size = packet.size;
            packet_data = packet.data;

            // decode frames till packet contains data
            while (packet.size > 0) {
                avcodec_get_frame_defaults(&frame);
                int len = avcodec_decode_audio4(codecCtx, &frame, &got_frame, &packet);
                if (len < 0) {
                    break; // probably corrupted packet
                }

                packet.data += len;
                packet.size -= len;

                if (got_frame) {
                    got_frame = 0;

                    int decoded_size = av_samples_get_buffer_size(NULL, codecCtx->channels, frame.nb_samples, codecCtx->sample_fmt, 1);

                    // TODO: stream params can be changed on the fly, add moare checks
                    if (!swrCtx &&
                            codecCtx->channel_layout != av_get_default_channel_layout(2) ||
                            codecCtx->sample_fmt != AV_SAMPLE_FMT_FLT ||
                            codecCtx->sample_rate != 44100) {
                        allocSWR();
                    }

                    if (swrCtx) {
                        uint8_t *shadow_array[] = { shadow };
                        const uint8_t *input_array[] = { frame.data[0] };
                        // todo: check original code^ some nasty shit inside
                        int ret = swr_convert(swrCtx, shadow_array, AVCODEC_MAX_AUDIO_FRAME_SIZE, input_array, decoded_size);
                        if (ret > 0) {
                            fillRing(reinterpret_cast<float *>(shadow), ret*2/4);
                        }
                    } else {
                        fillRing(reinterpret_cast<float *>(frame.data[0]), decoded_size);
                    }
                }
            }

            // restore original size and pointer
            packet.size = packet_size;
            packet.data = packet_data;
        }

        // free packet data, reuse structure
        av_free_packet(&packet);
    }
}

void AVFile::close()
{
    if (codecCtx) {
        avcodec_close(codecCtx);
        av_freep(&codecCtx); // free context
        codecCtx = 0;
    }

    if (formatCtx) {
        avformat_close_input(&formatCtx);
    }

    if (swrCtx) {
        swr_free(&swrCtx);
    }

    audioStream = -1;

    if (ring) {
        delete ring;
    }
}

size_t AVFile::pull(float * buffer, size_t size)
{
    if (!ring)
        return 0;

    size_t ret = ring->pull(buffer, size);
    conditon.signal();
    return ret;
}

void AVFile::allocRing()
{
    ring = new MemRing<float>(44100 * 8 * 2);
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

void AVFile::fillRing(float * buffer, size_t size)
{
//    qDebug() << "AVFile: pushing data to the ring" << size
//             << "ring free space:" << ring->writeSpace()
//             << "ring used space:" << ring->readSpace();

    while (ring->writeSpace() < size) {
        conditon.lock();
        conditon.wait(); // magic
        conditon.unlock();
    }

    ring->push(buffer, size);
}

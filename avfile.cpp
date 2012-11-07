#include "avfile.h"
#include "avexception.h"
#include "memring.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <QDebug>

static volatile bool ffmpeginit = false;

AVFile::AVFile() :
    AVThread(), formatCtx(0), codecCtx(0), swrCtx(0), audioStream(-1), ring(0), conditon(),
    do_shutdown(false), eof(false), position(0), seek_to(-1)
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
    stopDecoder();
    join();
    close();
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

    codecCtx = formatCtx->streams[audioStream]->codec;
    if (avcodec_open2(codecCtx, codec, 0) < 0)
        throw AVException("Could not open codec");

    if (!codecCtx->channel_layout)
        codecCtx->channel_layout = av_get_default_channel_layout(codecCtx->channels);

    // TODO: stream params can be changed on the fly, add moare checks
    if (codecCtx->channel_layout != av_get_default_channel_layout(2) ||
            codecCtx->sample_fmt != AV_SAMPLE_FMT_FLT ||
            codecCtx->sample_rate != 44100) {
        allocSWR();
    }

    allocRing();
}

void AVFile::startDecoder()
{
    qDebug() << "AVFile::startDecoder()";
    create();
}

void AVFile::stopDecoder()
{
    qDebug() << "AVFile::stopDecoder()";
    do_shutdown = true;
    ring->reset();
    conditon.signal();
    join();
}

void AVFile::close()
{
    qDebug() << "AVFile::close()";
    if (codecCtx) {
        avcodec_close(codecCtx);
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

size_t AVFile::getDuration()
{
    return formatCtx->duration / AV_TIME_BASE;
}

float AVFile::getPositionPercent()
{
    qDebug() << "AVFile::getPositionPercent()" << position * codecCtx->time_base.num / codecCtx->time_base.den / 326;
    return ((float)position * codecCtx->time_base.num / codecCtx->time_base.den / 326 ) / (formatCtx->duration / AV_TIME_BASE);
}

void AVFile::seekToPositionPercent(float p)
{
    if (0 < p < 1)
        seek_to = formatCtx->duration * p;
}

size_t AVFile::pull(float * buffer, size_t size)
{
    if (!ring)
        return 0;

    size_t ret = ring->pull(buffer, size);
    conditon.signal();

    return ret;
}

// Protected
void AVFile::run()
{
    qDebug() << "AVFile::run()";
    eof = false;

    AVFrame frame;
    int got_frame;

    AVPacket packet;
    int packet_size;
    uint8_t *packet_data;
    av_init_packet(&packet);

    uint8_t * shadow = reinterpret_cast<uint8_t*>(av_malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE * 4));

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

                    if (swrCtx) {
                        uint8_t *shadow_array[] = { shadow };
                        const uint8_t *input_array[] = { frame.data[0] };
                        // todo: check original code^ some nasty shit inside
                        int ret = swr_convert(swrCtx, shadow_array, AVCODEC_MAX_AUDIO_FRAME_SIZE, input_array, frame.nb_samples);
                        if (ret > 0) {
                            fillRing(reinterpret_cast<float *>(shadow), ret*2);
                        }
                    } else {
                        fillRing(reinterpret_cast<float *>(frame.data[0]), frame.nb_samples * 2);
                    }

                    // update position
                    if (frame.pts != AV_NOPTS_VALUE)
                        position = frame.pts;
                    else if (packet.pts != AV_NOPTS_VALUE)
                        position = packet.pts;
                    else
                        position = 0;
                }
                // hurry up, no time to decode one more frame
                if (do_shutdown)
                    break;
            }

            // restore original size and pointer
            packet.size = packet_size;
            packet.data = packet_data;
        }
        // free packet data, reuse structure
        av_free_packet(&packet);
        // complete decoding thread shutdown
        if (do_shutdown) {
            do_shutdown = false;
            break;
        }

        if (seek_to > -1) {
            av_seek_frame(formatCtx, audioStream, seek_to * AV_TIME_BASE, 0);
            seek_to = -1;
        }
    }
    av_free(shadow);
    eof = true;
    qDebug() << "AVFile::run() done";
}

// Private
void AVFile::allocRing()
{
    qDebug() << "AVFile::allocRing()";
    ring = new MemRing<float>(44100 * 2);
    if (!ring)
        throw AVException("Unable to allocate ring");
}


void AVFile::allocSWR()
{
    qDebug() << "AVFile::allocSWR()" << codecCtx->channel_layout << codecCtx->sample_fmt << codecCtx->sample_rate;
    swrCtx = swr_alloc_set_opts(0, av_get_default_channel_layout(2), AV_SAMPLE_FMT_FLT, 44100,
                                codecCtx->channel_layout, codecCtx->sample_fmt, codecCtx->sample_rate,
                                0, 0);

    if (!swrCtx)
        throw AVException("Unable to allocate swresample context");

    swr_init(swrCtx);
}

void AVFile::fillRing(float * buffer, size_t size)
{
//    qDebug() << "AVFile::fillRing();";
    while (ring->writeSpace() < size) {
        conditon.lock();
        conditon.wait(); // magic
        conditon.unlock();
    }

    ring->push(buffer, size);
}

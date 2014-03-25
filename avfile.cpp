#include "avfile.h"
#include "avobject.h"
#include "avexception.h"
#include "memring.h"

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}


static volatile bool ffmpeginit = false;

AVFile::AVFile() :
    AVObject(),
    formatCtx(0), codecCtx(0), swrCtx(0), audioStream(-1),
    _abort(false), _position(0), _seek_to(-1)
{
    if (!ffmpeginit) {
        av_register_all();
        avcodec_register_all();
        ffmpeginit = true;
    }
}

AVFile::~AVFile()
{
    close();
}

const char * AVFile::getName() {
    return "AVFile";
}

size_t AVFile::pull(av_sample_t */*buffer_ptr*/, size_t /*buffer_size*/)
{
    return 0;
}

size_t AVFile::push(av_sample_t */*buffer_ptr*/, size_t /*buffer_size*/)
{
    return 0;
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

    codecCtx = formatCtx->streams[audioStream]->codec;
    if (avcodec_open2(codecCtx, codec, 0) < 0)
        throw AVException("Could not open codec");

    if (!codecCtx->channel_layout)
        codecCtx->channel_layout = av_get_default_channel_layout(codecCtx->channels);

    // TODO: stream params can be changed on the fly, add moar checks
    if (codecCtx->channel_layout != (uint64_t)av_get_default_channel_layout(_channels) ||
            codecCtx->sample_fmt != AV_SAMPLE_FMT_FLT ||
            codecCtx->sample_rate != (int)_sample_rate) {
        _allocSWR();
    }
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

    if (swrCtx) {
        swr_free(&swrCtx);
    }

    audioStream = -1;
}

float AVFile::getDurationInSeconds() {
    return (float) formatCtx->duration / AV_TIME_BASE;
}

size_t AVFile::getDurationInSamples() {
    return formatCtx->duration * _sample_rate / AV_TIME_BASE;
}

float AVFile::getPositionInSeconds() {
    AVStream * s = formatCtx->streams[audioStream];
    return (float) _position * s->time_base.num / s->time_base.den;
}

float AVFile::getPositionInPercents()
{
    return getPositionInSeconds() / getDurationInSeconds();
}

size_t AVFile::getBitrate() {
    return formatCtx->bit_rate;
}

size_t AVFile::getCodecBitrate() {
    return codecCtx->bit_rate;
}

int AVFile::getCodecSamplerate() {
    return codecCtx->sample_rate;
}

int AVFile::getCodecChannels() {
    return codecCtx->channels;
}

void AVFile::seekToPercent(float p)
{
    if (0. < p && p < 1.) {
        AVStream * s = formatCtx->streams[audioStream];
        _seek_to = av_rescale(p * formatCtx->duration, s->time_base.den, AV_TIME_BASE * s->time_base.num);
    }
}

// Protected
void AVFile::decode()
{
    AVFrame frame;
    int got_frame;

    AVPacket packet;
    int packet_size;
    uint8_t *packet_data;
    av_init_packet(&packet);

    uint8_t * shadow = reinterpret_cast<uint8_t*>(av_malloc(192000 * 4));

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
                        const uint8_t **input_array = (const uint8_t **)frame.extended_data;
                        // todo: check original code^ some nasty shit inside
                        int ret = swr_convert(swrCtx, shadow_array, 192000, input_array, frame.nb_samples);
                        if (ret > 0) {
                            _push(reinterpret_cast<float *>(shadow), ret * _channels);
                        }
                    } else {
                        _push(reinterpret_cast<float *>(frame.data[0]), frame.nb_samples * _channels);
                    }

                    // update position
                    if (frame.pts != AV_NOPTS_VALUE) {
                        _position = frame.pts;
                    } else if (packet.pts != AV_NOPTS_VALUE) {
                        _position = packet.pts;
                    } else {
                        _position = 0;
                    }
                }
                // hurry up, no time to decode one more frame
                if (_abort)
                    break;
            }

            // restore original size and pointer
            packet.size = packet_size;
            packet.data = packet_data;
        }
        // free packet data, reuse structure
        av_free_packet(&packet);
        // complete decoding thread shutdown
        if (_abort) {
            _abort = false;
            break;
        }

        if (_seek_to > -1) {
            int flags = AVSEEK_FLAG_ANY;
            if (_seek_to < _position)
                flags = flags | AVSEEK_FLAG_BACKWARD;
            av_seek_frame(formatCtx, audioStream, _seek_to, flags);
            _seek_to = -1;
        }
    }
    av_free(shadow);
}
void AVFile::abort()
{
    _abort = true;
}

void AVFile::_push(av_sample_t *buffer_ptr, size_t buffer_size)
{
    if (_output) {
        size_t ret;
        while (buffer_size > 0) {
            ret = _output->push(buffer_ptr, buffer_size);
            buffer_size -= ret;
            buffer_ptr += ret;
        }
    }
}

void AVFile::_allocSWR()
{
    swrCtx = swr_alloc_set_opts(0, av_get_default_channel_layout(_channels), AV_SAMPLE_FMT_FLT, _sample_rate,
                                codecCtx->channel_layout, codecCtx->sample_fmt, codecCtx->sample_rate,
                                0, 0);

    if (!swrCtx)
        throw AVException("Unable to allocate swresample context");

    swr_init(swrCtx);
}

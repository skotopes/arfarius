#include "avfile.h"
#include "avexception.h"

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <QDebug>
#define AVFILE_SHADOW_BUFFER_SIZE 192000

AVFile::AVFile()
    : AVObject()
    , formatCtx(nullptr)
    , codecCtx(nullptr)
    , swrCtx(nullptr)
    , audio_stream_id(-1)
    , decoding(false)
    , _position(0)
    , _seek_to(-1) {
}

AVFile::~AVFile() {
    close();
}

const char* AVFile::getName() {
    return "AVFile";
}

void AVFile::setSamplerate(av_sample_rate_t samplerate) {
    AVObject::setSamplerate(samplerate);
    _updateSWR();
}

void AVFile::setChannels(av_channels_t channels) {
    AVObject::setChannels(channels);
    _updateSWR();
}

size_t AVFile::pull(av_sample_t* /*buffer_ptr*/, size_t /*buffer_size*/) {
    return 0;
}

size_t AVFile::push(av_sample_t* /*buffer_ptr*/, size_t /*buffer_size*/) {
    return 0;
}

void AVFile::open(const char* url) {
    if(!url) throw AVException("Null URL");
    if(formatCtx) throw AVException("File already open");

    if(avformat_open_input(&formatCtx, url, nullptr, nullptr) < 0)
        throw AVException("Unable to open media");

    if(avformat_find_stream_info(formatCtx, nullptr) < 0)
        throw AVException("Unable to find streams in media");

    const AVCodec* codec = nullptr;
    audio_stream_id = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if(audio_stream_id < 0) throw AVException("No audio stream found");

    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx) throw AVException("Could not allocate codec context");

    if(avcodec_parameters_to_context(codecCtx, formatCtx->streams[audio_stream_id]->codecpar) <
       0) {
        throw AVException("Could not copy codec parameters from stream");
    }

    if(avcodec_open2(codecCtx, codec, nullptr) < 0) {
        throw AVException("Could not open codec");
    }

    _updateSWR();
}

void AVFile::close() {
    decoding.store(false);
    _seek_to.store(-1);

    if(codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
    }

    if(formatCtx) {
        avformat_close_input(&formatCtx);
        formatCtx = nullptr;
    }

    if(swrCtx) {
        swr_free(&swrCtx);
        swrCtx = nullptr;
    }

    audio_stream_id = -1;
}

float AVFile::getDurationInSeconds() {
    if(!formatCtx) return 0.0f;
    return (float)formatCtx->duration / AV_TIME_BASE;
}

size_t AVFile::getDurationInSamples() {
    if(!formatCtx) return 0;
    return formatCtx->duration * _sample_rate / AV_TIME_BASE;
}

float AVFile::getPositionInSeconds() {
    if(!formatCtx || audio_stream_id < 0) return 0.0f;
    AVStream* s = formatCtx->streams[audio_stream_id];
    if(s->time_base.den == 0) return 0.0f;
    return (float)_position.load() * s->time_base.num / s->time_base.den;
}

float AVFile::getPositionInPercents() {
    float duration = getDurationInSeconds();
    if(duration <= 0.0f || duration != duration) return 0.0f; // also guard NaN
    float pos = getPositionInSeconds();
    if(pos != pos) return 0.0f; // NaN guard
    return pos / duration;
}

size_t AVFile::getBitrate() {
    if(!formatCtx) return 0;
    return formatCtx->bit_rate;
}

size_t AVFile::getCodecBitrate() {
    if(!codecCtx) return 0;
    return codecCtx->bit_rate;
}

int AVFile::getCodecSamplerate() {
    if(!codecCtx) return 0;
    return codecCtx->sample_rate;
}

int AVFile::getCodecChannels() {
    if(!codecCtx) return 0;
    return codecCtx->ch_layout.nb_channels;
}

void AVFile::seekToPercent(float percent) {
    if(percent < 0.0f || percent > 1.0f) return;
    if(!formatCtx || formatCtx->duration <= 0) return;

    AVStream* s = formatCtx->streams[audio_stream_id];
    if(audio_stream_id < 0 || s->time_base.den == 0) return;

    int64_t target = av_rescale(
        percent * formatCtx->duration, s->time_base.den, AV_TIME_BASE * s->time_base.num);

    _seek_to.store(target);
}

void AVFile::seekToSecond(float second) {
    if(second < 0.0f) return;
    float dur = getDurationInSeconds();
    if(dur <= 0.0f) return;
    seekToPercent(second / dur);
}

void AVFile::seekBackward(float seconds) {
    seekToSecond(getPositionInSeconds() - seconds);
}

void AVFile::seekForward(float seconds) {
    seekToSecond(getPositionInSeconds() + seconds);
}

// Protected
void AVFile::decode() {
    AVFrame* frame = av_frame_alloc();
    if(!frame) return;

    AVPacket* packet = av_packet_alloc();
    if(!packet) {
        av_frame_free(&frame);
        return;
    }

    uint8_t* shadow =
        reinterpret_cast<uint8_t*>(av_malloc(AVFILE_SHADOW_BUFFER_SIZE * sizeof(float)));
    if(!shadow) {
        av_frame_free(&frame);
        av_packet_free(&packet);
        return;
    }
    decoding.store(true);

    while(av_read_frame(formatCtx, packet) == 0) {
        if(!decoding.load()) {
            break;
        }

        if(packet->stream_index == audio_stream_id) {
            // send packet to decoder
            int ret = avcodec_send_packet(codecCtx, packet);
            if(ret < 0) {
                av_packet_unref(packet);
                continue; // probably corrupted packet
            }

            // receive frames till decoder has no more output
            while(ret >= 0) {
                ret = avcodec_receive_frame(codecCtx, frame);
                if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                if(ret < 0) {
                    av_packet_unref(packet);
                    break; // error decoding
                }

                if(swrCtx) {
                    if(!frame->extended_data || !frame->extended_data[0]) {
                        av_frame_unref(frame);
                        break;
                    }

                    uint8_t* shadow_array[] = {shadow};
                    int converted = swr_convert(
                        swrCtx,
                        shadow_array,
                        AVFILE_SHADOW_BUFFER_SIZE,
                        frame->extended_data,
                        frame->nb_samples);
                    if(converted > 0 && _output) {
                        _output->push(reinterpret_cast<float*>(shadow), converted * _channels);
                    }
                } else {
                    if(_output) {
                        _output->push(
                            reinterpret_cast<float*>(frame->data[0]),
                            frame->nb_samples * _channels);
                    }
                }

                // update position
                if(frame->pts != AV_NOPTS_VALUE) {
                    _position.store(frame->pts);
                } else if(packet->pts != AV_NOPTS_VALUE) {
                    _position.store(packet->pts);
                } else {
                    _position.store(0);
                }

                av_frame_unref(frame);
                // hurry up, no time to decode one more frame
                if(!decoding.load()) {
                    break;
                }
            }
        }
        // free packet data, reuse structure
        av_packet_unref(packet);

        int64_t seek_target = _seek_to.exchange(-1);
        if(seek_target >= 0) {
            qDebug() << "Seek to" << seek_target;
            av_seek_frame(formatCtx, audio_stream_id, seek_target, AVSEEK_FLAG_ANY);
        }
    }

    av_frame_free(&frame);
    av_packet_free(&packet);
    av_free(shadow);
}

void AVFile::cancelDecoding() {
    decoding.store(false);
}

void AVFile::_updateSWR() {
    if(swrCtx) {
        swr_free(&swrCtx);
    }

    if(!_channels || !_sample_rate || !codecCtx) return;

    AVChannelLayout out_chlayout;
    av_channel_layout_default(&out_chlayout, _channels);

    if(codecCtx->ch_layout.nb_channels != (uint64_t)out_chlayout.nb_channels ||
       codecCtx->sample_fmt != AV_SAMPLE_FMT_FLT || codecCtx->sample_rate != (int)_sample_rate) {
        qDebug() << this << "_updateSWR():"
                 << "out:" << out_chlayout.nb_channels << AV_SAMPLE_FMT_FLT << _sample_rate
                 << "in:" << codecCtx->ch_layout.nb_channels << codecCtx->sample_fmt
                 << codecCtx->sample_rate;
        swrCtx = swr_alloc();
        if(!swrCtx) throw AVException("Unable to allocate swresample context");
        int ret = swr_alloc_set_opts2(
            &swrCtx,
            &out_chlayout,
            AV_SAMPLE_FMT_FLT,
            _sample_rate,
            &codecCtx->ch_layout,
            codecCtx->sample_fmt,
            codecCtx->sample_rate,
            0,
            nullptr);

        if(ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "Failed to set swr options: %d\n", ret);
            swr_free(&swrCtx);
            throw AVException("Unable to configure swresample context");
        }

        swr_init(swrCtx);
    }
}

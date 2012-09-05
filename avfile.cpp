#include "avfile.h"
#include "avexception.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

static volatile bool ffmpeginit = false;

AVFile::AVFile() :
    formatCtx(0), codecCtx(0), audioStream(-1)
{
    if (!ffmpeginit) {
       avcodec_register_all();
       av_register_all();
       ffmpeginit = true;
    }
}

AVFile::~AVFile()
{
    avformat_free_context(formatCtx);
}

void AVFile::open(std::string url)
{
    int ret;
    ret = avformat_open_input(&formatCtx, url.c_str(), 0, 0);
    if (ret < 0)
        throw new AVException("Unable to open media");

    for(unsigned int i=0; i<formatCtx->nb_streams; i++)
        if(formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream=i;
            break;
        }

    //    if(audioStream==-1)
//       return -3; // FFmpeg: audio stream not found

//    aCodecCtx=aFormatCtx->streams[audioStream]->codec;
//    aCodec=avcodec_find_decoder(aCodecCtx->codec_id);

//    if(aCodec==NULL)
//       return -4; // Unsupported codec

//    if(avcodec_open(aCodecCtx, aCodec)<0)
//       return -5; // Cannot open codec

//    o_buff->ctx.filename = aFormatCtx->filename;
//    o_buff->ctx.author = aFormatCtx->author;
//    o_buff->ctx.title = aFormatCtx->title;
//    o_buff->ctx.album = aFormatCtx->album;
//    o_buff->ctx.copyright = aFormatCtx->copyright;
//    o_buff->ctx.comment = aFormatCtx->comment;
//    o_buff->ctx.genre = aFormatCtx->genre;
//    o_buff->ctx.year = aFormatCtx->year;
//    o_buff->ctx.channels = aCodecCtx->channels;
//    o_buff->ctx.sample_rate = aCodecCtx->sample_rate;
//    o_buff->ctx.sample_fmt = aCodecCtx->sample_fmt;
//    o_buff->ctx.codec_name = aCodec->name;
//    o_buff->ctx.codec_name_long = aCodec->long_name;

//    if (aCodecCtx->sample_rate != outgoingSamplerate || aCodecCtx->channels != outgoingChannels) {
//       need_resampler = true;
//       aResample = av_audio_resample_init(outgoingChannels, aCodecCtx->channels,
//                              outgoingSamplerate, aCodecCtx->sample_rate,
//                              SAMPLE_FMT_S16, aCodecCtx->sample_fmt,
//                              16, 10, 0, 0.8);
//       if (!aResample)
//          return -6;
//    } else {
//       need_resampler = false;
//    }
}

bool AVFile::isAudio()
{
    return (audioStream == -1) ? false : true;
}

/*
 * @Author: your name
 * @Date: 2022-01-10 09:40:47
 * @LastEditTime: 2022-01-10 14:37:14
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/src/audio_mix.cpp
 */
#include "audio_mix.h"


AudioMix::AudioMix()
{
    std::vector<AVFrame*> af0 = AudioDecode("0.mp4");

    FILE* file = fopen("0.pcm", "wb");
    assert(NULL != file);
    for(auto iter = af0.begin(); iter != af0.end(); iter++)
    {
        size_t unpadded_linesize = (*iter)->nb_samples * av_get_bytes_per_sample((AVSampleFormat)(*iter)->format);
        fwrite((*iter)->extended_data[0], 1, unpadded_linesize, file);
        av_frame_free(&(*iter));
    }
    fclose(file);

    std::cout << "Finish!" << std::endl;
}

AudioMix::~AudioMix()
{
}

std::vector<AVFrame*> AudioMix::AudioDecode(const std::string& file)
{
    int ret = 0;
    AVPacket pkt = {0};
    AVCodec* codec = NULL;
    AVFormatContext* fmtCtx = NULL;
    AVCodecContext* codecCtx = NULL;
    int audioStreamIndex = -1;
    std::vector<AVFrame*> frames;

    ret = avformat_open_input(&fmtCtx, file.c_str(), NULL, NULL);
    assert(0 == ret);

    ret = avformat_find_stream_info(fmtCtx, NULL);
    assert(0 == ret);

    audioStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    assert(0 <= audioStreamIndex);

    codecCtx = avcodec_alloc_context3(codec);
    assert(NULL != codecCtx);

    avcodec_parameters_to_context(codecCtx, fmtCtx->streams[audioStreamIndex]->codecpar);

    ret = avcodec_open2(codecCtx, codec, NULL);
    assert(0 == ret);

    if(!codecCtx->channel_layout)
    {
        codecCtx->channel_layout = av_get_default_channel_layout(codecCtx->channels);
    }

    enum AVSampleFormat sfmt = codecCtx->sample_fmt;
    int channels = codecCtx->channels;

    if(av_sample_fmt_is_planar(sfmt))
    {
        // const char *packed = av_get_sample_fmt_name(sfmt);
        printf("Warning: the sample format the decoder produced is planar! \n");
        sfmt = av_get_packed_sample_fmt(sfmt);
        channels = 1;
    }

    printf("audio channels: %d, AVSampleFormat: %d \n", channels, sfmt);

    while (true)
    {
        if(av_read_frame(fmtCtx, &pkt) < 0)
        {
            break;
        }

        if(pkt.stream_index == audioStreamIndex)
        {
            ret = avcodec_send_packet(codecCtx, &pkt);
            if(ret < 0)
            {
                break;
            }

            while (ret >= 0)
            {
                AVFrame* frame = av_frame_alloc();
                ret = avcodec_receive_frame(codecCtx, frame);
                if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if(ret < 0)
                {
                    assert(0);
                }

                frames.push_back(frame);
            }
        }
        av_packet_unref(&pkt);
    }

    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);

    return frames;
}


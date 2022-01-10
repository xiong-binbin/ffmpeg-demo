/*
 * @Author: your name
 * @Date: 2022-01-10 09:40:47
 * @LastEditTime: 2022-01-10 18:49:18
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/src/audio_mix.cpp
 */
#include "audio_mix.h"


AudioMix::AudioMix()
{
    int ret = 0;
    int size = 0;
    AVFrame* filtFrame = av_frame_alloc();
    
    std::vector<AVFrame*> af0 = AudioDecode("0.mp4");
    std::vector<AVFrame*> af1 = AudioDecode("1.mp4");
    size = af0.size() < af1.size() ? af0.size() : af1.size();

    ret = CreateAudioFilter(&m_filterGraph, m_filterCtxSrc, 2, &m_filterCtxSink);

    FILE* file = fopen("out.pcm", "wb");
    assert(NULL != file);

    for(int i = 0; i < size; i++)
    {
        ret = av_buffersrc_add_frame(m_filterCtxSrc[0], af0[i]);
        assert(0 == ret);

        ret = av_buffersrc_add_frame(m_filterCtxSrc[1], af1[i]);
        assert(0 == ret);

        while ((ret = av_buffersink_get_frame(m_filterCtxSink, filtFrame)) >= 0)
        {
            size_t unpadded_linesize = filtFrame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)filtFrame->format);
            fwrite(filtFrame->extended_data[0], 1, unpadded_linesize, file);
            av_frame_unref(filtFrame);
        }

        av_frame_free(&(af0[i]));
        av_frame_free(&(af1[i]));
    }

    fclose(file);

    // FILE* file = fopen("1.pcm", "wb");
    // assert(NULL != file);
    // for(auto iter = af1.begin(); iter != af1.end(); iter++)
    // {
    //     size_t unpadded_linesize = (*iter)->nb_samples * av_get_bytes_per_sample((AVSampleFormat)(*iter)->format);
    //     fwrite((*iter)->extended_data[0], 1, unpadded_linesize, file);
    //     av_frame_free(&(*iter));
    // }
    // fclose(file);

    std::cout << "Finish!" << std::endl;
}

AudioMix::~AudioMix()
{
}

AVFrame* AudioMix::AllocSilenceFrame(int nb_samples, int sample_rate, AVSampleFormat sample_fmt, int channels, uint64_t channel_layout)
{
    int ret = 0;
    AVFrame* frame = NULL;

    frame = av_frame_alloc();
    assert(NULL != frame);

    frame->nb_samples = nb_samples;
    frame->sample_rate = sample_rate;
    frame->format = sample_fmt;
    frame->channels = channels;
    frame->channel_layout = channel_layout;

    ret = av_frame_get_buffer(frame, 0);
    assert(0 == ret);

    av_samples_set_silence(frame->extended_data, 0, frame->nb_samples, frame->channels, (AVSampleFormat)frame->format);
    return frame;
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

    printf("file: %s, audio sample_rate: %d, channels: %d, sample_format: %d, channel_layout=%ld \n", file.c_str(), codecCtx->sample_rate, channels, sfmt, codecCtx->channel_layout);

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

int AudioMix::CreateAudioFilter(AVFilterGraph **graph, AVFilterContext **srcs, int len, AVFilterContext **sink)
{
    int ret = 0;
    char args[256] = {0};
    AVFilterGraph* filterGraph = avfilter_graph_alloc();
    assert(NULL != filterGraph);

    for (int i = 0; i < len; i++)
    {
        //创建音频输入滤镜
        const AVFilter* abuffer = avfilter_get_by_name("abuffer");
        memset(args, 0, sizeof(args));
        snprintf(args, sizeof(args), "src%d", i);
        AVFilterContext* abufferCtx = avfilter_graph_alloc_filter(filterGraph, abuffer, args);

        memset(args, 0, sizeof(args));
        snprintf(args, sizeof(args), "time_base=1/44100:sample_rate=44100:sample_fmt=fltp:channel_layout=3");
        ret = avfilter_init_str(abufferCtx, args);
        assert(0 == ret);

        srcs[i] = abufferCtx;
    }

    //创建混合音频滤镜
    const AVFilter* amix = avfilter_get_by_name("amix");
    AVFilterContext* amixCtx = avfilter_graph_alloc_filter(filterGraph, amix, "amix");
    memset(args, 0, sizeof(args));
    snprintf(args, sizeof(args), "inputs=%d:duration=first:dropout_transition=3", len);
    ret = avfilter_init_str(amixCtx, args);
    assert(0 == ret);

    //创建音频格式滤镜
    const AVFilter* aformat = avfilter_get_by_name("aformat");
    AVFilterContext* aformatCtx = avfilter_graph_alloc_filter(filterGraph, aformat, "aformat");
    memset(args, 0, sizeof(args));
    snprintf(args, sizeof(args), "sample_rates=48000:sample_fmts=fltp:channel_layouts=3");
    ret = avfilter_init_str(aformatCtx, args);
    assert(0 == ret);

    //创建音频输出滤镜
    const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterContext* sinkCtx = avfilter_graph_alloc_filter(filterGraph, abuffersink, "sink");
    avfilter_init_str(sinkCtx, NULL);

    //链接滤镜
    for(int i = 0; i < len; i++)
    {
        ret = avfilter_link(srcs[i], 0, amixCtx, i);
        assert(0 == ret);
    }

    ret = avfilter_link(amixCtx, 0, aformatCtx, 0);
    assert(0 == ret);

    ret = avfilter_link(aformatCtx, 0, sinkCtx, 0);
    assert(0 == ret);

    //设置参数
    ret = avfilter_graph_config(filterGraph, NULL);
    assert(0 == ret);

    *graph = filterGraph;
    *sink = sinkCtx;

    return 0;
}

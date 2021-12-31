/*
 * @Author: your name
 * @Date: 2021-12-17 16:17:06
 * @LastEditTime: 2021-12-31 18:00:55
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/src/audio_filter.cpp
 */
#include "audio_filter.h"

AudioFilter::AudioFilter()
{
    int ret = 0;
    int audioStreamIndex = -1;
    AVCodec* codec = NULL;
    AVFormatContext* fmtCtx = NULL;
    AVCodecContext* codecCtx = NULL;
    AVFrame* frame = av_frame_alloc();
    AVFrame* filtFrame = av_frame_alloc();

    char args[512] = {0};
    AVFilterContext* buffersrcCtx = NULL;
    AVFilterContext* buffersinkCtx = NULL;
    AVFilterGraph*   filterGraph = NULL;
    const AVFilter* abuffersrc = avfilter_get_by_name("abuffer");
    const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
    AVFilterInOut*  filterIn = avfilter_inout_alloc();
    AVFilterInOut*  filterOut = avfilter_inout_alloc();
    AVFilterLink*   outlink = NULL;
    enum AVSampleFormat outSampleFmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
    int64_t outChannelLayouts[] = { AV_CH_LAYOUT_MONO, -1 };
    int outSampleRates[] = { 8000, -1 };

    ret = avformat_open_input(&fmtCtx, "test.mp4", NULL, NULL);
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

    filterGraph = avfilter_graph_alloc();
    assert(NULL != filterGraph);

    if(!codecCtx->channel_layout)
    {
        codecCtx->channel_layout = av_get_default_channel_layout(codecCtx->channels);
    }

    AVRational time_base = fmtCtx->streams[audioStreamIndex]->time_base;
    snprintf(args, sizeof(args), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%");

}

AudioFilter::~AudioFilter()
{
}



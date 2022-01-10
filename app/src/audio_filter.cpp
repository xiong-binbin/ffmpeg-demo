/*
 * @Author: your name
 * @Date: 2021-12-17 16:17:06
 * @LastEditTime: 2022-01-10 15:13:48
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
    AVPacket pkt = {0};
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
    int outSampleRates[] = { 48000, -1 };
    const char* filter_descr = "aresample=48000,aformat=sample_fmts=s16:channel_layouts=mono";

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

    if(!codecCtx->channel_layout)
    {
        codecCtx->channel_layout = av_get_default_channel_layout(codecCtx->channels);
    }

    filterGraph = avfilter_graph_alloc();
    assert(NULL != filterGraph);

    AVRational time_base = fmtCtx->streams[audioStreamIndex]->time_base;
    snprintf(args, sizeof(args), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64, 
        time_base.num, time_base.den, codecCtx->sample_rate, av_get_sample_fmt_name(codecCtx->sample_fmt), codecCtx->channel_layout);

    ret = avfilter_graph_create_filter(&buffersrcCtx, abuffersrc, "in", args, NULL, filterGraph);
    assert(0 == ret);

    ret = avfilter_graph_create_filter(&buffersinkCtx, abuffersink, "out", NULL, NULL, filterGraph);
    assert(0 == ret);

    ret = av_opt_set_int_list(buffersinkCtx, "sample_fmts", outSampleFmts, -1, AV_OPT_SEARCH_CHILDREN);
    assert(0 == ret);

    ret = av_opt_set_int_list(buffersinkCtx, "channel_layouts", outChannelLayouts, -1, AV_OPT_SEARCH_CHILDREN);
    assert(0 == ret);

    ret = av_opt_set_int_list(buffersinkCtx, "sample_rates", outSampleRates, -1, AV_OPT_SEARCH_CHILDREN);
    assert(0 == ret);

    filterOut->name        = av_strdup("in");
    filterOut->filter_ctx  = buffersrcCtx;
    filterOut->pad_idx     = 0;
    filterOut->next        = NULL;

    filterIn->name       = av_strdup("out");
    filterIn->filter_ctx = buffersinkCtx;
    filterIn->pad_idx    = 0;
    filterIn->next       = NULL;

    ret = avfilter_graph_parse_ptr(filterGraph, filter_descr, &filterIn, &filterOut, NULL);
    assert(0 <= ret);

    ret = avfilter_graph_config(filterGraph, NULL);
    assert(0 == ret);

    outlink = buffersinkCtx->inputs[0];
    memset(args, 0, sizeof(args));
    av_get_channel_layout_string(args, sizeof(args), -1, outlink->channel_layout);
    
    std::cout << "sample_rate: " << outlink->sample_rate << std::endl;
    std::cout << "sample_fmt: " << av_get_sample_fmt_name((AVSampleFormat)outlink->format) << std::endl;
    std::cout << "channel_layout: " << args << std::endl;

    avfilter_inout_free(&filterIn);
    avfilter_inout_free(&filterOut);

    FILE* file = fopen("out.pcm", "wb");
    assert(NULL != file);

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
                ret = avcodec_receive_frame(codecCtx, frame);
                if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if(ret < 0)
                {
                    assert(0);
                }

                ret = av_buffersrc_add_frame_flags(buffersrcCtx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
                assert(0 == ret);

                while (true)
                {
                    ret = av_buffersink_get_frame(buffersinkCtx, filtFrame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    else if(ret < 0)
                    {
                        assert(0);
                    }

                    size_t unpadded_linesize = filtFrame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)filtFrame->format);
                    fwrite(filtFrame->extended_data[0], 1, unpadded_linesize, file);

                    av_frame_unref(filtFrame);
                }

                av_frame_unref(frame);
            }
        }
        av_packet_unref(&pkt);
    }

    fclose(file);
    avfilter_graph_free(&filterGraph);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
    av_frame_free(&frame);
    av_frame_free(&filtFrame);

    std::cout << "Finish!" << std::endl;
}

AudioFilter::~AudioFilter()
{
}



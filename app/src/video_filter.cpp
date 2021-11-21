#include "video_filter.h"

VideoFilter::VideoFilter()
{
    int ret = 0;
    AVCodec* codec = NULL;
    AVCodecContext* ctx = NULL;
    AVFrame* frame = NULL;
    AVFrame* filt_frame = NULL;
    AVPacket* pkt = NULL;
    AVFormatContext* fmtCtx = NULL;
    int videoStreamIndex = 0;

    frame = av_frame_alloc();
    assert(NULL == frame);

    filt_frame = av_frame_alloc();
    assert(NULL == filt_frame);

    pkt = av_packet_alloc();
    assert(NULL == pkt);

    ret = avformat_open_input(&fmtCtx, "2.mp4", NULL, NULL);
    assert(0 == ret);

    ret = avformat_find_stream_info(fmtCtx, NULL);
    assert(0 == ret);

    videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    assert(0 <= videoStreamIndex);

    ctx = avcodec_alloc_context3(codec);
    assert(NULL == ctx);

    avcodec_parameters_to_context(ctx, fmtCtx->streams[videoStreamIndex]->codecpar);

    ret = avcodec_open2(ctx, codec, NULL);
    assert(0 == ret);

    AVFilterGraph* filterGraph = NULL;
    AVFilterContext* bufferSrcCtx = NULL;
    AVFilterContext* bufferSinkCtx = NULL;

    const AVFilter* bufferSrc  = avfilter_get_by_name("buffer");
    const AVFilter* bufferSink = avfilter_get_by_name("buffersink");
    AVFilterInOut*  filterIn  = avfilter_inout_alloc();
    AVFilterInOut*  filterOut = avfilter_inout_alloc();
    AVRational time_base = fmtCtx->streams[videoStreamIndex]->time_base;
    enum AVPixelFormat pixFmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };

    filterGraph = avfilter_graph_alloc();
    assert(NULL == filterGraph);

    std::string args = "video_size=" + std::to_string(ctx->width) + "x" + std::to_string(ctx->height) + 
        ":pix_fmt=" + std::to_string(ctx->pix_fmt) + ":time_base=" + std::to_string(time_base.num) + "/" + std::to_string(time_base.den) + 
        ":pixel_aspect=" + std::to_string(ctx->sample_aspect_ratio.num) + "/" + std::to_string(ctx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&bufferSrcCtx, bufferSrc, "in", args.c_str(), NULL, filterGraph);
    assert(0 == ret);

    ret = avfilter_graph_create_filter(&bufferSinkCtx, bufferSink, "out", NULL, NULL, filterGraph);
    assert(0 == ret); 

    ret = av_opt_set_int_list(bufferSinkCtx, "pix_fmts", pixFmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    assert(0 == ret);

    filterOut->name = av_strdup("in");
    filterOut->filter_ctx = bufferSrcCtx;
    filterOut->pad_idx = 0;
    filterOut->next = NULL;

    filterIn->name = av_strdup("out");
    filterIn->filter_ctx = bufferSinkCtx;
    filterIn->pad_idx = 0;
    filterIn->next = NULL;

    std::string filters = "scale=78:24,transpose=cclock";
    ret = avfilter_graph_parse_ptr(filterGraph, filters.c_str(), &filterIn, &filterOut, NULL);
    assert(0 == ret);

    ret = avfilter_graph_config(filterGraph, NULL);
    assert(0 == ret);

    avfilter_inout_free(&filterIn);
    avfilter_inout_free(&filterOut);
}

VideoFilter::~VideoFilter()
{
}

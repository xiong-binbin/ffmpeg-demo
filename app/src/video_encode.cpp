#include "video_encode.h"


VideoEncode::VideoEncode()
{
    int ret = 0;
    FILE* fp = NULL;
    AVFrame* frame = NULL;
    AVPacket* pkt = NULL;
    AVCodec* codec = NULL;
    AVCodecContext* ctx = NULL;

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    assert(NULL != codec);

    ctx = avcodec_alloc_context3(codec);
    assert(NULL != ctx);

    pkt = av_packet_alloc();
    assert(NULL != pkt);

    ctx->bit_rate = 400000;
    ctx->width = 1280;
    ctx->height = 720;
    ctx->time_base = (AVRational){1, 25};
    ctx->framerate = (AVRational){25, 1};
    ctx->gop_size = 10;
    ctx->max_b_frames = 1;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    if(codec->id == AV_CODEC_ID_H264)
    {
        av_opt_set(ctx->priv_data, "preset", "slow", 0);
    }

    ret = avcodec_open2(ctx, codec, NULL);
    assert(0 == ret);

    fp = fopen("dst", "wb");
    assert(NULL != fp);

    frame = av_frame_alloc();
    assert(NULL != frame);

    frame->format = ctx->pix_fmt;
    frame->width = ctx->width;
    frame->height = ctx->height;

    ret = av_frame_get_buffer(frame, 0);
    assert(0 == ret);

    for(int i = 0; i < 25; i++)
    {
        ret = av_frame_make_writable(frame);
        assert(0 == ret);

        for(int y = 0; y < ctx->height; y++)
        {
            for(int x = 0; x < ctx->width; x++)
            {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i*3;
            }
        }

        for(int y = 0; y < ctx->height/2; y++)
        {
            for(int x = 0; x < ctx->width/2; x++)
            {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i*2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i*5;
            }
        }

        frame->pts = i;

        encode(ctx, frame, pkt, fp);
    }

    encode(ctx, NULL, pkt, fp);
    if(codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
    {
        uint8_t endcode[] = { 0, 0, 1, 0xb7 };
        fwrite(endcode, 1, sizeof(endcode), fp);
    }

    fclose(fp);
    avcodec_free_context(&ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

VideoEncode::~VideoEncode()
{
}

void VideoEncode::encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, FILE *fp)
{
    int ret = 0;

    ret = avcodec_send_frame(ctx, frame);
    assert(0 == ret);

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(ctx, pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if(ret < 0)
        {
            assert(0);
        }

        fwrite(pkt->data, 1, pkt->size, fp);
        av_packet_unref(pkt);
    }
}

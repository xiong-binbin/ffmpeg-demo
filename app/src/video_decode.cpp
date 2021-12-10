#include "video_decode.h"
#include "libyuv.h"

#define INBUF_SIZE 4096

VideoDecode::VideoDecode()
{
    int ret = 0;
    FILE *in_file = NULL;
    AVFrame *frame = NULL;
    AVCodec *codec = NULL;
    AVPacket *pkt = NULL;
    AVCodecContext *ctx= NULL;
    AVCodecParserContext *parser = NULL;
    uint8_t *data = NULL;
    size_t   data_size = 0;
    uint8_t  inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE] = {0};

    pkt = av_packet_alloc();
    assert(NULL != pkt);

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    assert(NULL != codec);

    parser = av_parser_init(codec->id);
    assert(NULL != parser);

    ctx = avcodec_alloc_context3(codec);
    assert(NULL != ctx);

    if (avcodec_open2(ctx, codec, NULL) < 0) 
    {
        assert(0);
    }

    in_file = fopen("video.h264", "rb");
    assert(NULL != in_file);

    frame = av_frame_alloc();
    assert(NULL != frame);

    while (!feof(in_file))
    {
        data_size = fread(inbuf, 1, INBUF_SIZE, in_file);
        if(data_size <= 0)
        {
            break;
        }

        data = inbuf;
        while (data_size > 0) 
        {
            ret = av_parser_parse2(parser, ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            assert(0 <= ret);

            data      += ret;
            data_size -= ret;

            if(pkt->size > 0)
            {
                decode(ctx, frame, pkt, "yuv");
            }
        }
    }

    decode(ctx, frame, NULL, "yuv");

    fclose(in_file);
    av_parser_close(parser);
    avcodec_free_context(&ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    std::cout << "Finish!" << std::endl;
}

VideoDecode::~VideoDecode()
{
}

void VideoDecode::decode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, const char *file)
{
    int ret = 0;
    char name[128] = {0};
    int i = 0;
    FILE *fp = NULL;

    int x = 0;
    int y = 0;

    AVFrame* bgFrame  = av_frame_alloc();
    AVFrame* aFrame   = av_frame_alloc();
    assert(NULL != bgFrame);
    assert(NULL != aFrame);

    bgFrame->width = 1280;
    bgFrame->height = 720;
    bgFrame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(bgFrame, 0);

    //黑色背景
    memset(bgFrame->data[0], 0x00, bgFrame->linesize[0]*bgFrame->height);
    memset(bgFrame->data[1], 0x80, bgFrame->linesize[1]*bgFrame->height/2);
    memset(bgFrame->data[2], 0x80, bgFrame->linesize[2]*bgFrame->height/2);

    aFrame->width = 640;
    aFrame->height = 480;
    aFrame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(aFrame, 0);

    ret = avcodec_send_packet(ctx, pkt);
    assert(0 == ret);

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if (ret < 0) 
        {
            assert(0);
        }

        libyuv::I420Scale(frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2], frame->width, frame->height, 
            aFrame->data[0], aFrame->linesize[0], aFrame->data[1], aFrame->linesize[1], aFrame->data[2], aFrame->linesize[2], aFrame->width, aFrame->height, libyuv::kFilterBilinear);

        x = 0;
        y = 0;

        libyuv::I420Copy(aFrame->data[0] + aFrame->linesize[0],
                         aFrame->linesize[0],
                         aFrame->data[1] + aFrame->linesize[1],
                         aFrame->linesize[1],
                         aFrame->data[2] + aFrame->linesize[2],
                         aFrame->linesize[2],
                         bgFrame->data[0] + x + y*bgFrame->linesize[0],
                         bgFrame->linesize[0],
                         bgFrame->data[1] + x/2 + y/2*bgFrame->linesize[1],
                         bgFrame->linesize[1],
                         bgFrame->data[2] + x/2 + y/2*bgFrame->linesize[2],
                         bgFrame->linesize[2],
                         bgFrame->width - x >= aFrame->width ? aFrame->width : (bgFrame->width - x),
                         bgFrame->height - y >= aFrame->height ? aFrame->height : (bgFrame->height - y));

        x = 700;
        y = 400;

        libyuv::I420Copy(aFrame->data[0] + aFrame->linesize[0],
                         aFrame->linesize[0],
                         aFrame->data[1] + aFrame->linesize[1],
                         aFrame->linesize[1],
                         aFrame->data[2] + aFrame->linesize[2],
                         aFrame->linesize[2],
                         bgFrame->data[0] + x + y*bgFrame->linesize[0],
                         bgFrame->linesize[0],
                         bgFrame->data[1] + x/2 + y/2*bgFrame->linesize[1],
                         bgFrame->linesize[1],
                         bgFrame->data[2] + x/2 + y/2*bgFrame->linesize[2],
                         bgFrame->linesize[2],
                         bgFrame->width - x >= aFrame->width ? aFrame->width : (bgFrame->width - x),
                         bgFrame->height - y >= aFrame->height ? aFrame->height : (bgFrame->height - y));

        snprintf(name, sizeof(name), "%s-%d", file, ctx->frame_number);

        fp = fopen(name, "wb");
        fprintf(fp, "P5\n%d %d\n%d\n", bgFrame->width, bgFrame->height, 255);
        for(i = 0; i < bgFrame->height; i++)
        {
            fwrite(bgFrame->data[0] + i * bgFrame->linesize[0], 1, bgFrame->width, fp);
        }
        fclose(fp);
    }

    av_frame_free(&bgFrame);
    av_frame_free(&aFrame);
}

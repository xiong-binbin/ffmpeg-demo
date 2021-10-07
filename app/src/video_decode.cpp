#include "video_decode.h"

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
    if (!pkt)
    {
        exit(1);
    }

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) 
    {
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if (!parser) 
    {
        exit(1);
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) 
    {
        exit(1);
    }

    if (avcodec_open2(ctx, codec, NULL) < 0) 
    {
        exit(1);
    }

    in_file = fopen("video.h264", "rb");
    if (!in_file) 
    {
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) 
    {
        exit(1);
    }

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
            if(ret < 0)
            {
                exit(1);
            }

            data      += ret;
            data_size -= ret;

            if(pkt->size > 0)
            {
                decode(ctx, frame, pkt, "test");
            }
        }
    }

    decode(ctx, frame, NULL, "test");

    fclose(in_file);
    av_parser_close(parser);
    avcodec_free_context(&ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
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

    ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) 
    {
        exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if (ret < 0) 
        {
            exit(1);
        }

        snprintf(name, sizeof(name), "%s-%d", file, ctx->frame_number);

        fp = fopen(name, "wb");
        fprintf(fp, "P5\n%d %d\n%d\n", frame->width, frame->height, 255);
        for(i = 0; i < frame->height; i++)
        {
            fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width, fp);
        }
        fclose(fp);
    }
}

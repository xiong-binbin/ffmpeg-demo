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
            assert(0 == ret);

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

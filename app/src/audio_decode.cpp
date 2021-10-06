#include "audio_decode.h"


#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

AudioDecode::AudioDecode()
{
    int ret = 0;
    int len = 0;
    FILE *in_file = NULL;
    FILE *out_file = NULL;
    uint8_t *data = NULL;
    size_t  data_size = 0;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE] = {0};
    AVCodec *codec = NULL;
    AVPacket *pkt = NULL;
    AVFrame *frame = NULL;
    AVCodecContext *ctx= NULL;
    AVCodecParserContext *parser = NULL;
    AVSampleFormat sfmt;

    pkt = av_packet_alloc();

    codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
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

    in_file = fopen("audio.aac", "rb");
    if (!in_file) 
    {
        exit(1);
    }

    out_file = fopen("out.pcm", "wb");
    if (!out_file) 
    {
        av_free(ctx);
        exit(1);
    }

    data      = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, in_file);

    while(data_size > 0)
    {
        if (!frame) 
        {
            frame = av_frame_alloc();
            if (!frame)
            {
                exit(1);
            }
        }
        ret = av_parser_parse2(parser, ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0)
        {
            exit(1);
        }

        data      += ret;
        data_size -= ret;

        if(pkt->size > 0)
        {
            decode(ctx, pkt, frame, out_file);
        }

        if(data_size < AUDIO_REFILL_THRESH)
        {
            memmove(inbuf, data, data_size);
            data = inbuf;
            len = fread(inbuf + data_size, 1, AUDIO_INBUF_SIZE - data_size, in_file);
            if(len > 0)
            {
                data_size += len;
            }
        }
    }

    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(ctx, pkt, frame, out_file);

    sfmt = ctx->sample_fmt;
    if(av_sample_fmt_is_planar(sfmt))
    {
        sfmt = av_get_packed_sample_fmt(sfmt);
    }

    fclose(out_file);
    fclose(in_file);

    avcodec_free_context(&ctx);
    av_parser_close(parser);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    std::cout << "end" << std::endl;
}

AudioDecode::~AudioDecode()
{
}

void AudioDecode::decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, FILE *file)
{
    int ret = 0;
    int len = 0, i = 0;
    int ch = 0;

    ret = avcodec_send_packet(ctx, pkt);
    if(ret < 0)
    {
        exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(ctx, frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if(ret < 0)
        {
            exit(1);
        }

        len = av_get_bytes_per_sample(ctx->sample_fmt);
        if(len < 0)
        {
            exit(1);
        }
        
        for(i=0; i<frame->nb_samples; i++)
        {
            for(ch=0; ch<ctx->channels; ch++)
            {
                fwrite(frame->data[ch] + len*i, 1, len, file);
            }
        }
    }
}

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
    assert(NULL != codec);

    parser = av_parser_init(codec->id);
    assert(NULL != parser);

    ctx = avcodec_alloc_context3(codec);
    assert(NULL != ctx);

    if (avcodec_open2(ctx, codec, NULL) < 0) 
    {
        assert(0);
    }

    in_file = fopen("audio.aac", "rb");
    assert(NULL != in_file);

    out_file = fopen("out.pcm", "wb");
    assert(NULL != out_file);

    data      = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, in_file);

    while(data_size > 0)
    {
        if (!frame) 
        {
            frame = av_frame_alloc();
            assert(NULL != frame);
        }
        ret = av_parser_parse2(parser, ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        assert(0 == ret);

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

    std::cout << "Finish!" << std::endl;
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
    assert(0 == ret);

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(ctx, frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return;
        }
        else if(ret < 0)
        {
            assert(0);
        }

        //创建格式转换器
        if(swr == NULL) {
            swr = swr_alloc_set_opts(NULL, frame->channel_layout, AV_SAMPLE_FMT_S16, frame->sample_rate,
                                     frame->channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);
            assert(swr != NULL);

            ret =swr_init(swr);
            assert(0 == ret);
        }

        //构建格式转换后的frame
        AVFrame* outFrame = av_frame_alloc();
        assert(NULL != outFrame);

        outFrame->nb_samples     = frame->nb_samples;
        outFrame->format         = AV_SAMPLE_FMT_S16;
        outFrame->channel_layout = frame->channel_layout;

        ret = av_frame_get_buffer(outFrame, 0);
        assert(0 == ret);

//        ret = swr_convert(swr, outFrame->data[0], )
//
//        len = av_get_bytes_per_sample(ctx->sample_fmt);
//        assert(0 <= len);
//
//        for(i=0; i<frame->nb_samples; i++)
//        {
//            for(ch=0; ch<ctx->channels; ch++)
//            {
//                fwrite(frame->data[ch] + len*i, 1, len, file);
//            }
//        }
    }
}

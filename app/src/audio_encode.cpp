/*
 * @Author: XiongBinBin
 * @Date: 2021-11-18 14:42:14
 * @LastEditTime: 2021-11-18 16:27:21
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/src/audio_encode.cpp
 */
#include "audio_encode.h"

AudioEncode::AudioEncode()
{
    int ret = 0;
    FILE* fp = NULL;
    AVFrame* frame = NULL;
    AVPacket* pkt = NULL;
    AVCodec* codec = NULL;
    AVCodecContext* ctx = NULL;

    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    assert(NULL != codec);

    ctx = avcodec_alloc_context3(codec);
    assert(NULL != ctx);

    ctx->bit_rate = 128000;
    ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    ret = check_sample_fmt(codec, ctx->sample_fmt);
    assert(0 == ret);

    ctx->sample_rate = 48000;
    ctx->channel_layout = AV_CH_LAYOUT_STEREO;
    ctx->channels = 2;

    ret = avcodec_open2(ctx, codec, NULL);
    assert(0 == ret);

    fp = fopen("audio.mp2", "wb");
    assert(NULL != fp);

    pkt = av_packet_alloc();
    assert(NULL != pkt);

    frame = av_frame_alloc();
    assert(NULL != frame);

    frame->nb_samples     = ctx->frame_size;
    frame->format         = ctx->sample_fmt;
    frame->channel_layout = ctx->channel_layout;

    ret = av_frame_get_buffer(frame, 0);
    assert(0 == ret);

    float t = 0;
    float tincr = 2 * M_PI * 440.0 / ctx->sample_rate;
    uint16_t* samples = NULL;

    for(int i=0; i<200; i++)
    {
        ret = av_frame_make_writable(frame);
        assert(0 == ret);
        samples = (uint16_t*)frame->data[0];

        for(int j=0; j<ctx->frame_size; j++)
        {
            samples[2*j] = (int)(sin(t) * 10000);
            for(int k=1; k<ctx->channels; k++)
            {
                samples[2*j + k] = samples[2*j];
            }
            t += tincr;
        }
        encode(ctx, frame, pkt, fp);
    }

    encode(ctx, NULL, pkt, fp);

    fclose(fp);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    std::cout << "Finish!" << std::endl;
}

AudioEncode::~AudioEncode()
{
}

int AudioEncode::check_sample_fmt(const AVCodec* codec, enum AVSampleFormat fmt)
{
    const enum AVSampleFormat* p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE)
    {
        if(*p == fmt) 
        {
            return 0; 
        }
        p++;
    }
    return -1;
}

void AudioEncode::encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, FILE* fp)
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

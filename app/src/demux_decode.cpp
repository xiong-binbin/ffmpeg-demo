#include "demux_decode.h"

DemuxDecode::DemuxDecode()
{
    int ret = 0;
    std::string srcFile = "2.mp4";

    ret = avformat_open_input(&m_fmtCtx, srcFile.c_str(), NULL, NULL);
    assert(0 == ret);

    ret = avformat_find_stream_info(m_fmtCtx, NULL);
    assert(0 == ret);

    if (open_codec_context(&m_videoStreamIdx, &m_videoDecCtx, m_fmtCtx, AVMEDIA_TYPE_VIDEO) >= 0) 
    {
        m_videoStream = m_fmtCtx->streams[m_videoStreamIdx];
        m_videoDstFile = fopen("video.yuv", "wb");
        assert(NULL != m_videoDstFile);

        m_videoWidth  = m_videoDecCtx->width;
        m_videoHeight = m_videoDecCtx->height;
        m_pixFmt      = m_videoDecCtx->pix_fmt;

        ret = av_image_alloc(m_videoDstData, m_videoDstLinesize, m_videoDecCtx->width, m_videoDecCtx->height, m_videoDecCtx->pix_fmt, 1);
        assert(0 == ret);

        m_videoDstBufsize = ret;
    }

    if (open_codec_context(&m_audioStreamIdx, &m_audioDecCtx, m_fmtCtx, AVMEDIA_TYPE_AUDIO) >= 0)
    {
        m_audioStream = m_fmtCtx->streams[m_audioStreamIdx];
        m_audioDstFile = fopen("audio.pcm", "wb");
        assert(NULL != m_audioDstFile);
    }

    av_dump_format(m_fmtCtx, 0, srcFile.c_str(), 0);
    if(!m_videoStream && !m_audioStream)
    {
        assert(0);
    }

    m_frame = av_frame_alloc();
    assert(NULL != m_frame);

    m_pkt = av_packet_alloc();
    assert(NULL != m_pkt);

    while (av_read_frame(m_fmtCtx, m_pkt) >= 0)
    {
        if(m_pkt->stream_index == m_videoStreamIdx)
        {
            ret = decode_packet(m_videoDecCtx, m_pkt);
        }
        else if(m_pkt->stream_index == m_audioStreamIdx)
        {
            ret = decode_packet(m_audioDecCtx, m_pkt);
        }

        av_packet_unref(m_pkt);
        if (ret < 0)
        {
            break;
        }
    }

    if(m_videoDecCtx)
    {
        decode_packet(m_videoDecCtx, NULL);
    }
    if(m_audioDecCtx)
    {
        decode_packet(m_audioDecCtx, NULL);
    }

    if(m_videoStream)
    {
        printf("ffplay -f rawvideo -pix_fmt %s -video_size %dx%d video.yuv \n", av_get_pix_fmt_name(m_videoDecCtx->pix_fmt), m_videoDecCtx->width, m_videoDecCtx->height);
    }

    if(m_audioStream)
    {
        enum AVSampleFormat sfmt = m_audioDecCtx->sample_fmt;
        int channels = m_audioDecCtx->channels;

        //planar：每个声道数据单独存放;packed：多个声道数据交错存放
        if(av_sample_fmt_is_planar(sfmt))
        {
            // const char *packed = av_get_sample_fmt_name(sfmt);
            printf("Warning: the sample format the decoder produced is planar! \n");
            sfmt = av_get_packed_sample_fmt(sfmt);
            channels = 1;
        }

        printf("audio channels: %d, AVSampleFormat: %d \n", channels, sfmt);
    }

    std::cout << "Finish!" << std::endl;
}

DemuxDecode::~DemuxDecode()
{
    avcodec_free_context(&m_videoDecCtx);
    avcodec_free_context(&m_audioDecCtx);
    avformat_close_input(&m_fmtCtx);
    if (m_videoDstFile)
    {
        fclose(m_videoDstFile);
    }
    if (m_audioDstFile)
    {
        fclose(m_audioDstFile);
    }
    av_packet_free(&m_pkt);
    av_frame_free(&m_frame);
    av_free(m_videoDstData[0]);
}

int DemuxDecode::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, AVMediaType type)
{
    int ret = -1; 
    int stream_index = 0;
    AVStream *stream = NULL;
    AVCodec *codec = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) 
    {
        return ret;
    }
    else
    {
        stream_index = ret;
        stream = fmt_ctx->streams[stream_index];
        codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) 
        {
            return AVERROR(EINVAL);
        }

        *dec_ctx = avcodec_alloc_context3(codec);
        if (!*dec_ctx) 
        {
            return AVERROR(ENOMEM);
        }

        if ((ret = avcodec_parameters_to_context(*dec_ctx, stream->codecpar)) < 0) 
        {
            return ret;
        }

        if ((ret = avcodec_open2(*dec_ctx, codec, NULL)) < 0) 
        {
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

int DemuxDecode::decode_packet(AVCodecContext *ctx, const AVPacket *pkt)
{
    int ret = 0;

    ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) 
    {
        return ret;
    }

    while (ret >= 0) 
    {
        ret = avcodec_receive_frame(ctx, m_frame);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        {
            return 0;
        }
        else if(ret < 0)
        {
            return ret;
        }

        if(ctx->codec->type  == AVMEDIA_TYPE_VIDEO)
        {
            ret = output_video_frame(m_frame);
        }
        else if(ctx->codec->type  == AVMEDIA_TYPE_AUDIO)
        {
            ret = output_audio_frame(m_frame);
        }

        av_frame_unref(m_frame);
        if(ret < 0)
        {
            return ret;
        }
    }

    return 0;
}

int DemuxDecode::output_audio_frame(AVFrame *frame)
{
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
    fwrite(frame->extended_data[0], 1, unpadded_linesize, m_audioDstFile);
    return 0;
}

int DemuxDecode::output_video_frame(AVFrame *frame)
{
    if (frame->width != m_videoWidth || frame->height != m_videoHeight || frame->format != m_pixFmt)
    {
        return -1;
    }

    av_image_copy(m_videoDstData, m_videoDstLinesize, (const uint8_t **)(frame->data), frame->linesize, (AVPixelFormat)frame->format, frame->width, frame->height);
    fwrite(m_videoDstData[0], 1, m_videoDstBufsize, m_videoDstFile);
    return 0;
}

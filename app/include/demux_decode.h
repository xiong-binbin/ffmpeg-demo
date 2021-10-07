#include "common.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class DemuxDecode
{
public:
    DemuxDecode();
    ~DemuxDecode();

protected:
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, AVMediaType type);
    int decode_packet(AVCodecContext *ctx, const AVPacket *pkt);
    int output_audio_frame(AVFrame *frame);
    int output_video_frame(AVFrame *frame);

private:
    int32_t  m_videoStreamIdx{ -1 };
    int32_t  m_audioStreamIdx{ -1 };
    FILE*    m_videoDstFile{ NULL };
    FILE*    m_audioDstFile{ NULL };
    uint8_t* m_videoDstData[4];
    int32_t  m_videoDstLinesize[4];
    int32_t  m_videoDstBufsize;
    int32_t  m_videoWidth;
    int32_t  m_videoHeight;

    AVPixelFormat    m_pixFmt;
    AVCodecContext*  m_videoDecCtx{ NULL };
    AVCodecContext*  m_audioDecCtx{ NULL };
    AVFormatContext* m_fmtCtx{ NULL };
    AVStream*  m_videoStream{ NULL };
    AVStream*  m_audioStream{ NULL };
    AVFrame*   m_frame;
    AVPacket*  m_pkt;
};


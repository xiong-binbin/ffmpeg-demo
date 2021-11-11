#include "common.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

class VideoEncode
{
public:
    VideoEncode();
    ~VideoEncode();

protected:
    static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, FILE *fp);
};


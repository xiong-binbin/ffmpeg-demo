#include "common.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

class VideoDecode
{
public:
    VideoDecode();
    ~VideoDecode();

protected:
    static void decode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, const char *file);

};

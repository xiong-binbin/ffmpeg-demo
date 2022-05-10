#include "common.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

class AudioDecode
{
public:
  AudioDecode();
  ~AudioDecode();

protected:
    void decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, FILE *file);

private:
    SwrContext* swr{ NULL };
};

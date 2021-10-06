#include "common.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

class AudioDecode
{
public:
  AudioDecode();
  ~AudioDecode();

protected:
  static void decode(AVCodecContext *ctx, AVPacket *pkt, AVFrame *frame, FILE *file);
};

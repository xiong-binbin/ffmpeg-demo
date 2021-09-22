#include "audio_decode.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}


AudioDecode::AudioDecode()
{
    AVPacket *pkt = NULL;
    pkt = av_packet_alloc();
    (void)pkt;
}

AudioDecode::~AudioDecode()
{
}

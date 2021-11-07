#include "common.h"
#include "audio_decode.h"
#include "video_decode.h"
#include "demux_decode.h"


int main(int argc, char *argv[])
{
    // AudioDecode* ad = new AudioDecode();
    // VideoDecode* vd = new VideoDecode();
    DemuxDecode* dd = new DemuxDecode();

    while (true)
    {
        pause();
    }

    return 0;
}

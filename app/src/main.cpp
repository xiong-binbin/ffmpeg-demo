/*
 * @Author: your name
 * @Date: 2021-11-17 15:19:08
 * @LastEditTime: 2021-12-08 16:29:22
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/src/main.cpp
 */
#include "common.h"
#include "audio_decode.h"
#include "video_decode.h"
#include "demux_decode.h"
#include "audio_encode.h"
#include "video_encode.h"


int main(int argc, char *argv[])
{
    // AudioDecode* ad = new AudioDecode();
    VideoDecode* vd = new VideoDecode();
    // DemuxDecode* dd = new DemuxDecode();
    // AudioEncode* ae = new AudioEncode();
    // VideoEncode* ve = new VideoEncode();

    while (true)
    {
        pause();
    }

    return 0;
}

/*
 * @Author: XiongBinBin
 * @Date: 2021-11-18 14:41:43
 * @LastEditTime: 2021-11-18 16:21:29
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/include/audio_encode.h
 */

#include "common.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
}

class AudioEncode
{
public:
    AudioEncode();
    ~AudioEncode();

protected:
    static int check_sample_fmt(const AVCodec* codec, enum AVSampleFormat fmt);
    static void encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, FILE* fp);
};


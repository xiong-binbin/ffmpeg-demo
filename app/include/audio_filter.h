/*
 * @Author: your name
 * @Date: 2021-12-17 16:15:23
 * @LastEditTime: 2022-01-06 13:44:54
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/include/audio_filter.h
 */
#include "common.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

class AudioFilter
{
public:
    AudioFilter();
    ~AudioFilter();

private:
    /* data */
};



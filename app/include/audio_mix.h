/*
 * @Author: your name
 * @Date: 2022-01-10 09:40:58
 * @LastEditTime: 2022-01-10 16:54:21
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /ffmpeg-demo/app/include/audio_mix.h
 */
#include "common.h"
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

class AudioMix
{
public:
    AudioMix();
    ~AudioMix();

    std::vector<AVFrame*> AudioDecode(const std::string& file);
    int CreateAudioFilter(AVFilterGraph **graph, AVFilterContext **srcs, int len, AVFilterContext **sink);

private:
    AVFilterGraph*   m_filterGraph{ NULL };
    AVFilterContext* m_filterCtxSrc[8];
    AVFilterContext* m_filterCtxSink{ NULL };
};


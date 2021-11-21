#include "common.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

class VideoFilter
{
public:
    VideoFilter();
    ~VideoFilter();

protected:
    void display_frame(const AVFrame* frame, AVRational time_base);

private:
    int64_t m_lastPts{ 0 };
};


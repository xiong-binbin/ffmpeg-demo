#include "common.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
}

class VideoScale
{
public:
    VideoScale();
    ~VideoScale();
};


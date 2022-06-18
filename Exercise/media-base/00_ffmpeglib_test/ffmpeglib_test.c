#include <stdio.h>
#include "ffmpeg_test.h"

int main(void)
{
    printf("ffmpeg lib test:\n");

    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_DEBUG, "hello world.\n");

    return 0;
}

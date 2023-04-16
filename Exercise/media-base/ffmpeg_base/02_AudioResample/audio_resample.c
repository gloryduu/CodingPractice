#include "ffmpeg_test.h"

/*
20220621
运行失败1:
[alsa @ 0x55c94d7c9f00] ALSA buffer xrun.
[alsa @ 0x55c94d7c9f00] ALSA read error: Input/output error
av_read_frame faild[-5]: Input/output error
free(): invalid pointer
Aborted (core dumped)
--错误原因: 输入流写的速度过快，av_read_frame读的过慢，导致缓冲区满了，就会报这个IO错误

        iret = swr_convert(pstSwrCtx, dst_data, dstLineSize, (const uint8_t**)src_data, srcLineSize);
    out_count -> 512 int in_count -> 512
--直接原因如上接口参数错误
*/
int main(int argc, char *argv[])
{
    int iret = 0;
    char errors[1024] = "";
    char audioName[32] = "";
    unsigned int cnt = 0;
    AVDictionary *pstAVDic = NULL;
    AVFormatContext *pstAVFmtCtx = NULL;
    AVPacket stAVPkt;
    const char *pcmResfile = "./02audioRes.pcm";
    FILE *pfile = NULL;

    if (argc != 2)
    {
        printf("Usage: ./xx + audioDevide(alsa -> hw:0,0?)\n");
        return -1;
    }

    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_INFO, "ffmpeg audioin test bgn\n");

    //register all avdevice
    avdevice_register_all();

    //get audio format
    AVInputFormat *aiFmt = av_find_input_format("alsa");
    if (NULL == aiFmt)
    {
        av_log(NULL, AV_LOG_INFO, "find_input_format failed\n");
        return -1;
    }

    av_log(NULL, AV_LOG_INFO, "av_find_input_format succ: %p\n", aiFmt);
    if (argc >= 2)
    {
        strncpy(audioName, argv[1], sizeof(audioName));
        av_log(NULL, AV_LOG_INFO, "audioName: %s\n", audioName);
    }

    av_dict_set_int(&pstAVDic, "read_ahead_limit", INT_MAX, 0);
    iret = avformat_open_input(&pstAVFmtCtx, audioName, aiFmt, &pstAVDic);
    if (iret < 0)
    {
        av_strerror(iret, errors, 1024);
        av_log(NULL, AV_LOG_INFO, "open_input_device faild[%d]: %s\n", iret, errors);
        return -1;
    }
    av_log(NULL, AV_LOG_INFO, "avformat_open_input succ: pstAVDic %p pstAVFmtCtx %p\n", pstAVDic, pstAVFmtCtx);

    pfile = fopen(pcmResfile, "w");
    if (NULL == pfile)
    {
        av_log(NULL, AV_LOG_ERROR, "open audiofile[%s] failed\n", pcmResfile);
        return -1;
    }

    //输出采样为16000似乎支持的不太好?播放声音有问题
    SwrContext *pstSwrCtx = NULL;
    pstSwrCtx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
                                    AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 48000, 0, NULL);
    if (!pstSwrCtx)
    {
        av_log(NULL, AV_LOG_ERROR, "swr_alloc_set_opts failed\n");
        return -1;
    }

    iret = swr_init(pstSwrCtx);
    if (iret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "swr_init failed\n");
        return -1;
    }

    uint8_t **src_data = NULL;
    uint8_t **dst_data = NULL;
    uint32_t dataInSize = 0;
    int srcLineSize = 0;
    int dstLineSize = 0;
    iret = av_samples_alloc_array_and_samples(&src_data, &srcLineSize, 2, 512, AV_SAMPLE_FMT_S16, 0);
    iret |= av_samples_alloc_array_and_samples(&dst_data, &dstLineSize, 2, 512, AV_SAMPLE_FMT_S16, 0);
    if (iret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "av_samples_alloc_array_and_samples failed\n");
        return -1;
    }

    av_log(NULL, AV_LOG_INFO, "srcdata[%p] size[%u], dstdata[%p] size[%u]\n", src_data, srcLineSize, dst_data, dstLineSize);

    for (;;)
    {
        //当前linux环境下每次采样包大小为64字节，需累计至少32个采样点才可送至重采样器
        memset(&stAVPkt, 0, sizeof(stAVPkt));
        iret = av_read_frame(pstAVFmtCtx, &stAVPkt);
        if (iret < 0)
        {
            av_strerror(iret, errors, 1024);
            printf("av_read_frame faild[%d]: %s\n", iret, errors);
            return -1;
        }

        memcpy(src_data[0] + dataInSize, stAVPkt.data, stAVPkt.size);
        dataInSize += stAVPkt.size;
        av_packet_unref(&stAVPkt);

        if (dataInSize < 2048)
        {
            //printf("av_read_frame size[%u]\n", dataInSize);
            continue;
        }
        else
        {
            printf("av_read_frame size[%u]\n", dataInSize);
        }

        iret = swr_convert(pstSwrCtx, dst_data, 512, (const uint8_t**)src_data, 512);
        if (iret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "swr_convert failed\n");
            return -1;
        }

        fwrite(dst_data[0], 1, dstLineSize, pfile);
        fflush(pfile);

        av_log(NULL, AV_LOG_INFO, "write audio resample frame size[%u]\n", dstLineSize);
        dataInSize = 0;
    }

    //刷新最后待重采样的数据
    iret = swr_convert(pstSwrCtx, dst_data, 512, NULL, 0);
    if (iret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "swr_convert failed, no input data\n");
    }
    else
    {
        fwrite(dst_data[0], 1, dstLineSize, pfile);
        fflush(pfile);
    }

    //fini resource
    fclose(pfile);
    avformat_close_input(&pstAVFmtCtx);

    av_log(NULL, AV_LOG_INFO, "ffmpeg audioin test end\n");

    return 0;
}

#include "ffmpeg_base.h"

/*
20220630
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
    //const char *pcmResfile = "./02audioRes.pcm";
    const char *audioEncfile = "./03audioEnc.aac";
    FILE *pfile = NULL;

    if (argc != 2)
    {
        printf("Usage: ./xx + audioDevide(alsa -> hw:0,0?)\n");
        return -1;
    }

    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_INFO, "ffmpeg audioin test bgn: test-size %u\n", AV_CODEC_CAP_VARIABLE_FRAME_SIZE);

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

    pfile = fopen(audioEncfile, "w");
    if (NULL == pfile)
    {
        av_log(NULL, AV_LOG_ERROR, "open audiofile[%s] failed\n", audioEncfile);
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

    const char *audioEncoderName = "libfdk_aac";
    AVCodec *audioCodec = avcodec_find_encoder_by_name(audioEncoderName);
    if (!audioCodec)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_encoder_by_name[%s] failed\n", audioEncoderName);
        return -1;
    }

    AVCodecContext *audioCodecCtx = avcodec_alloc_context3(audioCodec);
    if (!audioCodecCtx)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3[codec-%p] failed\n", audioCodec);
        return -1;
    }
    audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
    audioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    audioCodecCtx->channels = 2;
    audioCodecCtx->sample_rate = 44100;
    audioCodecCtx->bit_rate = 0;    //如果设置profile，则bit_rate需设置为0，因为ffmpeg不解析该字段
    audioCodecCtx->profile = FF_PROFILE_AAC_HE_V2;
    //audioCodecCtx->frame_size = 512;

    iret = avcodec_open2(audioCodecCtx, audioCodec, NULL);
    if (iret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2[codecctx-%p codec-%p] failed\n", audioCodecCtx, audioCodec);
        return -1;
    }

    AVFrame audioFrame;
    memset(&audioFrame, 0, sizeof(audioFrame));
    audioFrame.nb_samples = 512;
    audioFrame.format = AV_SAMPLE_FMT_S16;
    audioFrame.channel_layout = AV_CH_LAYOUT_STEREO;
    iret = av_frame_get_buffer(&audioFrame, 0);
    if (iret < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "av_frame_get_buffer failed\n");
        return -1;
    }

    int encCnt = 0;
    AVPacket audioPkt;
    memset(&audioPkt, 0, sizeof(audioPkt));

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

        av_log(NULL, AV_LOG_INFO, "av_read_frame size[%u]\n", dataInSize);
        iret = swr_convert(pstSwrCtx, dst_data, 512, (const uint8_t**)src_data, 512);
        if (iret < 0)
        {
            av_log(NULL, AV_LOG_ERROR, "swr_convert failed\n");
            return -1;
        }

        av_log(NULL, AV_LOG_INFO, "swr_convert size[%u]\n", iret);
        memcpy(audioFrame.data[0], dst_data[0], dstLineSize);
        iret = avcodec_send_frame(audioCodecCtx, &audioFrame);
        while (iret >= 0)
        {
            iret = avcodec_receive_packet(audioCodecCtx, &audioPkt);
            if (iret < 0)
            {
                if (AVERROR(EAGAIN) == iret || AVERROR_EOF == iret)
                {
                    av_log(NULL, AV_LOG_INFO, "avcodec_receive_packet end[%#x]\n", iret);
                    break;
                }

                av_log(NULL, AV_LOG_ERROR, "avcodec_receive_packet failed\n");
                return -1;
            }

            fwrite(audioPkt.data, 1, audioPkt.size, pfile);
            fflush(pfile);
            av_log(NULL, AV_LOG_INFO, "write audio encode packet size[%u]\n", audioPkt.size);
        }

        dataInSize = 0;
        if (encCnt++ == 32)
        {
            av_log(NULL, AV_LOG_INFO, "enc cnt[%u], exit enc\n", encCnt);
            break;
        }
    }

    //刷缓冲区的编码数据
    iret = avcodec_send_frame(audioCodecCtx, NULL);
    while (iret >= 0)
    {
        iret = avcodec_receive_packet(audioCodecCtx, &audioPkt);
        if (iret < 0)
        {
            if (AVERROR(EAGAIN) == iret || AVERROR_EOF == iret)
            {
                break;
            }

            av_log(NULL, AV_LOG_ERROR, "avcodec_receive_packet failed\n");
            return -1;
        }

        fwrite(audioPkt.data, 1, audioPkt.size, pfile);
        fflush(pfile);
        av_log(NULL, AV_LOG_INFO, "write audio encode packet size[%u]\n", audioPkt.size);
    }

    //fini resource
    fclose(pfile);
    avformat_close_input(&pstAVFmtCtx);

    av_log(NULL, AV_LOG_INFO, "ffmpeg audioEnc test end\n");

    return 0;
}

#include "ffmpeg_test.h"

/*
* 编译错误1:
* /usr/local/ffmpeg/lib/libavdevice.a(alsa_dec.o): In function `audio_read_header':
* ...
* ---缺少alsa库,添加编译依赖-lasound
*
* 编译错误2:
* /usr/bin/ld: /usr/local/ffmpeg/lib/libavfilter.a(asrc_sine.o): undefined reference to symbol 'lrint@@GLIBC_2.2.│5'
* ---缺少库libm.so(libmath),添加编译依赖-lm
*
* 编译错误3:
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavdevice/alsa_dec.c:108: undefined reference to `av_new_packet'
* ---ffmpeglib间存在依赖,编译过程需要循环查找依赖库,添加编译选项解决:-Wl,--start-group $(REFLIB) -Wl,--end-group
*
*
* 编译错误4:
* /usr/bin/ld: /usr/local/lib/libSDL2.a(SDL_dynapi.o): undefined reference to symbol 'dlclose@@GLIBC_2.2.5'
* ---动态链接相关库,添加编译依赖-ldl
*
*
* 编译错误5:
* undefined reference to `operator new(unsigned long)'
* ---缺少库c++库,添加编译依赖-lstdc++
*
*
* 编译错误6:
* /usr/bin/ld: /usr/local/lib/libSDL2.a(SDL_syssem.o): undefined reference to symbol 'sem_getvalue@@GLIBC_2.2.5'
* ---缺少线程库,添加编译依赖-lpthread
*
*
* 编译错误7:
* undefined reference to symbol 'xcb_setup_pixmap_formats_length'
* ---缺少库xcb相关库,添加编译依赖-lxcb-shm -lxcb-xfixes -lxcb-shape
*
*
* 编译错误8:
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavfilter/vf_deinterlace_vaapi.c:94: undefined reference to `vaErrorStr'
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavdevice/xv.c:92: undefined reference to `XShmDetach'
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavcodec/libfdk-aacdec.c:215: undefined reference to `aacDecoder_Close'
* /usr/bin/ld: /usr/local/ffmpeg/lib/libavdevice.a(xv.o): undefined reference to symbol 'XGetWindowAttributes'
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavdevice/xv.c:166: undefined reference to `XvQueryAdaptors'
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavutil/hwcontext_vaapi.c:1490: undefined reference to `vaGetDisplay'
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavutil/hwcontext_vaapi.c:1514: undefined reference to `vaGetDisplayDRM'
* /home/gloryd/Work/code/ffmpeg-4.1.9/libavcodec/libx264.c:183: undefined reference to `x264_encoder_reconfig'
*
* ---grep -wrns "vaErrorStr" /usr/ --> Binary file /usr/lib64/libva.so.1.4000.0 matches
* ---grep -wrns "XShmDetach" /usr/ --> Binary file /usr/lib64/libXext.so.6.4.0 matches
* ---grep -wrns "aacDecoder_Close" /usr/ --> Binary file /usr/lib64/libfdk-aac.so.1.0.0 matches
* ---grep -wrns "XGetWindowAttributes" /usr/ --> invalid --> search web -> -lX11
* ---grep -wrns "XvQueryAdaptors" /usr/ -> Binary file /usr/lib64/libXv.so.1.0.0 matches
* ---grep -wrns "XvQueryAdaptors" /usr/ -> Binary file /usr/lib64/libva-x11.so.1.4000.0 matches
* ---grep -wrns "vaGetDisplayDRM" /usr/ -> Binary file /usr/lib64/libva-drm.so.1.4000.0 matches
* ---grep -wrns "x264_encoder_reconfig" /usr/ -> Binary file /usr/lib64/libx264.so.142 matches
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
    const char *pcmfile = "./01audio.pcm";
    FILE *pfile = NULL;

    if (argc != 2)
    {
        printf("Usage: ./xx + audioDevide(alsa -> hw:0,0?) + packetnum\n");
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
        return 1;
    }

    av_log(NULL, AV_LOG_INFO, "av_find_input_format succ: %p\n", aiFmt);
    if (argc >= 2)
    {
        strncpy(audioName, argv[1], sizeof(audioName));
        av_log(NULL, AV_LOG_INFO, "audioName: %s\n", audioName);
    }

    iret = avformat_open_input(&pstAVFmtCtx, audioName, aiFmt, &pstAVDic);
    if (iret < 0)
    {
        av_strerror(iret, errors, 1024);
        av_log(NULL, AV_LOG_INFO, "open_input_device faild[%d]: %s\n", iret, errors);
        return 2;
    }
    av_log(NULL, AV_LOG_INFO, "avformat_open_input succ: pstAVDic %p pstAVFmtCtx %p\n", pstAVDic, pstAVFmtCtx);

    memset(&stAVPkt, 0, sizeof(stAVPkt));
    pfile = fopen(pcmfile, "w");
    if (NULL == pfile)
    {
        av_log(NULL, AV_LOG_ERROR, "open audiofile[%s] failed\n", pcmfile);
        return 3;
    }

    while (cnt++ < 8192)
    {
        iret = av_read_frame(pstAVFmtCtx, &stAVPkt);
        if (iret)
        {
            av_strerror(iret, errors, 1024);
            printf("av_read_frame faild[%d]: %s\n", iret, errors);
            return 3;
        }
        fwrite(stAVPkt.data, 1, stAVPkt.size, pfile);
        fflush(pfile);

        av_log(NULL, AV_LOG_INFO, "stAVPkt audio frame[%u] size: %u\n", cnt, stAVPkt.size);
        av_packet_unref(&stAVPkt);
    }

    //fini resource
    fclose(pfile);
    avformat_close_input(&pstAVFmtCtx);

    av_log(NULL, AV_LOG_INFO, "ffmpeg audioin test end\n");

    return 0;
}

#include "ffmpeg_base.h"

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
*
*
* 20220619
* 动态链接ffmpeglib方式可以从根本上解决上述各种依赖问题
    gloryd@ubuntu20:01_audioin$ ldd 01_audioin
        linux-vdso.so.1 (0x00007ffd7fb17000)
        libavcodec.so.58 => /usr/local/ffmpeg/lib/libavcodec.so.58 (0x00007f8092b10000)
        libavdevice.so.58 => /usr/local/ffmpeg/lib/libavdevice.so.58 (0x00007f8092af1000)
        libavformat.so.58 => /usr/local/ffmpeg/lib/libavformat.so.58 (0x00007f8092807000)
        libavutil.so.56 => /usr/local/ffmpeg/lib/libavutil.so.56 (0x00007f8092773000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f809256f000)
        libswresample.so.3 => /usr/local/ffmpeg/lib/libswresample.so.3 (0x00007f809254a000)
        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f80923f9000)
        libz.so.1 => /lib/x86_64-linux-gnu/libz.so.1 (0x00007f80923dd000)
        libfdk-aac.so.2 => /usr/local/lib/libfdk-aac.so.2 (0x00007f8092291000)
        libspeex.so.1 => /usr/local/lib/libspeex.so.1 (0x00007f8092274000)
        libx264.so.164 => /usr/local/lib/libx264.so.164 (0x00007f8091fca000)
        libx265.so.179 => /usr/local/lib/libx265.so.179 (0x00007f8091dea000)
        libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f8091dc5000)
        libavfilter.so.7 => /usr/local/ffmpeg/lib/libavfilter.so.7 (0x00007f8091958000)
        libxcb.so.1 => /lib/x86_64-linux-gnu/libxcb.so.1 (0x00007f809192e000)
        libasound.so.2 => /lib/x86_64-linux-gnu/libasound.so.2 (0x00007f8091833000)
        libSDL2-2.0.so.0 => /usr/local/lib/libSDL2-2.0.so.0 (0x00007f8091679000)
        libXv.so.1 => /lib/x86_64-linux-gnu/libXv.so.1 (0x00007f8091474000)
        libX11.so.6 => /lib/x86_64-linux-gnu/libX11.so.6 (0x00007f8091335000)
        libXext.so.6 => /lib/x86_64-linux-gnu/libXext.so.6 (0x00007f8091320000)
        /lib64/ld-linux-x86-64.so.2 (0x00007f809442c000)
        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f809131a000)
        libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f8091138000)
        libmvec.so.1 => /lib/x86_64-linux-gnu/libmvec.so.1 (0x00007f809110c000)
        libswscale.so.5 => /usr/local/ffmpeg/lib/libswscale.so.5 (0x00007f8090f7f000)
        libpostproc.so.55 => /usr/local/ffmpeg/lib/libpostproc.so.55 (0x00007f8090f5e000)
        libXau.so.6 => /lib/x86_64-linux-gnu/libXau.so.6 (0x00007f8090f58000)
        libXdmcp.so.6 => /lib/x86_64-linux-gnu/libXdmcp.so.6 (0x00007f8090f50000)
        libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f8090f35000)
        libbsd.so.0 => /lib/x86_64-linux-gnu/libbsd.so.0 (0x00007f8090f19000)
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

#include <jni.h>
#include <string>
#include "log.h"
#include <pthread.h>
#include "FFmpegVideo.h"
#include "FFmpegAudio.h"
#include <android/native_window_jni.h>

extern "C" {
#include <libavformat/avformat.h>
}
pthread_t main_tid;
FFmpegVideo *video;
FFmpegAudio *audio;
AVFormatContext *ifmt_ctx;
ANativeWindow* window;
ANativeWindow_Buffer window_buffer;
void draw_frame(AVFrame* frame) {
    if (!window) {
        return;
    }
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        return;
    }
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int dstStride = window_buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}

void *main_run(void *argc) {
    av_register_all();
    ifmt_ctx = avformat_alloc_context();
    if (avformat_open_input(&ifmt_ctx, (char *) argc, NULL, NULL) < 0) {
        LOGE("地址错误");
        return NULL;
    }
    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
        LOGE("查找流信息错误");
        return NULL;
    }
    int video_id = -1;
    int audio_id = -1;
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {

        //4.获取视频解码器
        AVCodecContext *pCodeCtx = ifmt_ctx->streams[i]->codec;
        AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);

        AVCodecContext *codec = avcodec_alloc_context3(pCodec);
        avcodec_copy_context(codec, pCodeCtx);
        if(avcodec_open2(codec,pCodec,NULL) < 0){
            LOGE("%s","解码器无法打开");
            continue;
        }

        if (pCodeCtx->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_id = i;
            video->setCodecContext(codec);
            if (video && video->adec_ctx) {
                ANativeWindow_setBuffersGeometry(window, video->adec_ctx->width, video->adec_ctx->height,
                                                 WINDOW_FORMAT_RGBA_8888);
            }
        }
        if(pCodeCtx->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_id = i;
            audio->setAvCodecContext(codec);
            audio->p_playid=audio_id;
            audio->time_base=ifmt_ctx->streams[i]->time_base;
        }
    }
    video->play();
    audio->play();
    AVPacket* packet = (AVPacket *) malloc(sizeof(AVPacket));
    while (av_read_frame(ifmt_ctx,packet)>=0) {
        if (video && video->is_playing && packet->stream_index == video_id) {
            video->put(packet);
        } else  if (audio&& audio->isPlay && packet->stream_index == audio_id) {
            audio->put(packet);
        }
        av_packet_unref(packet);
    }
    pthread_exit(0);
}

extern "C"
JNIEXPORT void JNICALL
Java_onairm_com_syncdemo_ZzpPlayer_play(JNIEnv *env, jobject instance, jstring path_,
                                        jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);

    char *path = (char *) env->GetStringUTFChars(path_, NULL);
    char *s2 = new char[strlen(path) + 1];
    char *copy_path = (char *) memcpy(s2, path, strlen(path) + 1);

    video = new FFmpegVideo();
    audio = new FFmpegAudio();
    video->set_draw_fun(draw_frame);
    pthread_create(&main_tid, NULL, main_run, (void *) copy_path);

    env->ReleaseStringUTFChars(path_, path);

}

//
// Created by Administrator on 2017/11/30.
//
#include <pthread.h>
#include <queue>

#ifndef NEWFFMPEGD_MASTER_FFMPEGVIDEO_H
#define NEWFFMPEGD_MASTER_FFMPEGVIDEO_H

#endif //NEWFFMPEGD_MASTER_FFMPEGVIDEO_H

#include "FFmpegAudio.h"
extern "C"{
#include <libavcodec/avcodec.h>
};
class FFmpegVideo{
public:
    FFmpegVideo();
    ~FFmpegVideo();
    void get(AVPacket* packet);
    void put(AVPacket* packet);
    void set_draw_fun(void (*temp)(AVFrame*));
    void setCodecContext(AVCodecContext* adec_ctx);
    void play();
    void setAudio(FFmpegAudio*);
    double synchronize(AVFrame *frame, double play);
public:
    int is_playing;
    std::queue<AVPacket*> queue;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_t video_tid;
    AVCodecContext *adec_ctx;
    AVRational time_base;
    FFmpegAudio* audio;
    double  clock;
};

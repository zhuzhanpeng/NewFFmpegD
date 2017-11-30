//
// Created by Edison on 2017/11/30.
//
#include <pthread.h>
#include <queue>
#ifndef SYNCDEMO_FFMPEGMUSIC_H
#define SYNCDEMO_FFMPEGMUSIC_H
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#endif //SYNCDEMO_FFMPEGMUSIC_H
class FFmpegMusic{
public:
    FFmpegMusic();
    ~FFmpegMusic();
    void get(AVPacket* pkt);
    void put(AVPacket* pkt);
    void setCodecContext(AVCodecContext* adec_ctx);
    void play();
public:
    int is_playing;
    pthread_t audio_tid;
    std::queue<AVPacket*> queue;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    AVCodecContext *adec_ctx;

};
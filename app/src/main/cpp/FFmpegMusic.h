//
// Created by Edison on 2017/11/30.
//
#include <pthread.h>
#include <queue>
#ifndef SYNCDEMO_FFMPEGMUSIC_H
#define SYNCDEMO_FFMPEGMUSIC_H

#endif //SYNCDEMO_FFMPEGMUSIC_H
class FFmpegMusic{
public:
    FFmpegMusic();
    ~FFmpegMusic();
    void get(int* value);
    void put(int value);
public:
    pthread_t audio_tid;
    std::queue<int> queue;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
};
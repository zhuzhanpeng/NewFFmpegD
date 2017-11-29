//
// Created by Administrator on 2017/11/30.
//
#include <pthread.h>
#include <queue>

#ifndef NEWFFMPEGD_MASTER_FFMPEGVIDEO_H
#define NEWFFMPEGD_MASTER_FFMPEGVIDEO_H

#endif //NEWFFMPEGD_MASTER_FFMPEGVIDEO_H

class FFmpegVideo{
public:
    FFmpegVideo();
    ~FFmpegVideo();
    void get(int* value);
    void put(int value);
public:
    std::queue<int> queue;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_t video_tid;
};

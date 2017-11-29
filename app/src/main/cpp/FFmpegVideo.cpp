//
// Created by Administrator on 2017/11/30.
//
#include "FFmpegVideo.h"
#include "log.h"
void* video_run(void* argc){
    FFmpegVideo *video = (FFmpegVideo *) argc;
    int value=0;
    int sum=0;
    while(sum<400){
        sum++;
        video->get(&value);
    }

    pthread_exit(0);
}
FFmpegVideo::FFmpegVideo(){
    pthread_create(&video_tid,NULL,video_run,this);
}

void FFmpegVideo::get(int* value){
    pthread_mutex_lock(&mutex);
    while (true){
        if(!queue.empty()){
            *value=queue.front();
            queue.pop();
            LOGE("get%d",*value);
            break;
        }   else{
            LOGE("wait");
            pthread_cond_wait(&cond,&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

}
void FFmpegVideo::put(int value){

    pthread_mutex_lock(&mutex);
    queue.push(value);
    LOGE("put%d",value);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}




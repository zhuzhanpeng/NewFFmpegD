//
// Created by Edison on 2017/11/30.
//
#include "log.h"
#include "FFmpegMusic.h"
void play_audio_frame(int value){
    LOGE("播放音频%d",value);
}
void* audio_run(void* argc){
    FFmpegMusic *video = (FFmpegMusic *) argc;
    int value=0;
    int sum=0;
    while(sum<400){
        sum++;
        video->get(&value);
        play_audio_frame(value);
    }
    pthread_exit(0);
}
FFmpegMusic::FFmpegMusic(){
    pthread_create(&audio_tid,NULL,audio_run,this);
}

void FFmpegMusic::get(int* value){
    pthread_mutex_lock(&mutex);
    while (true){
        if(!queue.empty()){
            *value=queue.front();
            queue.pop();
            LOGE("get音频%d",*value);
            break;
        }   else{
            LOGE("音频wait");
            pthread_cond_wait(&cond,&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

}
void FFmpegMusic::put(int value){
    pthread_mutex_lock(&mutex);
    queue.push(value);
    LOGE("put音频%d",value);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}
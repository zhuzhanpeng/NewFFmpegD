//
// Created by Edison on 2017/11/30.
//
#include "log.h"
#include "FFmpegMusic.h"
void play_audio_frame(int value){
    LOGE("播放音频%d",value);
}
void* audio_run(void* argc){
    FFmpegMusic *audio = (FFmpegMusic *) argc;
    AVPacket* packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    while(audio->is_playing){

        audio->get(packet);
        play_audio_frame(0);
    }
    pthread_exit(0);
}
FFmpegMusic::FFmpegMusic(){
}

void FFmpegMusic::play() {
    pthread_create(&audio_tid,NULL,audio_run,this);
}
void FFmpegMusic::get(AVPacket* pkt){
    pthread_mutex_lock(&mutex);
    while (true){
        if(!queue.empty()){
            //此处有疑问
            //从队列取出一个packet，clone一个给入参对象
            if(av_packet_ref(pkt, queue.front())==0){
                LOGE("取出一帧音频");
            }
            //取成功了 弹出队列 销毁packet
            AVPacket *packet = queue.front();
            queue.pop();
            av_free(packet);
            break;
        }   else{
            LOGE("音频wait");
            pthread_cond_wait(&cond,&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

}
void FFmpegMusic::put(AVPacket* packet){
    AVPacket *packet1 = (AVPacket *) av_malloc(sizeof(AVPacket));

    if (av_copy_packet(packet1, packet)) {
        LOGE("克隆失败");
//        克隆失败
    }
    pthread_mutex_lock(&mutex);
    queue.push(packet1);
//    给消费者解锁
    pthread_cond_signal(&cond);
    LOGE("压入一帧音频数据");
    pthread_mutex_unlock(&mutex);
}
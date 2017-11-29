#include <jni.h>
#include <string>
#include "log.h"
#include <pthread.h>
#include "FFmpegVideo.h"
pthread_t main_tid;
FFmpegVideo *video;

void* main_run(void* argc){
    int count=1;
    while(count < 200){
        video->put(count);
        count++;
    }
    pthread_exit(0);
}

extern "C"
JNIEXPORT void JNICALL
Java_onairm_com_syncdemo_ZzpPlayer_play(JNIEnv *env, jobject instance, jobject surface) {
    video = new FFmpegVideo();
    pthread_create(&main_tid,NULL,main_run,NULL);

}
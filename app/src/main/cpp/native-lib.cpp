#include <jni.h>
#include <string>
#include "log.h"
#include <pthread.h>

void* main_run(void* argc){
    for (int i = 0; i < 1000; ++i) {
        LOGE("让大爷心情好一些");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_onairm_com_syncdemo_ZzpPlayer_play(JNIEnv *env, jobject instance, jobject surface) {

    pthread_t main_tid;
    pthread_create(&main_tid,NULL,main_run,NULL);

}
#include <jni.h>
#include <string>
#include "log.h"
/*
#include <pthread.h>
#include <queue>

pthread_mutex_t mutex;
pthread_cond_t cond;
std::queue<int> queue;
pthread_t main_tid,video_tid;

void get(int* value){
    while (true){
        pthread_mutex_lock(&mutex);
        if(!queue.empty()){
            *value=queue.front();
            queue.pop();
            LOGE("get%d",*value);
            break;
        }   else{
            LOGE("wait");
            pthread_cond_wait(&cond,&mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

}
void put(int value){

    pthread_mutex_lock(&mutex);
    LOGE("put");
        queue.push(value);
    LOGE("put%d",value);
        pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void* video_run(void* argc){
    int value=0;

       get(&value);

    pthread_exit(0);
}

void* main_run(void* argc){
    int count=1;
    pthread_create(&video_tid,NULL,video_run,NULL);
        put(count);
    LOGE("执行到了吗");
//        LOGE("%d",count);
        count++;
    pthread_exit(0);
}
*/
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
using namespace std;

int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//该函数增加count数值
void * creator(void * arg)
{
    LOGE("creator add lock");
    pthread_mutex_lock(&mutex);

    count ++;

    LOGE("in creator count is : ");
    //条件满足时发送信号
    if(count > 0)
    {

        pthread_cond_signal(&cond);
    }


   LOGE("creator release lock");

    pthread_mutex_unlock(&mutex);

    return NULL;

}

//该函数减少count数值
void * consumer(void * arg)
{
   LOGE("consumer add lock");

    pthread_mutex_lock(&mutex);
    //当条件不满足时等待
    if(count <= 0)
    {
       LOGE("begin wait");
        pthread_cond_wait(&cond,&mutex);
        LOGE("end wait");
    }

    count --;

    LOGE("in consumer count is ");

    pthread_mutex_unlock(&mutex);

    LOGE("consumer release lock");

    return NULL;

}
/*int main()
{
    //两个线程，一个生产者线程一个消费者线程
    pthread_t createthread,consumethread;

    pthread_create(&consumethread, NULL, consumer, NULL);
    　　 sleep(2);
    pthread_create(&createthread, NULL, creator, NULL);

    //主进程等待两个线程结束
    pthread_join(createthread, NULL);
    pthread_join(consumethread, NULL);
    return 0;
}*/
extern "C"
JNIEXPORT void JNICALL
Java_onairm_com_syncdemo_ZzpPlayer_play(JNIEnv *env, jobject instance, jobject surface) {
    /*pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);

    pthread_create(&main_tid,NULL,main_run,NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);*/
    //两个线程，一个生产者线程一个消费者线程
    pthread_t createthread,consumethread;

    pthread_create(&consumethread, NULL, consumer, NULL);
    sleep(2);
    pthread_create(&createthread, NULL, creator, NULL);

    //主进程等待两个线程结束
   /* pthread_join(createthread, NULL);
    pthread_join(consumethread, NULL);*/
}
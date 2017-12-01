#include "FFmpegVideo.h"
#include "log.h"
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}
#define AV_SYNC_THRESHOLD_MIN 0.04;
#define AV_SYNC_THRESHOLD_MAX 0.1;
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1;
#define AV_NOSYNC_THRESHOLD 10.0;
double timestamp;
static void (*frame_call)(AVFrame* rgb);
void* video_run(void* argc){
    FFmpegVideo *video = (FFmpegVideo *) argc;
    AVFrame* frame = (AVFrame *) av_frame_alloc();
    //转换rgba
    SwsContext *sws_ctx = sws_getContext(
            video->adec_ctx->width, video->adec_ctx->height,video->adec_ctx->pix_fmt,
            video->adec_ctx->width,video->adec_ctx->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, 0, 0, 0);

    AVFrame *rgb_frame = av_frame_alloc();
    int out_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA,video->adec_ctx->width,
                                            video->adec_ctx->height, 1);
    LOGE("上下文宽%d  高%d",video->adec_ctx->width, video->adec_ctx->height);
    uint8_t *out_buffer = (uint8_t *) malloc(sizeof(uint8_t) * out_size);
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, out_buffer,
                         AV_PIX_FMT_RGBA,
                         video->adec_ctx->width,video->adec_ctx->height, 1);
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    int len=0,got_frame=0;
    while(video->is_playing){
        video->get(packet);
        len = avcodec_decode_video2(video->adec_ctx,frame, &got_frame, packet);

        if (!got_frame) {
            continue;
        }
       timestamp = av_frame_get_best_effort_timestamp(frame)*av_q2d(video->time_base);
//        转码成rgb
        int code=sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
                           video->adec_ctx->height,
                           rgb_frame->data, rgb_frame->linesize);
        if(packet->pts == AV_NOPTS_VALUE) {
            timestamp = 0;
        } else {
            timestamp = av_frame_get_best_effort_timestamp(frame)*av_q2d(video->time_base);
        }
//计算帧率，平均每帧间隔时间
        double frameRate = video->time_base.num/video->time_base.den;
        frameRate += frame->repeat_pict * (frameRate * 0.5);
        if (timestamp == 0.0) {
            //按照默认帧率播放
            av_usleep((unsigned long)(frameRate*1000));
        }else {
            bool a=fabs(timestamp - video->audio->clock) > AV_SYNC_THRESHOLD_MIN;
            bool b = fabs(timestamp - video->audio->clock) < AV_NOSYNC_THRESHOLD;
            if (a&&b) {
                //如果视频比音频快，延迟差值播放，否则直接播放，这里没有做丢帧处理
                if (timestamp > video->audio->clock) {
                    av_usleep((unsigned long)((timestamp - video->audio->clock)*1000000));
                }
            }
        }

        frame_call(rgb_frame);
    }
    pthread_exit(0);
}
FFmpegVideo::FFmpegVideo(){

}
void FFmpegVideo::set_draw_fun(void (*temp)(AVFrame* )){
    frame_call=temp;
}
void FFmpegVideo::setCodecContext(AVCodecContext* adec_ctx){
    this->adec_ctx = adec_ctx;
}
void FFmpegVideo::play() {
    is_playing=1;
    pthread_create(&video_tid,NULL,video_run,this);
}
void FFmpegVideo::get(AVPacket* pkt){
    pthread_mutex_lock(&mutex);
    while (true){
        if(!queue.empty()){
            //此处有疑问
            //从队列取出一个packet，clone一个给入参对象
            if(av_packet_ref(pkt, queue.front())==0){
                LOGE("取出一帧视频");
            }
            //取成功了 弹出队列 销毁packet
            AVPacket *packet = queue.front();
            queue.pop();
            av_free(packet);
            break;
        }   else{
            LOGE("视频wait");
            pthread_cond_wait(&cond,&mutex);
        }
    }
    pthread_mutex_unlock(&mutex);

}
void FFmpegVideo::put(AVPacket* packet){
    AVPacket *packet1 = (AVPacket *) av_malloc(sizeof(AVPacket));

    if (av_copy_packet(packet1, packet)) {
        LOGE("克隆失败");
//        克隆失败
    }
    pthread_mutex_lock(&mutex);
    queue.push(packet1);
//    给消费者解锁
    pthread_cond_signal(&cond);
    LOGE("压入一帧视频数据");
    pthread_mutex_unlock(&mutex);
}
void FFmpegVideo::setAudio(FFmpegAudio* audio){
    this->audio=audio;
}




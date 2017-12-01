#include "FFmpegVideo.h"
#include "log.h"
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}
/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

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
        double timestamp;

        //判断是否有有效的pts
        if(packet->pts == AV_NOPTS_VALUE) {
            timestamp = 0;
        } else {
            timestamp = av_frame_get_best_effort_timestamp(frame)*av_q2d(video->time_base);
        }
        LOGE("差%ld",timestamp-video->audio->clock);
//计算帧率，平均每帧间隔时间
        double frameRate = av_q2d(video->time_base);
        frameRate += frame->repeat_pict * (frameRate * 0.5);
        if (timestamp == 0.0) {
            //按照默认帧率播放
//            av_usleep((unsigned long)(frameRate*1000));
        }else {
            if (fabs(timestamp - video->audio->clock) > AV_SYNC_THRESHOLD_MIN &&
                fabs(timestamp - video->audio->clock) < AV_NOSYNC_THRESHOLD) {
                //如果视频比音频快，延迟差值播放，否则直接播放，这里没有做丢帧处理
                if (timestamp > video->audio->clock) {
//                    av_usleep((unsigned long)((timestamp - video->audio->clock)*1000000));
                }
            }
        }
//        转码成rgb
        int code=sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0,
                           video->adec_ctx->height,
                           rgb_frame->data, rgb_frame->linesize);

        frame_call(rgb_frame);
    }
    pthread_exit(0);
}
FFmpegVideo::FFmpegVideo(){

}
double FFmpegVideo::synchronize(AVFrame *frame, double play) {
    //clock是当前播放的时间位置
    if (play != 0)
        clock=play;
    else //pst为0 则先把pts设为上一帧时间
        play = clock;
    //可能有pts为0 则主动增加clock
    //frame->repeat_pict = 当解码时，这张图片需要要延迟多少
    //需要求出扩展延时：
    //extra_delay = repeat_pict / (2*fps) 显示这样图片需要延迟这么久来显示
    double repeat_pict = frame->repeat_pict;
    //使用AvCodecContext的而不是stream的
    double frame_delay = av_q2d(time_base);
    //如果time_base是1,25 把1s分成25份，则fps为25
    //fps = 1/(1/25)
    double fps = 1 / frame_delay;
    //pts 加上 这个延迟 是显示时间
    double extra_delay = repeat_pict / (2 * fps);
    double delay = extra_delay + frame_delay;
//    LOGI("extra_delay:%f",extra_delay);
    clock += delay;
    return play;
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
            if(av_packet_ref(pkt, queue.front())){
                LOGE("取出一帧视频");
                break;
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




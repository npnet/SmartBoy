//
// Created by sine on 18-8-1.
//
#include "audioRecorder.h"

#include <stdio.h>

#include "tinycthread.h"

#include "malloc.h"
#include "VoiceConnectIf.h"
#include "alsa/asoundlib.h"
#include "AudioQueue.h"
#if 1

#define AUDIO_QUEUE_BUFF_LEN  (1024*10)

#define QUEUE_BUFF_MULTIPLE 1000

struct pcm_config conf;
#define AUDIO_QUEUE_BUFF_LEN  (1024*10)

#define QUEUE_BUFF_MULTIPLE 1000

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164


struct wav_header {
    uint32 riff_id;
    uint32 riff_sz;
    uint32 riff_fmt;
    uint32 fmt_id;
    uint32 fmt_sz;
    uint16 audio_format;
    uint16 num_channels;
    uint32 sample_rate;
    uint32 byte_rate;
    uint16 block_align;
    uint16 bits_per_sample;
    uint32 data_id;
    uint32 data_sz;
};

struct wav_header header;

typedef void (*record_audio_fn)(void *context,void *buf,int len,int err_code);

typedef struct _RecordData {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    audio_queue_t *queue;
    char *queue_buff;
    char *buffer;
    int buff_size;
    snd_pcm_uframes_t frames;
    record_audio_fn cb;
    void *user_data;
    pthread_t tid_pcm_read;
    pthread_t tid_queue_read;
    int runing;
    void *writer;
    r_pwrite rPwrite;

} RecordData;


enum
{
    SUCCESS								= 0,
    ERROR_FAIL							= -1,
    ERROR_EXCEPTION						= -2,

    /* General errors 10100(0x2774) */
            ERROR_GENERAL						= 10100,
    ERROR_OUT_OF_MEMORY					= 10101,
    ERROR_OPEN_FILE						= 10102,

};






void recorderCallback(void *context,void *data, int len,int err_code)
{

    FUNC_START
//写到一个文件
#if 1
#define OUT_PCM_NAME "1.pcm"

    if (SUCCESS == err_code) {

        FILE *fp = fopen(OUT_PCM_NAME, "ab+");
        if (NULL == fp) {
            printf("fopen error\n");
            return;
        }
        LOG("往文件写入数据长度=%d\n",len);
        fwrite(data, len, 1, fp);
        fclose(fp);
    }
//#else

    RecordData *recorder=(RecordData *)context;
    //typedef int (*r_pwrite)(void *_writer, const void *_data, unsigned long _sampleCout);
//    recorder->write(recorder->writer, recorder->recordingBuffer, recorder->recordingBufFrames);
    //对应recorderShortWrite
   // recorder->rPwrite(recorder->writer,recorder->buffer,conf.rate);
    //这地方得注意
    LOG("传入识别引擎数据长度len=%d\n",len);
    recorder->rPwrite(recorder->writer,data,len);


#endif
    FUNC_END
}


int initRecorder(int sampleRateInHz, int channels, int _audioFormat, int _bufferSize, void **_precorder){
    char sound_device_name[256];
    int card=0;
    int device=1;
    memset(&conf,0,sizeof(conf));
    snprintf(sound_device_name, sizeof(sound_device_name), "hw:%u,%u", card, device);

    conf.rate=sampleRateInHz;
    conf.channels=channels;
    strcpy(conf.device_name,sound_device_name);
    conf.period_count=4;
    conf.period_size=512;
#ifdef D_RECORD
    //每次采样多少位
    header.bits_per_sample =16;
    header.byte_rate = (header.bits_per_sample / 8) * conf.channels * conf.rate;
    header.block_align = conf.channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;
#endif
    LOG("录音设备初始化--->设备名:%s,采样频率=%d,通道=%d,period_count=%d,period_size=%d\n",conf.device_name,conf.rate,
           conf.channels,conf.period_count,conf.period_size);

    FUNC_END
    return 0;

}



static void *QueueReadThread(void *param) {
    RecordData *record = (RecordData *) param;
    FUNC_START
    int readLen = 0;

    while (record->runing) {
        char *data_buff = NULL;
        readLen = queue_read(record->queue, &data_buff);
        LOG("从队列读出数据长度readLen=%d\n", readLen);
        if (readLen<=0) {
            usleep(16000);
            continue;
        }
        /*
        if (record->buff_size != readLen) {
            //printf("\nqueue_read readLen %d\n", readLen);
        }
         */

        //回调
        record->cb(record,data_buff, readLen, 0);
        free(data_buff);
    }

    FUNC_END
    return NULL;
}


static void *RecordThread(void *param) {
    RecordData *record = (RecordData *) param;
    int ret = 0;
    int i=0,j=0;

    FUNC_START
    /*
    //绑定CPU核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0,&mask);
    ret = sched_setaffinity(0, sizeof(mask), &mask);
     */
    char *new_buffer=(char *)malloc(record->buff_size);
    //char *new_buffer=(char *)malloc(record->buff_size);
    LOG("分配读取数据缓冲：new_buffer size=%d\n",record->buff_size);
    //LOG("sched_setaffinity return = %d\n", ret);
    while (record->runing) {
        ret = snd_pcm_readi(record->handle, record->buffer, record->frames);

        LOG("从设备读出多少帧数据snd_pcm_readi=%d\n",ret);
        if (ret == -EPIPE) {
            /* EPIPE means overrun */
            LOG("overrun occurred\n");
            snd_pcm_prepare(record->handle);
            continue;
        } else if (ret < 0) {
            LOG("error from read: %s\n",snd_strerror(ret));
            record->cb(NULL,NULL, 0, ret);
            continue;
        } else if (ret != (int) record->frames) {
            LOG("short read, read %d frames\n", ret);
            continue;
        }
//=========
#if 0
        //增加通道号
        for(i=0;i<record->buff_size;i+=2){
            new_buffer[2*i + 0] = 0;
            new_buffer[2*i + 1] = j++%8 + 1;
            new_buffer[2*i + 2] = (record->buffer)[i];
            new_buffer[2*i + 3] = (record->buffer)[i+1];
        }
        //fwrite(new_buffer, record->buff_size*2, 1, fp);
        if(queue_write(record->queue, new_buffer, record->buff_size * 2) < 0){
            LOG("RecordThread write error\n");
            usleep(30*1000);
            queue_write(record->queue, new_buffer, record->buff_size * 2);
        }
//==========
#else
        // LOG("往队列写入数据长度=%d\n",record->buff_size);
#if 1
        for(i=0,j=0;i<ret*4;i+=4,j+=2){
            new_buffer[j]=(record->buffer)[i];
            new_buffer[j+1]=(record->buffer)[i+1];

        }
        if(queue_write(record->queue, new_buffer, ret*4/2 ) < 0){
            LOG("RecordThread write error\n");
            usleep(30*1000);
            queue_write(record->queue, new_buffer, ret*4/2 );
        }
#else

        //通道channels=2,采样位数为16,那么写入数据应该为帧数×channelsx16/8
        //LOG("往队列写入数据长度=%d\n",ret*4);
        //queue_write(record->queue, record->buffer, ret*4);

        //通道channels=1,采样位数为16,那么写入数据应该为帧数×1x16/8
        LOG("往队列写入数据长度=%d\n",ret*2);
        queue_write(record->queue, record->buffer, ret*2);

#endif
#endif
    }
    if(new_buffer != NULL){
        free(new_buffer);
        new_buffer = NULL;
    }


    FUNC_END
    return NULL;
}

/************************************************************************
 * 开始录音
 *@_writer :VoiceRecognizer
 ************************************************************************/
int startRecord(void *_recorder, void *_writer, r_pwrite _pwrite){
    int rc;
    int ret;
    unsigned int val;
    int dir;
    int size;
    pthread_attr_t thread_attr;
    struct sched_param thread_param;
    snd_pcm_hw_params_t *params;
    struct VoiceRecognizer *recognizer;

    snd_pcm_uframes_t frames;
    char *buffer;
    RecordData *record = NULL;
    void *audio_queue_buff = NULL;

    FUNC_START
    record = (RecordData *) malloc(sizeof(RecordData));
    audio_queue_buff = malloc(sizeof(audio_queue_t) + AUDIO_QUEUE_BUFF_LEN + 1);
    if (NULL == record) {
        return -1;
    }
    memset(record, 0, sizeof(RecordData));

    //回调函数
    record->cb = recorderCallback;

    record->rPwrite=_pwrite;
    record->writer=_writer;
    //设置录音参数参数
    rc = snd_pcm_open(&record->handle, conf.device_name, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        LOG("unable to open pcm device: %s\n", snd_strerror(rc));
        ret = -1;
        goto error;
    }

    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(record->handle, params);

    /* 设置参数 */
    /* Interleaved mode */
    snd_pcm_hw_params_set_access(record->handle, params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    //设置音频采集格式
    snd_pcm_hw_params_set_format(record->handle, params,SND_PCM_FORMAT_S16_LE);
    //snd_pcm_hw_params_set_format(record->handle, params,SND_PCM_FORMAT_S16_BE);

    //设置通道
    snd_pcm_hw_params_set_channels(record->handle, params, conf.channels);

    //采样率
    val = conf.rate;
    snd_pcm_hw_params_set_rate_near(record->handle, params, & val,&dir);

    //设置每次中断，产生多少帧数据
    frames = conf.period_size;
    snd_pcm_hw_params_set_period_size_near(record->handle, params, &frames, &dir);
    record->frames = frames;

    /* 把参数写进驱动 */
    rc = snd_pcm_hw_params(record->handle, params);
    if (rc < 0) {
        LOG("unable to set hw parameters: %s\n",snd_strerror(rc));
        return -1;
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    //分配录音缓冲大小
    size = frames * 8;
    record->buff_size = size;
    LOG("frames=%d,record->buff_size=%d\n", frames, record->buff_size);
    record->buffer = (char *) malloc(size);
    if (NULL == record->buffer) {
        ret = -1;
        goto error;
    }

    //？
    record->queue_buff = (char *) malloc(sizeof(audio_queue_t) + size * QUEUE_BUFF_MULTIPLE + 1);
    if (NULL == record->queue_buff) {
        ret = -1;
        goto error;
    }

    //初始化缓冲队列
    record->queue = queue_init(record->queue_buff, size * QUEUE_BUFF_MULTIPLE + 1);
    pthread_attr_init(&thread_attr);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    thread_param.sched_priority = sched_get_priority_max(SCHED_RR);
    pthread_attr_setschedparam(&thread_attr, &thread_param);
    record->runing = 1;

    //创建录音线程和队列线程
    //LOG("record_start record :%x\n", record);
    ret=pthread_create(&record->tid_pcm_read, &thread_attr, RecordThread, (void *) record);
    if(0!=ret){
        LOG("Create RecordThread err\n");
        goto error;
    }
    ret=pthread_create(&record->tid_queue_read, NULL, QueueReadThread, (void *) record);
    if(0!=ret){
        LOG("Create RecordThread err\n");
        goto error;
    }

    return ret;

    error:
    printf("record start error %d\n", ret);
    stopRecord(record);

    exit:
    printf("record start out %d\n", ret);
    free(audio_queue_buff);

    return ret;



    FUNC_END
}

/************************************************************************
 * 停止录音，该函数需同步返回（销毁函数是真正停止后才能释放内存）
 ************************************************************************/
int stopRecord(void *recorder){
    FUNC_START
    RecordData *record = (RecordData *) recorder;
    LOG("\nrecord_stop in record:%x\n\n", record);
    if (NULL != record) {
        record->runing = 0;
        pthread_join(record->tid_pcm_read, NULL);
        pthread_join(record->tid_queue_read, NULL);
        snd_pcm_drain(record->handle);
        snd_pcm_close(record->handle);

        if (NULL != record->queue) {
            queue_destroy(record->queue);
        }
        if (NULL != record->queue_buff) {
            free(record->queue_buff);
        }
        if (NULL != record->buffer) {
            free(record->buffer);
        }
        free(record);
    }
    FUNC_END
}

/************************************************************************
 * 释放录音器的资源
 ************************************************************************/
 int releaseRecorder(void *_recorder){

}
#endif
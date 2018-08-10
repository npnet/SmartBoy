//
// Created by sine on 18-8-2.
//
#include <stdio.h>
#include <PROC_VOICE/include/audioRecorder.h>

#include "tinycthread.h"

#include "malloc.h"
#include "VoiceConnectIf.h"
#include "alsa/asoundlib.h"
#include "AudioQueue.h"
#if 0
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;
#define androidPlayQueueBufferCount 3
#define doubleBufferCount 4

extern unsigned long prePerforceTime;
extern unsigned long currPerforceTime;

static SLObjectItf outputMixObject = NULL;


static FILE * gFile = NULL;

static int playerRef = 0;
static int recorderRef = 0;
typedef unsigned int SLresult;

struct SLObjectItf_ {
    SLresult (*Realize) (
            SLObjectItf self,
            SLboolean async
    );
    SLresult (*Resume) (
            SLObjectItf self,
            SLboolean async
    );
    SLresult (*GetState) (
            SLObjectItf self,
            SLuint32 * pState
    );
    SLresult (*GetInterface) (
            SLObjectItf self,
            const SLInterfaceID iid,
            void * pInterface
    );
    SLresult (*RegisterCallback) (
            SLObjectItf self,
            slObjectCallback callback,
            void * pContext
    );
    void (*AbortAsyncOperation) (
            SLObjectItf self
    );
    void (*Destroy) (
            SLObjectItf self
    );
    SLresult (*SetPriority) (
            SLObjectItf self,
            SLint32 priority,
            SLboolean preemptable
    );
    SLresult (*GetPriority) (
            SLObjectItf self,
            SLint32 *pPriority,
            SLboolean *pPreemptable
    );
    SLresult (*SetLossOfControlInterfaces) (
            SLObjectItf self,
            SLint16 numInterfaces,
            SLInterfaceID * pInterfaceIDs,
            SLboolean enabled
    );
};

typedef const struct SLObjectItf_ * const * SLObjectItf;


struct SLRecordItf_;
typedef const struct SLRecordItf_ * const * SLRecordItf;

typedef void (SLAPIENTRY *slRecordCallback) (
    SLRecordItf caller,
    void *pContext,
        SLuint32 event
);

/** Recording interface methods */
struct SLRecordItf_ {
    SLresult (*SetRecordState) (
            SLRecordItf self,
            SLuint32 state
    );
    SLresult (*GetRecordState) (
            SLRecordItf self,
            SLuint32 *pState
    );
    SLresult (*SetDurationLimit) (
            SLRecordItf self,
            SLmillisecond msec
    );
    SLresult (*GetPosition) (
            SLRecordItf self,
            SLmillisecond *pMsec
    );
    SLresult (*RegisterCallback) (
            SLRecordItf self,
            slRecordCallback callback,
            void *pContext
    );
    SLresult (*SetCallbackEventsMask) (
            SLRecordItf self,
            SLuint32 eventFlags
    );
    SLresult (*GetCallbackEventsMask) (
            SLRecordItf self,
            SLuint32 *pEventFlags
    );
    SLresult (*SetMarkerPosition) (
            SLRecordItf self,
            SLmillisecond mSec
    );
    SLresult (*ClearMarkerPosition) (
            SLRecordItf self
    );
    SLresult (*GetMarkerPosition) (
            SLRecordItf self,
            SLmillisecond *pMsec
    );
    SLresult (*SetPositionUpdatePeriod) (
            SLRecordItf self,
            SLmillisecond mSec
    );
    SLresult (*GetPositionUpdatePeriod) (
            SLRecordItf self,
            SLmillisecond *pMsec
    );
};


typedef struct SLAndroidSimpleBufferQueueState_ {
    SLuint32	count;
    SLuint32	index;
} SLAndroidSimpleBufferQueueState;


struct SLAndroidSimpleBufferQueueItf_ {
    SLresult (*Enqueue) (
            SLAndroidSimpleBufferQueueItf self,
            const void *pBuffer,
            SLuint32 size
    );
    SLresult (*Clear) (
            SLAndroidSimpleBufferQueueItf self
    );
    SLresult (*GetState) (
            SLAndroidSimpleBufferQueueItf self,
            SLAndroidSimpleBufferQueueState *pState
    );
    SLresult (*RegisterCallback) (
            SLAndroidSimpleBufferQueueItf self,
            slAndroidSimpleBufferQueueCallback callback,
            void* pContext
    );
};



struct AudioRecorderInfo
{

    char *recordingBuffer;
    int recordingBufLen;
    int recordingBufFrames;
    void *writer;
    r_pwrite write;

    SLObjectItf recorderObject;
   // SLRecordItf recorderRecord;
   // SLAndroidSimpleBufferQueueItf recorderBufferQueue;

};

struct AudioPlayerInfo
{
    int finishedBufferCount;
    int enquedBufferCount;

    int buffersFilled;

    struct Buffer doubleBuffer;
    struct BufferData *playingBuffer;
    mtx_t playingBufMtx;

   // SLObjectItf bqPlayerObject;
   // SLPlayItf bqPlayerPlay;
   // SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

};


struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};
#endif
struct pcm_config conf;
#define AUDIO_QUEUE_BUFF_LEN  (1024*10)

#define QUEUE_BUFF_MULTIPLE 1000


typedef void (*record_audio_fn)(void *buf,int len,int err_code);

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
} RecordData;



void recorderCallback(void *data, int len,int code)
{
#if 0
    struct AudioRecorderInfo *recorder = (struct AudioRecorderInfo *)context;
    myassert(bq == recorder->recorderBufferQueue);

    SLresult result;
    recorder->write(recorder->writer, recorder->recordingBuffer, recorder->recordingBufFrames);

    result = (*recorder->recorderBufferQueue)->Enqueue(recorder->recorderBufferQueue, recorder->recordingBuffer,
                                                       recorder->recordingBufLen);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

#ifdef SAVE_PCM_DATA
    if(gFile != NULL)
    {
    	fwrite(recorder->recordingBuffer, recorder->recordingBufLen, 1, gFile);
    }
#endif
#endif
}



int initRecorder(int sampleRateInHz, int channel, int audioFormat, int bufferSize, void **_precorder)
{
    FUNC_START
#if 0
    androidLog("initRecorder");
    createOpenSLEngine();

    recorderRef ++;

    struct AudioRecorderInfo *recorder = mymalloc(sizeof(struct AudioRecorderInfo));

    recorder->recordingBufFrames = _bufferSize;
    recorder->recordingBufLen = _bufferSize * (_audioFormat/8) * _channel;
    recorder->recordingBuffer = mymalloc(recorder->recordingBufLen);
    //recorder->recorderObject = NULL;
    recorder->writer = NULL;
    recorder->write = NULL;
    *_precorder = recorder;

    SLresult result;

    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, ((SLuint32)_sampleRateInHz) * 1000,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorder->recorderObject, &audioSrc,
                                                  &audioSnk, 1, id, req);
    androidLog("CreateAudioRecorder xxxxxxxxxxxxx:%d", result);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

    result = (*recorder->recorderObject)->Realize(recorder->recorderObject, SL_BOOLEAN_FALSE);
    androidLog("recorder Realize:%d", result);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

    result = (*recorder->recorderObject)->GetInterface(recorder->recorderObject, SL_IID_RECORD, &recorder->recorderRecord);
    androidLog("GetInterface SL_IID_RECORD:%d", result);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

    result = (*recorder->recorderObject)->GetInterface(recorder->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                       &recorder->recorderBufferQueue);
    androidLog("GetInterface SL_IID_ANDROIDSIMPLEBUFFERQUEUE:%d", result);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

    result = (*recorder->recorderBufferQueue)->RegisterCallback(recorder->recorderBufferQueue, recorderCallback,
                                                                recorder);
    androidLog("recorder RegisterCallback:%d", result);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

    return 0;
#endif

    char sound_device_name[256];
    int card=0;
    int device=1;
    memset(&conf,0,sizeof(conf));
    snprintf(sound_device_name, sizeof(sound_device_name), "hw:%u,%u", card, device);
    conf.rate=sampleRateInHz;
    conf.channels=channel;
    conf.device_name=sound_device_name;
    conf.period_count=4;
    conf.period_size=1024;

    FUNC_END
    return 0;
}

static void *QueueReadThread(void *param) {
    RecordData *record = (RecordData *) param;
    FUNC_START
    int readLen = 0;
    printf("QueueReadThread record :%x\n", record);
    while (record->runing) {
        char *data_buff = NULL;
        readLen = queue_read(record->queue, &data_buff);

        if (readLen<=0) {
            //printf("queue_read readLen = 0\n");
            usleep(16000);
            continue;
        }
        /*
        if (record->buff_size != readLen) {
            //printf("\nqueue_read readLen %d\n", readLen);
        }
         */

        //回调
        record->cb(data_buff, readLen, 0);
        free(data_buff);
    }
    printf("QueueReadThread end \n");
    FUNC_END
    return NULL;
}


static void *RecordThread(void *param) {
    RecordData *record = (RecordData *) param;
    int ret = 0;
    int i=0,j=0;
    cpu_set_t mask;
    FUNC_START
    printf("RecordThread record:%x\n", record);
    // CPU_ZERO(&mask);
    // CPU_SET(0,&mask);
    // ret = sched_setaffinity(0, sizeof(mask), &mask);
    char *new_buffer=(char *)malloc(record->buff_size*2);

    //LOG("sched_setaffinity return = %d\n", ret);
    while (record->runing) {
        ret = snd_pcm_readi(record->handle, record->buffer, record->frames);
        //printf("snd_pcm_readi return = %d\n", ret);
        if (ret == -EPIPE) {
            /* EPIPE means overrun */
            LOG("overrun occurred\n");
            snd_pcm_prepare(record->handle);
            continue;
        } else if (ret < 0) {
            LOG("error from read: %s\n",snd_strerror(ret));
            record->cb(NULL, 0, ret);
            continue;
        } else if (ret != (int) record->frames) {
            LOG("short read, read %d frames\n", ret);
            continue;
        }
//=========
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
        // queue_write(record->queue, record->buffer, record->buff_size);
    }
    if(new_buffer != NULL){
        free(new_buffer);
        new_buffer = NULL;
    }

    printf("RecordThread end\n");
    FUNC_END
    return NULL;
}

int startRecord(void *_recorder, void *_writer, r_pwrite _pwrite)
{
    FUNC_START
#if 0
    androidLog("startRecord");
    struct AudioRecorderInfo *recorder = (struct AudioRecorderInfo *)_recorder;
    recorder->writer = _writer;
    recorder->write = _pwrite;

    SLresult result;

    result = (*recorder->recorderRecord)->SetRecordState(recorder->recorderRecord, SL_RECORDSTATE_STOPPED);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;
    result = (*recorder->recorderBufferQueue)->Clear(recorder->recorderBufferQueue);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

    result = (*recorder->recorderBufferQueue)->Enqueue(recorder->recorderBufferQueue, recorder->recordingBuffer,
                                                       recorder->recordingBufLen);

    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

    result = (*recorder->recorderRecord)->SetRecordState(recorder->recorderRecord, SL_RECORDSTATE_RECORDING);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

#ifdef SAVE_PCM_DATA
    if(gFile == NULL)
    {

    	    	char *pcmFileName = "/data/data/com.example.wifilist/cache/data.pcm";
    	    	gFile = fopen(pcmFileName, "wb");
    	    	androidLog("%s open:%s\n", pcmFileName, (gFile?"success":"fail"));
    	myassert(gFile != NULL);
    }
#endif

    return 0;
#endif
    int rc;
    int ret;
    int val;
    int dir;
    int size;
    pthread_attr_t thread_attr;
    struct sched_param thread_param;
    snd_pcm_hw_params_t *params;

    snd_pcm_uframes_t frames;
    char *buffer;
    RecordData *record = NULL;
    void *audio_queue_buff = NULL;

    record = (RecordData *) malloc(sizeof(RecordData));
    audio_queue_buff = malloc(sizeof(audio_queue_t) + AUDIO_QUEUE_BUFF_LEN + 1);
    if (NULL == record) {
        return -1;
    }
    memset(record, 0, sizeof(RecordData));
    //回调函数
    record->cb = recorderCallback;

    //设置录音参数参数
    rc = snd_pcm_open(&record->handle, conf.device_name, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        LOG("unable to open pcm device: %s/n", snd_strerror(rc));
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
    snd_pcm_hw_params_set_format(record->handle, params,
                                 SND_PCM_FORMAT_S16_LE);

    //设置通道
    snd_pcm_hw_params_set_channels(record->handle, params, conf.channels);

    //采样率
    val = conf.rate;
    snd_pcm_hw_params_set_rate_near(record->handle, params, &val, &dir);

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
    LOG("record_start record :%x\n", record);
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

    goto exit;

error:
    printf("record start error %d\n", ret);
    stopRecord(record);

exit:
    printf("record start out %d\n", ret);
    free(audio_queue_buff);

    return ret;



    FUNC_END

}

int stopRecord(void *_recorder)
{
    FUNC_START
#if 0
    struct AudioRecorderInfo *recorder = (struct AudioRecorderInfo *)_recorder;
    SLresult result;

    result = (*recorder->recorderRecord)->SetRecordState(recorder->recorderRecord, SL_RECORDSTATE_STOPPED);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;
    result = (*recorder->recorderBufferQueue)->Clear(recorder->recorderBufferQueue);
    myassert(SL_RESULT_SUCCESS == result);
    (void)result;

#ifdef SAVE_PCM_DATA
    if(gFile != NULL)
    {
    	fclose(gFile);
    	gFile = NULL;
    }
#endif

    return 0;
#endif


    FUNC_END
}

int releaseRecorder(void *_recorder)
{
    FUNC_START
#if 0
    recorderRef --;

    struct AudioRecorderInfo *recorder = (struct AudioRecorderInfo *)_recorder;

    myfree(recorder->recordingBuffer);
    if (recorder->recorderObject != NULL) {
        (*recorder->recorderObject)->Destroy(recorder->recorderObject);
        recorder->recorderObject = NULL;
        recorder->recorderRecord = NULL;
        recorder->recorderBufferQueue = NULL;
    }
    myfree(recorder);

    destoryOpenSLEngine();
#endif
    FUNC_END
    return 0;
}

//====================基于alsa-lib==========================

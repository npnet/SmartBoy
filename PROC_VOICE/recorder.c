//
// Created by sine on 18-8-2.
//
#include <stdio.h>
#include <PROC_VOICE/include/audioRecorder.h>
#include "tinycthread.h"
#include "asoundlib.h"
#include "malloc.h"
#include "VoiceConnectIf.h"
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
struct recorder{
    int fd;
    int channels;   //通道
    int sampleRate; //采样率
    int frames;
    int bits;       //多少位
    int period_size;//
    int period_count;//
};

/*
unsigned int capture_sample(FILE *file, unsigned int card, unsigned int device,
                            unsigned int channels, unsigned int rate,
                            enum pcm_format format, unsigned int period_size,
                            unsigned int period_count)
{
    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    unsigned int size;
    unsigned int bytes_read = 0;

    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    config.format = format;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    pcm = pcm_open(card, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n",
                pcm_get_error(pcm));
        return 0;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return 0;
    }

    printf("Capturing sample: %u ch, %u hz, %u bit\n", channels, rate,
           pcm_format_to_bits(format));

    while (capturing && !pcm_read(pcm, buffer, size)) {
        if (fwrite(buffer, 1, size, file) != size) {
            fprintf(stderr,"Error capturing sample\n");
            break;
        }
        bytes_read += size;
    }

    free(buffer);
    pcm_close(pcm);
    return pcm_bytes_to_frames(pcm, bytes_read);
}
*/

typedef struct SLAndroidSimpleBufferQueueItf_T{

}SLAndroidSimpleBufferQueueItf;

typedef enum RecordStatus_T{
    Capturing=0,
}RecordStatus;

typedef enum RecordCtrl_T{
    Capture=0
}RecordCtrl;


struct AudioRecorderInfo
{

    char *recordingBuffer;
    int recordingBufLen;
    int recordingBufFrames;
    void *writer;
    r_pwrite write;

    struct pcm *rpcm;
    struct pcm_config *config;

    //SLObjectItf recorderObject;
    // SLRecordItf recorderRecord;
    // SLAndroidSimpleBufferQueueItf recorderBufferQueue;

};

struct AudioRecorderInfo aRecorderInfo;

//struct pcm RecordPcm;
void recorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
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

int initRecorder(int _sampleRateInHz, int _channel, int _audioFormat, int _bufferSize, void **_precorder)
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

    struct pcm_config config;
    struct pcm *pcm;
    char *buffer;
    unsigned int size;
    unsigned int bytes_read = 0;

    unsigned int card=0;
    unsigned int device=2;


    aRecorderInfo.config->channels = _channel;//通道
    aRecorderInfo.config->rate = _sampleRateInHz;
    aRecorderInfo.config->period_size = 1024;//中断一次产生多少帧数据
    aRecorderInfo.config->period_count = 4;//一个buffer需要多少次中断
    aRecorderInfo.config->format = _audioFormat;
    aRecorderInfo.config->start_threshold = 0;
    aRecorderInfo.config->stop_threshold = 0;
    aRecorderInfo.config->silence_threshold = 0;

    aRecorderInfo.rpcm = pcm_open(card, device, PCM_IN, &config);
    if (!aRecorderInfo.rpcm || !pcm_is_ready(aRecorderInfo.rpcm)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n",
                pcm_get_error(aRecorderInfo.rpcm));
        return 0;
    }

    size = pcm_frames_to_bytes(aRecorderInfo.rpcm, pcm_get_buffer_size(aRecorderInfo.rpcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(aRecorderInfo.rpcm);
        return 0;
    }

  //  printf("Capturing sample: %u ch, %u hz, %u bit\n", config.channels, config.rate,
  //         pcm_format_to_bits(config.format));
    FUNC_END
    return 0;
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
    struct pcm *pcm;

    pcm_start(aRecorderInfo.rpcm);
    /*
    while (capturing && !pcm_read(pcm, buffer, size)) {

        if (fwrite(buffer, 1, size, file) != size) {
            fprintf(stderr,"Error capturing sample\n");
            break;
        }
        bytes_read += size;

    }*/


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

    //struct pcm *pcm;

    //pcm_stop(pcm);
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


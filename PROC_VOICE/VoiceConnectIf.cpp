//
// Created by sine on 18-7-21.
//

#include <assert.h>

#include <string.h>
#include <pthread.h>
#include "stdio.h"
#include "string.h"

#include "../include/voice_encoder_VoicePlayer.h"


#include "../include/voicePlay.h"
#include "../include/voiceRecog.h"
#include "../include/audioRecorder.h"
#include "../include/util.h"
#include "../include/memory.h"
#include "../include/common.h"

static void *player = NULL;


static void *recognizer = NULL;


static pthread_t recogTid = NULL;
static void *recorder = NULL;
static int playerFreqs[19];
static int recognizerFreqs[19];
static int recognizerFreqsChanged = 0;
static int recognizerSampleRate = 44100;
char *javaBuf = NULL;

#define MYPLAYER

void playerStart(void *_listener) {
    /*
    printf("playerStart");
    assert(jplayer != NULL);
    JNIEnv *env = NULL;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    jclass thisClass = (*env)->GetObjectClass(env, jplayer);
    printf("voice.encoder.VoicePlayer class:%p\n", thisClass);
    jmethodID jmPlayStart = (*env)->GetMethodID(env, thisClass, "onPlayStart", "()V");
    printf("voice.encoder.VoicePlayer onPlayStart method:%p\n", jmPlayStart);
    (*env)->DeleteLocalRef(env, thisClass);
    (*env)->CallVoidMethod(env, jplayer, jmPlayStart);
    (*jvm)->DetachCurrentThread(jvm);
     */
}

void playerEnd(void *_listener) {
    /*
    printf("playerEnd");
    assert(jplayer != NULL);
    JNIEnv *env = NULL;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    jclass thisClass = (*env)->GetObjectClass(env, jplayer);
    printf("voice.encoder.VoicePlayer class:%p\n", thisClass);
    jmethodID jmPlayEnd = (*env)->GetMethodID(env, thisClass, "onPlayEnd", "()V");
    printf("voice.encoder.VoicePlayer onPlayEnd method:%p\n", jmPlayEnd);
    (*env)->DeleteLocalRef(env, thisClass);
    (*env)->CallVoidMethod(env, jplayer, jmPlayEnd);
     */
}

void voice_encoder_VoicePlayer_init(int _sampleRate) {
    printf("voice_encoder_VoicePlayer_init:%d", _sampleRate);

    if (player == NULL) {
        player = vp_createVoicePlayer2(_sampleRate);
        assert(player != NULL);
        vp_setPlayerListener(player, NULL, playerStart, playerEnd);
    }
}

void voice_encoder_VoicePlayer_play(string _text, int _playCount, int _muteInterval)
{
    printf("voice_encoder_VoicePlayer_play");
    assert(player != NULL);
    const char *text = (*env)->GetStringUTFChars(env, _text, NULL);
    printf("play text:%s, len:%d", text, strlen(text));
    vp_play(player, text, strlen(text), _playCount, _muteInterval
    );

    printf("voice_encoder_VoicePlayer_play out");
}

void voice_encoder_VoicePlayer_playex(intArray _freqs, int _duration, int _playCount, int _muteInterval, bool _dualTone)
{
    int *freqs = (*env)->GetIntArrayElements(env, _freqs, false);
    jsize len = (*env)->GetArrayLength(env, _freqs);
    vp_playex(player, freqs, len, _duration, _playCount, _muteInterval, _dualTone
    );
}

void voice_encoder_VoicePlayer_save(string _fileName, string _text, int _playCount, int _muteInterval)
{
    printf("voice_encoder_VoicePlayer_save");
    assert(player
           != NULL);
    const char *text = (*env)->GetStringUTFChars(env, _text, NULL);
    const char *fileName = (*env)->GetStringUTFChars(env, _fileName, NULL);
    printf("save text:%s, len:%d to %s", text, strlen(text), fileName);
    vp_save(player, fileName, text, strlen(text), _playCount, _muteInterval);

    printf("voice_encoder_VoicePlayer_save out");
}

void voice_encoder_VoicePlayer_setVolume(double _volume) {
    vp_setVolume(player, _volume);
}

void voice_encoder_VoicePlayer_setFreqs(intArray _freqs)
{
    int *freqs = (*env)->GetIntArrayElements(env, _freqs, false);
    jsize len = (*env)->GetArrayLength(env, _freqs);
    assert(len == sizeof(playerFreqs) / sizeof(int));
    memcpy(playerFreqs, freqs, sizeof(playerFreqs));

    vp_setFreqs(player, playerFreqs, sizeof(playerFreqs) / sizeof(int));
}

void voice_encoder_VoicePlayer_stop()
{
    if (player != NULL) {
        printf("stop player");
        while (!vp_isStopped(player)) {
            vp_stop(player);
            printf("wait player to stop");
            mysleep(5);
        }
    }
}

bool voice_encoder_VoicePlayer_isStopped()
{
    return ((player == NULL || vp_isStopped(player)) ? true : false);
}

void voice_encoder_VoicePlayer_setPlayerType(int _type) {
    printf("voice_encoder_VoicePlayer_setPlayerType:%d", _type);
    assert(player != NULL);
    if (_type == 1)
        vp_setPlayerType(player, VP_SoundPlayer);
    else if (_type == 2)
        vp_setPlayerType(player, VP_WavPlayer);
}

void voice_encoder_VoicePlayer_setWavPlayer(string _fileName) {
    printf("voice_encoder_VoicePlayer_setWavPlayer");
    assert(player != NULL);
    const char *fileName = (*env)->GetStringUTFChars(env, _fileName, NULL);
    printf("setWavPlayer:%s", fileName);
    vp_setWavPlayer(player, fileName);

}

void voice_encoder_VoicePlayer_mixWav(string _wavFileName, float _volume, int _muteInterval) {
    printf("voice_encoder_VoicePlayer_mixWav");
    assert(player != NULL);
    const char *wavFileName = (*env)->GetStringUTFChars(env, _wavFileName, NULL);
    printf("mixWav:%s, muteInterval:%d", wavFileName, _muteInterval);
    vp_mixWav(player, wavFileName, _volume, _muteInterval
    );
}

void voice_encoder_VoicePlayer_mixAssetWav(jobject _assMgr, string _wavFileName, float _volume, int _muteInterval)
{

   
        char *buffer = (char *) mymalloc(bufferSize + 1);
        buffer[bufferSize] = 0;
        printf("mixAssetWav alloc buffer %d at %p", bufferSize + 1, buffer);
        int numBytesRead = AAsset_read(asset, buffer, bufferSize);

        printf("mixAssetWav file size:%d, read size:%d\n", bufferSize, numBytesRead);
        if(numBytesRead <= 44)
        {
            printf("asset size:%d error", numBytesRead);
            myfree(buffer);
            return;
        }
        memmove(buffer, buffer+44, numBytesRead-44);
        vp_mixWav2(player, buffer, numBytesRead-44, _volume, _muteInterval);

        AAsset_close(asset);

}

void recognizerStart(void *_listener, float _soundTime)
{

}

void recognizerEnd(void *_listener, float _soundTime, int _recogStatus, char *_data, int _dataLen)
{
}

#define MAX_MATCH_FREQ_COUNT 8

void recognizerMatch(void *_listener, int _timeIdx, struct VoiceMatch *_matches, int _matchesLen) {
    assert(jrecognizer != NULL);
    JNIEnv *env = NULL;
    int i;
    short freqs[MAX_MATCH_FREQ_COUNT];
    short lens[MAX_MATCH_FREQ_COUNT];
    float strengths[MAX_MATCH_FREQ_COUNT];
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    jclass thisClass = (*env)->GetObjectClass(env, jrecognizer);
    jmethodID jmRecognizeMatch = (*env)->GetMethodID(env, thisClass, "onRecognizeMatch", "(II[S[S[F)V");
    (*env)->DeleteLocalRef(env, thisClass);
    for (i = 0; i < _matchesLen; i++) {
        freqs[i] = _matches[i].frequency;
        lens[i] = _matches[i].length;
        strengths[i] = _matches[i].strength;
        printf("match frequency %d length:%d, strength:%.2f from %d to %d\n", _matches[i].frequency, _matches[i].length,
               _matches[i].strength, _timeIdx - _matches[i].length, _timeIdx);
    }
    jshortArray jfreqsArray = (*env)->NewShortArray(env, _matchesLen);
    (*env)->SetShortArrayRegion(env, jfreqsArray, 0, _matchesLen, freqs);
    jshortArray jlensArray = (*env)->NewShortArray(env, _matchesLen);
    (*env)->SetShortArrayRegion(env, jlensArray, 0, _matchesLen, lens);
    floatArray jstrengthsArray = (*env)->NewFloatArray(env, _matchesLen);
    (*env)->SetFloatArrayRegion(env, jstrengthsArray, 0, _matchesLen, strengths);
    (*env)->CallVoidMethod(env, jrecognizer, jmRecognizeMatch, _timeIdx, _matchesLen, jfreqsArray, jlensArray,
                           jstrengthsArray);
    (*jvm)->DetachCurrentThread(jvm);
}

void voice_decoder_VoiceRecognizer_init(int _sampleRate) {
    printf("voice_decoder_VoiceRecognizer_init");
    if (jrecognizer != NULL) {
        assert(jrecognizer_env!= NULL);
        (*jrecognizer_env)->DeleteGlobalRef(jrecognizer_env, jrecognizer);
        jrecognizer = NULL;
    }
    recognizerFreqsChanged = false;
    jrecognizer = (*env)->NewGlobalRef(env, thiz);
    jrecognizer_env = env;
    recognizerSampleRate = _sampleRate;
    assert(jrecognizer != NULL);
}

void voice_decoder_VoiceRecognizer_setFreqs(int _freqs[]) {
    int *freqs = __freqs;
    jsize len = (*env)->GetArrayLength(env, _freqs);
    assert(len == sizeof(recognizerFreqs) / sizeof(int));
    memcpy(recognizerFreqs, freqs, sizeof(recognizerFreqs));
    (*env)->
            ReleaseIntArrayElements(env, _freqs, freqs,
                                    0);
    recognizerFreqsChanged = true;

    {
        char buffer[256];
        int i, ic = len;
        buffer[0] = 0;
        for (i = 0; i < ic; i++)
        {
            sprintf(buffer + strlen(buffer), "%d,", recognizerFreqs[i]);
        }
        printf("%s", buffer);
    }
}

#endif


//===========
int recorderShortWrite(void *recognizer, const char *data, unsigned long _sampleCout) {
    const int bytePerFrame = 2;
    return vr_writeData(recognizer, data, ((int) _sampleCout) * bytePerFrame);
}

void *runRecorderVoiceRecognize(void *_recognizer) {
    vr_runRecognizer(_recognizer);
}

void voice_decoder_VoiceRecognizer_start(int _minBufferSize) {
    printf("voice_decoder_VoiceRecognizer_start(%d)", _minBufferSize);
    if (recognizer != NULL && !vr_isRecognizerStopped(recognizer))
        return;

    printf("recognizerFreqs(%d):%d", sizeof(recognizerFreqs) / sizeof(int), recognizerFreqs[0]);
    if (recognizer == NULL) {
#if ((defined(FREQ_ANALYSE_TIME_MATCH2) && defined(TV_LIB)) || defined(TEST_MATCH_FREQ))
        recognizer = vr_createVoiceRecognizer2(CPUUsePriority, recognizerSampleRate);
#else
        recognizer = vr_createVoiceRecognizer2(MemoryUsePriority, recognizerSampleRate);
#endif


#ifdef CALLBACK_MATCH_EVENT
        vr_setRecognizerListener2(recognizer, NULL, recognizerStart, recognizerEnd, recognizerMatch);
#else
        vr_setRecognizerListener(recognizer, NULL, recognizerStart, recognizerEnd
        );
#endif
        if (recognizerFreqsChanged)
            vr_setRecognizeFreqs(recognizer, recognizerFreqs,
                                 sizeof(recognizerFreqs) / sizeof(int));

        assert(recorder
               == NULL);

        int r = initRecorder(recognizerSampleRate, 1, 16, _minBufferSize, &recorder);
        if (r != 0) {
            printf("recorder init error:%d", r);
            return;
        }

        r = startRecord(recorder, recognizer, recorderShortWrite);
        if (r != 0) {
            printf("recorder record error:%d", r);
            return;
        }

        assert(recogTid
               == NULL);
        pthread_create(&recogTid, NULL, runRecorderVoiceRecognize, recognizer
        );
    }
}

void voice_decoder_VoiceRecognizer_pause(int _microSeconds) {
    printf("voice_decoder_VoiceRecognizer_pause(%d)", _microSeconds);
    if (recognizer == NULL || vr_isRecognizerStopped(recognizer))
        return;
    vr_pauseRecognize(recognizer, _microSeconds
    );
}

void voice_decoder_VoiceRecognizer_stop() {
    printf("voice_decoder_VoiceRecognizer_stop, recorder:%p, recognizer:%p", recorder, recognizer);
    if (recorder != NULL) {

        int r = stopRecord(recorder);
        printf("recorder stop result:%d", r);
        r = releaseRecorder(recorder);
        printf("recorder release result:%d", r);
        recorder = NULL;
    }

    if (recognizer != NULL) {
        if (recogTid != NULL) {
            vr_stopRecognize(recognizer);
            pthread_join(recogTid, NULL
            );
            printf("recognize thread:%ld quit", recogTid);
            recogTid = NULL;
        }
        vr_destroyVoiceRecognizer(recognizer);
        printf("recognizer destory");
        recognizer = NULL;
    }

    if (javaBuf != NULL) {
        free(javaBuf);
        javaBuf = NULL;
    }
}

bool voice_decoder_VoiceRecognizer_isStopped() {
    return (recognizer == NULL || vr_isRecognizerStopped(recognizer)) ? true : false;
}


int voice_decoder_VoiceRecognizer_writeBuf(char * _audio, int _dataSize) {
    if (javaBuf == NULL)
        javaBuf =(char *) malloc(4096);
    if (recognizer != NULL) {
        vr_writeData(recognizer, javaBuf, _dataSize);
    }
}
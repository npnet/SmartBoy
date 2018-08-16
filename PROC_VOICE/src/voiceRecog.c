#include "../include/common.h"
#include "VoiceConnectIf.h"

#ifndef SUB_BAND_ANALYSER

#include <assert.h>
#include "../include/voiceRecog.h"
#include "../include/voiceRecognizer.h"
#include "../include/signalAnalyser.h"
#include "../include/util.h"
//#include "common.h"


#if 1
struct GeneralRecognitionListener {
    struct MyRecognitionListener parent;
    vr_pRecognizerStartListener startListener;
    vr_pRecognizerEndListener endListener;
    vr_pRecognizerMatchListener matchListener;
    void *listener;
};
#ifdef AA_INLINE
inline vr_pRecognizerStartListener grl_getfStartListener(struct GeneralRecognitionListener *_this){return _this->startListener;}
inline void grl_setfStartListener(struct GeneralRecognitionListener *_this, vr_pRecognizerStartListener _listener){_this->startListener = _listener;}
inline vr_pRecognizerEndListener grl_getfEndListener(struct GeneralRecognitionListener *_this){return _this->endListener;}
inline void grl_setfEndListener(struct GeneralRecognitionListener *_this, vr_pRecognizerEndListener _listener){_this->endListener = _listener;}
inline vr_pRecognizerMatchListener grl_getfMatchListener(struct GeneralRecognitionListener *_this){return _this->matchListener;}
inline void grl_setfMatchListener(struct GeneralRecognitionListener *_this, vr_pRecognizerMatchListener _listener){_this->matchListener = _listener;}
#else

vr_pRecognizerStartListener grl_getfStartListener(struct GeneralRecognitionListener *_this) {
    return _this->startListener;
}

void grl_setfStartListener(struct GeneralRecognitionListener *_this, vr_pRecognizerStartListener _listener)
{
    _this->startListener = _listener;
}

vr_pRecognizerEndListener grl_getfEndListener(struct GeneralRecognitionListener *_this)
{
    return _this->endListener;
}

void grl_setfEndListener(struct GeneralRecognitionListener *_this, vr_pRecognizerEndListener _listener)
{
    _this->endListener = _listener;
}

vr_pRecognizerMatchListener grl_getfMatchListener(struct GeneralRecognitionListener *_this)
{
    return _this->matchListener;
}

void grl_setfMatchListener(struct GeneralRecognitionListener *_this, vr_pRecognizerMatchListener _listener)
{
    _this->matchListener = _listener;
}


#endif

void grl_onStartRecognition(struct RecognitionListener *_this_, float _soundTime) {
    FUNC_START
    struct GeneralRecognitionListener *_this = (struct GeneralRecognitionListener *) _this_;
    if (_this->startListener != NULL) {
        _this->startListener(_this->listener, _soundTime);
    }
    FUNC_END
}

#if !(defined(FREQ_ANALYSE_SINGLE_MATCH) || defined(FREQ_ANALYSE_SINGLE_MATCH2)) && !defined(MFCC_CRY_DETECT) && !defined(SUB_BAND_ANALYSER)
#define MAX_MATCH_FREQ_COUNT 8

void mrl_onMatchFrequency(struct RecognitionListener *this_, struct SignalAnalyser *_analyser, struct EventInfo *_event,
                          struct FrequencyInfoSearcher *_fiSearcher) {
    struct GeneralRecognitionListener *_this = (struct GeneralRecognitionListener *) this_;
    FUNC_START
    if (_this->matchListener != NULL) {
        struct VoiceMatch matches[MAX_MATCH_FREQ_COUNT];
        int i, ic = vector_size(&_fiSearcher->checkedFreqs), j, jc, matchIdx = 0;
        struct FrequencyInfo **pfi = (struct FrequencyInfo **) vector_nativep(&_fiSearcher->checkedFreqs);
        asize2_1(pfi, ic, matches, MAX_MATCH_FREQ_COUNT, "fm", sizeof(struct FrequencyMatch));
        for (i = 0; i < ic && matchIdx < MAX_MATCH_FREQ_COUNT; i++) {
            struct FrequencyInfo *fi = adata(pfi, i);
            struct FrequencyMatch *fm;

            adata(matches, matchIdx).frequency = fi->frequencyY * _analyser->sampleRate / (_analyser->fftSize);
            adata(matches, matchIdx).length = fi_realTimesCount(fi);
#ifdef AMPLITUDE_SNR
            adata(matches, matchIdx).strength = fi->avgSNR;
#else

            fm = fi_times(fi);
            areset2("fm", fm, fi_timesCount(fi));
            for (j = 0, jc = fi_timesCount(fi); j < jc; j++) {
                adata(matches, matchIdx).strength += ((adata(fm, j).frequency->peakType == 1) ? 10 : 6);
            }
#endif

            matchIdx++;
        }

        if (matchIdx > 0) {
            _this->matchListener(_this, (_event == NULL ? 0 : _event->timeIdx), matches, matchIdx);
        }
    }
    FUNC_END
}

#endif


bool grl_onAfterECC(struct MyRecognitionListener *this_, float _soundTime, int _recogStatus, int *_indexs, int _count) {
    struct GeneralRecognitionListener *_this = (struct GeneralRecognitionListener *) this_;

    FUNC_START
    if (_this->endListener != NULL) {
#define MAX_CHARS 80
        char chars[MAX_CHARS];
        int i = 0;
        asize3(chars, MAX_CHARS, _indexs, _count, DEFAULT_CODE_BOOK, DEFAULT_CODE_FREQUENCY_SIZE);
        memset(chars, 0, MAX_CHARS);
        if (_recogStatus == 0) {
            for (; i < MAX_CHARS && i < _count; i++) {
                adata(chars, i) = adata(DEFAULT_CODE_BOOK, adata(_indexs, i));
            }
        }
        _this->endListener(_this->listener, _soundTime, _recogStatus, ((_recogStatus == 0) ? chars : NULL), i);

    }
    FUNC_END
    return _recogStatus == 0;
}

struct GeneralRecognitionListener *grl_init(struct GeneralRecognitionListener *_this, void *_listener,
                                            vr_pRecognizerStartListener _startListener,
                                            vr_pRecognizerEndListener _endListener,
                                            vr_pRecognizerMatchListener _matchListener) {
    FUNC_START
    mrl_init((struct MyRecognitionListener *) _this);
    _this->listener = _listener;
    _this->startListener = _startListener;
    _this->endListener = _endListener;
    _this->matchListener = _matchListener;
    _this->parent.onAfterECC = grl_onAfterECC;
    _this->parent.parent.onStartRecognition = grl_onStartRecognition;
#if !(defined(FREQ_ANALYSE_SINGLE_MATCH) || defined(FREQ_ANALYSE_SINGLE_MATCH2)) && !defined(MFCC_CRY_DETECT) && !defined(SUB_BAND_ANALYSER)
    _this->parent.parent.onMatchFrequency = mrl_onMatchFrequency;
#endif

    FUNC_END
    return _this;
}

void grl_finalize(struct GeneralRecognitionListener *_this) {
}

VOICERECOGNIZEDLL_API void *vr_createVoiceRecognizer(enum DecoderPriority _decoderPriority) {
    return vr_createVoiceRecognizer2(_decoderPriority, DEFAULT_SAMPLE_RATE);
}


/**
 *
 * @param _decoderPriority
 * @param _sampleRate
 * @return
 */
void *vr_createVoiceRecognizer2(enum DecoderPriority _decoderPriority, int _sampleRate) {
    FUNC_START
    enum ProcessorType processorType = FFTVoiceProcessorType;
    struct VoiceRecognizer *recognizer;
    int shouldBufferSize, bufferSize, overlap;
    if (_decoderPriority == MemoryUsePriority) {
        processorType = PreprocessVoiceProcessorType;
    }

    bufferSize = shouldBufferSize = (((int) (DEFAULT_FFT_SIZE * ((double) _sampleRate) / DEFAULT_SAMPLE_RATE)) / 2) * 2;
    if (bufferSize <= 256) {
        bufferSize = 256;
    } else if (bufferSize > 256 && bufferSize <= 512) {
        bufferSize = 512;
    } else {
        bufferSize = 1024;
    }
    overlap = bufferSize - shouldBufferSize / 2;
    LOG("sampleRate=%d,bufferSize=%d,overlap=%d\n",_sampleRate,bufferSize,overlap);
    //创建识别器
    recognizer = _new(struct VoiceRecognizer, vrr_init, processorType, _sampleRate, 1, 16, bufferSize, overlap);
    FUNC_END
    return recognizer;
};

VOICERECOGNIZEDLL_API void vr_destroyVoiceRecognizer(void *_recognizer) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    struct RecognitionListener *listener = NULL;
    int i, ic = vrr_getFreqsCount(recognizer);
    for (i = 0; i < ic; i++) {
        listener = vrr_getListener(recognizer, i);
        if (listener != NULL) {
            _delete((struct GeneralRecognitionListener *) listener, grl_finalize);
        }
    }
    _delete(recognizer, vrr_finalize);
}

VOICERECOGNIZEDLL_API void vr_setRecognizeFreqs(void *_recognizer, int *_freqs, int _freqCount) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
#ifdef FIX_17K
    int baseFreq = 17200;
    int i;
    for(i = 0; i < DEFAULT_CODE_FREQUENCY_SIZE; i ++)
    {
        _freqs[i] = baseFreq + i * 200;
    }
#endif
    vrr_setFreqs(recognizer, 0, _freqs, _freqCount);
}

VOICERECOGNIZEDLL_API void vr_setRecognizeFreqs2(void *_recognizer, int *_freqs, int _freqCount) {
#ifndef FIX_17K
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    vrr_setFreqs(recognizer, 1, _freqs, _freqCount);
#endif
}

void vr_setRecognizerListenerImple(void *_recognizer, int _idx, void *_listener, vr_pRecognizerStartListener _startListener,
                              vr_pRecognizerEndListener _endListener, vr_pRecognizerMatchListener _matchListener) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    struct RecognitionListener *listener = vrr_getListener(recognizer, _idx);
    FUNC_START
    if (listener == NULL) {
        vrr_setListener(recognizer, _idx,listener = (struct RecognitionListener *) _new(struct GeneralRecognitionListener,
                grl_init, _listener, _startListener, _endListener, _matchListener));

    } else {
        struct GeneralRecognitionListener *mylistener = (struct GeneralRecognitionListener *) (listener);
        if (_startListener != NULL) {
            grl_setfStartListener(mylistener, _startListener);
        }
        if (_endListener != NULL) {
            grl_setfEndListener(mylistener, _endListener);
        }
        if (_matchListener != NULL) {
            grl_setfMatchListener(mylistener, _matchListener);
        }
    }
    FUNC_END
}

VOICERECOGNIZEDLL_API void
vr_setRecognizerListener(void *_recognizer, void *_listener, vr_pRecognizerStartListener _startListener,
                         vr_pRecognizerEndListener _endListener)
{
    vr_setRecognizerListener2(_recognizer, _listener, _startListener, _endListener, NULL);
}

VOICERECOGNIZEDLL_API void
vr_setRecognizerListener2(void *_recognizer, void *_listener, vr_pRecognizerStartListener _startListener,
                          vr_pRecognizerEndListener _endListener, vr_pRecognizerMatchListener _matchListener) {
    vr_setRecognizerListenerImple(_recognizer, 0, _listener, _startListener, _endListener, _matchListener);
}

void vr_setRecognizerFreq2Listener(void *_recognizer, void *_listener, vr_pRecognizerStartListener _startListener,
                                   vr_pRecognizerEndListener _endListener, vr_pRecognizerMatchListener _matchListener) {
#ifndef FIX_17K
    vr_setRecognizerListenerImple(_recognizer, 1, _listener, _startListener, _endListener, _matchListener);
#endif
}

VOICERECOGNIZEDLL_API void vr_runRecognizer(void *_recognizer) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    vrr_run(recognizer);
}

VOICERECOGNIZEDLL_API void vr_pauseRecognize(void *_recognizer, int _microSeconds) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    vrr_pause(recognizer, _microSeconds);
}

VOICERECOGNIZEDLL_API void vr_stopRecognize(void *_recognizer) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    struct BufferDataWriter *writer = vrr_getBufferWriter(recognizer);
    bdw_flush(writer);
    vrr_freeBuffer((struct BufferSource *) recognizer, bd_getNullBuffer());
}

VOICERECOGNIZEDLL_API vr_bool vr_isRecognizerStopped(void *_recognizer) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    return (vrr_isStopped(recognizer) ? vr_true : vr_false);
}

VOICERECOGNIZEDLL_API int vr_writeData(void *_recognizer, char *_data, int _dataLen) {
    struct VoiceRecognizer *recognizer = (struct VoiceRecognizer *) _recognizer;
    struct BufferDataWriter *writer = vrr_getBufferWriter(recognizer);
    LOG("识别回调写入数据长度datalen=%d\n",_dataLen);
    return bdw_write(writer, _data, _dataLen);
}

VOICERECOGNIZEDLL_API int vr_getVer() {
    return MYVERSION;
}


//=========================解码部分==========================
/**
 *
 * @param _hexs
 * @param _hexsLen
 * @param _costHexsLen
 * @param _result
 * @param _resultLen
 * @param _resultBufSize
 * @return
 */
int hex2Digit(char *_hexs, int _hexsLen, int *_costHexsLen, char *_result, int _resultLen, int _resultBufSize) {
    int i;
    asize2(_result, _resultBufSize, _hexs, _hexsLen);
    for (i = 0; i < _hexsLen && (_resultLen < 0 || i < _resultLen); i++) {
        adata(_result, i) = adata(_hexs, i);
    }
    *_costHexsLen = i;
    return i;
}


#ifdef AA_INLINE
inline int hex2Chars(char *_hexs, int _hexsCount, int *_costHexsCount, char *_result, int _maxBitCount, int _resultBufSize)
{
    int i, hexsCount = ((_maxBitCount < 0)?_hexsCount:((_maxBitCount+3)/4));
    asize2(_hexs, _hexsCount, _result, _resultBufSize);
    hexsCount = ((hexsCount > _hexsCount)?_hexsCount:hexsCount);
    for (i = 0; i < hexsCount/2; i ++)
    {
        adata(_result, i) = hexChar2Int(adata(_hexs, 2 * i)) << 4 | hexChar2Int(adata(_hexs, 2 * i + 1));
    }
    *_costHexsCount = 2 * i;
    if (hexsCount%2 > 0)
    {
        *_costHexsCount = 2 * i + 1;
        adata(_result, i) = hexChar2Int(adata(_hexs, 2 * i)) << 4;
        i++;
    }
    return i;
}

inline int hex2Type(char *_hexs, int _hexsLen, int *_hexsCostLen, char *_type, int *_aPartLen)
{
    int r = 1;
    asize1(_hexs, _hexsLen);

    *_type = hexChar2Int(adata(_hexs, 0)) >> 1;
    *_hexsCostLen = 1;
    if (*_type == 1 || *_type == 2 || *_type == 4)
    {
        *_aPartLen = ((hexChar2Int(adata(_hexs, 0)) & 0x01) << 4) | hexChar2Int(adata(_hexs, 1));
        *_aPartLen += 1;
        r = 2;
        *_hexsCostLen = 2;
    }
    return r;
}


#else

int hex2Chars(char *_hexs, int _hexsCount, int *_costHexsCount, char *_result, int _maxBitCount, int _resultBufSize) {
    int i, hexsCount = ((_maxBitCount < 0) ? _hexsCount : ((_maxBitCount + 3) / 4));
    asize2(_hexs, _hexsCount, _result, _resultBufSize);
    hexsCount = ((hexsCount > _hexsCount) ? _hexsCount : hexsCount);
    for (i = 0; i < hexsCount / 2; i++) {
        adata(_result, i) = hexChar2Int(adata(_hexs, 2 * i)) << 4 | hexChar2Int(adata(_hexs, 2 * i + 1));
    }
    *_costHexsCount = 2 * i;
    if (hexsCount % 2 > 0) {
        *_costHexsCount = 2 * i + 1;
        adata(_result, i) = hexChar2Int(adata(_hexs, 2 * i)) << 4;
        i++;
    }
    return i;
}


int hex2Type(char *_hexs, int _hexsLen, int *_hexsCostLen, char *_type, int *_aPartLen) {
    int r = 1;
    asize1(_hexs, _hexsLen);

    *_type = hexChar2Int(adata(_hexs, 0)) >> 1;
    *_hexsCostLen = 1;
    if (*_type == 1 || *_type == 2 || *_type == 4) {
        *_aPartLen = ((hexChar2Int(adata(_hexs, 0)) & 0x01) << 4) | hexChar2Int(adata(_hexs, 1));
        *_aPartLen += 1;
        r = 2;
        *_hexsCostLen = 2;
    }
    return r;
}

#endif

int hex2Lower(char *_hexs, int _hexsLen, int *_costHexsLen, char *_result, int _resultLen, int _resultBufSize) {
    char *tempChars = (char *) mymalloc(sizeof(char) * (_hexsLen / 2 + 1));
    int resultIdx;
    int costBits = 0;
    asize1(_result, _resultBufSize);

    hex2Chars(_hexs, _hexsLen, _costHexsLen, tempChars, ((_resultLen > 0) ? _resultLen * 5 : _resultLen),
              sizeof(char) * (_hexsLen / 2 + 1));
    for (resultIdx = 0; (_resultLen < 0 || resultIdx < _resultLen); resultIdx++) {
        costBits = (resultIdx + 1) * 5;
        if (costBits > _hexsLen * 4)break;
        adata(_result, resultIdx) = 'a' + bitsGet(tempChars, resultIdx * 5, costBits);
        *_costHexsLen = (costBits + 3) / 4;
    }
    myfree(tempChars);
    return resultIdx;
}

int hex2Upper(char *_hexs, int _hexsLen, int *_costHexsLen, char *_result, int _resultLen, int _resultBufSize) {
    char *tempChars = (char *) mymalloc(sizeof(char) * (_hexsLen / 2 + 1));
    int resultIdx;
    int costBits = 0;
    asize1(_result, _resultBufSize);

    hex2Chars(_hexs, _hexsLen, _costHexsLen, tempChars, ((_resultLen > 0) ? _resultLen * 5 : _resultLen),
              sizeof(char) * (_hexsLen / 2 + 1));
    for (resultIdx = 0; (_resultLen < 0 || resultIdx < _resultLen); resultIdx++) {
        costBits = (resultIdx + 1) * 5;
        if (costBits > _hexsLen * 4)break;
        adata(_result, resultIdx) = 'A' + bitsGet(tempChars, resultIdx * 5, costBits);
        *_costHexsLen = (costBits + 3) / 4;
    }
    myfree(tempChars);
    return resultIdx;
}

int hex2Char64(char *_hexs, int _hexsLen, int *_costHexsLen, char *_result, int _resultLen, int _resultBufSize) {
    char *tempChars = (char *) mymalloc(sizeof(char) * (_hexsLen / 2 + 1));
    int resultIdx;
    int costBits = 0;
    asize1(_result, _resultBufSize);

    hex2Chars(_hexs, _hexsLen, _costHexsLen, tempChars, ((_resultLen > 0) ? _resultLen * 6 : _resultLen),
              sizeof(char) * (_hexsLen / 2 + 1));
    for (resultIdx = 0; (_resultLen < 0 || resultIdx < _resultLen); resultIdx++) {
        costBits = (resultIdx + 1) * 6;
        if (costBits > _hexsLen * 4)break;
        adata(_result, resultIdx) = int2Char64(bitsGet(tempChars, resultIdx * 6, costBits));
        *_costHexsLen = (costBits + 3) / 4;
    }
    myfree(tempChars);
    return resultIdx;
}

int hex2Char256(char *_hexs, int _hexsLen, int *_costHexsLen, char *_result, int _resultLen, int _resultBufSize) {
    return hex2Chars(_hexs, _hexsLen, _costHexsLen, _result, ((_resultLen > 0) ? _resultLen * 8 : _resultLen),
                     _resultBufSize);
}


int vr_decodeData(char *_hexs, int _hexsLen, int *_hexsCostLen, char *_result, int _resultLen, int _resultBufSize) {
    int aPartLen = _resultLen, costHexsAdd = 0, dataLen = 0;
    char type;

    *_hexsCostLen = 0;
    hex2Type(_hexs, _hexsLen, _hexsCostLen, &type, &aPartLen);
    if (type == 1) {

        dataLen += hex2Lower(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, aPartLen,
                             _resultBufSize);
        *_hexsCostLen += costHexsAdd;
        dataLen += hex2Digit(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result + dataLen,
                             ((_resultLen < 0) ? _resultLen : (_resultLen - dataLen)), _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 2) {

        dataLen += hex2Char64(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, aPartLen,
                              _resultBufSize);
        *_hexsCostLen += costHexsAdd;
        dataLen += hex2Digit(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result + dataLen,
                             ((_resultLen < 0) ? _resultLen : (_resultLen - dataLen)), _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 4) {

        dataLen += hex2Digit(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, aPartLen,
                             _resultBufSize);
        *_hexsCostLen += costHexsAdd;
        dataLen += hex2Char64(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result + dataLen,
                              ((_resultLen < 0) ? _resultLen : (_resultLen - dataLen)), _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 0) {

        dataLen += hex2Digit(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, _resultLen,
                             _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 3) {

        dataLen += hex2Char256(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, _resultLen,
                               _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 5) {

        dataLen += hex2Lower(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, _resultLen,
                             _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 6) {

        dataLen += hex2Char64(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, _resultLen,
                              _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    } else if (type == 7) {

        dataLen += hex2Upper(_hexs + *_hexsCostLen, _hexsLen - *_hexsCostLen, &costHexsAdd, _result, _resultLen,
                             _resultBufSize);
        *_hexsCostLen += costHexsAdd;
    }
    return dataLen;
}

vr_bool vr_decodeString(int _recogStatus, char *_data, int _dataLen, char *_result, int _resultLen) {
    int costHexsLen = 1, costHexsAdd = 0;
    int dataLen = 0;
    asize1(_result, _resultLen);
    assert(vr_decodeInfoType(_data, _dataLen) == IT_STRING);
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _result, _resultLen, _resultLen);
    assert(dataLen < _resultLen);
    adata(_result, dataLen) = 0;
    return vr_true;
}

enum InfoType vr_decodeInfoType(char *_data, int _dataLen) {
    asize1(_data, _dataLen);
    return (enum InfoType) (hexChar2Int(adata(_data, 0)) >> 1);
}

vr_bool vr_decodeWiFi(int _result, char *_data, int _dataLen, struct WiFiInfo *_wifi) {

    asize3(_data, _dataLen, _wifi->mac, sizeof(_wifi->mac), _wifi->pwd, sizeof(_wifi->pwd))
    int macLen = (((hexChar2Int(adata(_data, 0)) & 0x01) << 4) | hexChar2Int(adata(_data, 1))) + 1;
    int costHexsLen = 2, costHexsAdd = 0;
    int dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->mac, macLen,
                                sizeof(_wifi->mac));
    assert(dataLen == macLen);
    _wifi->macLen = macLen;
    costHexsLen += costHexsAdd;
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->pwd, -1,
                            sizeof(_wifi->pwd));
    adata(_wifi->pwd, dataLen) = 0;
    _wifi->pwdLen = dataLen;
    return vr_true;
}

vr_bool vr_decodeSSIDWiFi(int _result, char *_data, int _dataLen, struct SSIDWiFiInfo *_wifi) {

    asize3(_data, _dataLen, _wifi->ssid, sizeof(_wifi->ssid), _wifi->pwd, sizeof(_wifi->pwd))
    int ssidLen = (((hexChar2Int(adata(_data, 0)) & 0x01) << 4) | hexChar2Int(adata(_data, 1))) + 1;
    int costHexsLen = 2, costHexsAdd = 0;
    int dataLen = 0;
    assert(vr_decodeInfoType(_data, _dataLen) == IT_SSID_WIFI);
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->ssid, ssidLen,
                            sizeof(_wifi->ssid));
    assert(dataLen == ssidLen);
    adata(_wifi->ssid, dataLen) = 0;
    _wifi->ssidLen = ssidLen;
    costHexsLen += costHexsAdd;
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->pwd, -1,
                            sizeof(_wifi->pwd));
    adata(_wifi->pwd, dataLen) = 0;
    _wifi->pwdLen = dataLen;
    return vr_true;
}

vr_bool vr_decodePhone(int _result, char *_data, int _dataLen, struct PhoneInfo *_phone) {

    asize3(_data, _dataLen, _phone->imei, sizeof(_phone->imei), _phone->phoneName, sizeof(_phone->phoneName))
    int imeiLen = (((hexChar2Int(adata(_data, 0)) & 0x01) << 4) | hexChar2Int(adata(_data, 1))) + 1;
    int costHexsLen = 2, costHexsAdd = 0;
    int dataLen = 0;
    assert(vr_decodeInfoType(_data, _dataLen) == IT_PHONE);
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _phone->imei, imeiLen,
                            sizeof(_phone->imei));
    assert(dataLen == imeiLen);
    adata(_phone->imei, dataLen) = 0;
    _phone->imeiLen = dataLen;
    costHexsLen += costHexsAdd;
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _phone->phoneName, -1,
                            sizeof(_phone->phoneName));
    adata(_phone->phoneName, dataLen) = 0;
    _phone->nameLen = dataLen;
    return vr_true;
}

vr_bool vr_decodeFSSSIDWiFi(int _result, char *_data, int _dataLen, struct FS_SSIDWiFiInfo *_wifi) {

    asize3(_data, _dataLen, _wifi->ssid, sizeof(_wifi->ssid), _wifi->pwd, sizeof(_wifi->pwd))
    int ssidLen = (((hexChar2Int(_data[0]) & 0x01) << 4) | hexChar2Int(_data[1])) + 1;
    int costHexsLen = 2 + 5, costHexsAdd = 0;
    int dataLen = 0;
    _wifi->chAppType = _data[2];
    _wifi->chHead[0] = _data[3];
    _wifi->chHead[1] = _data[4];
    _wifi->chHead[2] = _data[5];
    _wifi->chHead[3] = _data[6];
    assert(vr_decodeInfoType(_data, _dataLen) == IT_SSID_WIFI);
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->ssid, ssidLen,
                            sizeof(_wifi->ssid));
    assert(dataLen == ssidLen);
    _wifi->ssid[dataLen] = 0;
    _wifi->ssidLen = ssidLen;
    costHexsLen += costHexsAdd;
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->pwd, -1,
                            sizeof(_wifi->pwd));
    _wifi->pwd[dataLen] = 0;
    _wifi->pwdLen = dataLen;
    return vr_true;
}

#define MAX_TRANS_BLOCK_SIZE 59

void vr_bs_reset(struct BlocksComposer *_this) {
    _this->blockCount = 0;
    _this->dataLen = 0;
    mymemset(_this->blockFlags, 0, sizeof(_this->blockFlags));
}

int vr_bs_composeBlock(struct BlocksComposer *_this, char *_blockData, int _blockDataLen) {
    int blockCount = (hexChar2Int(_blockData[0]) >> 2) + 1;
    int blockIdx = hexChar2Int(_blockData[0]) & 0x3;
    assert(blockCount <= 4 && blockIdx < 4);
    if (_this->blockCount == 0) {
        _this->blockCount = blockCount;
    }
    if (blockIdx == _this->blockCount - 1) {
        _this->dataLen = ((_this->blockCount - 1) * MAX_TRANS_BLOCK_SIZE) + _blockDataLen - 1;
    }
    _this->blockFlags[blockIdx] = 1;
    if (blockIdx < _this->blockCount - 1) { assert(MAX_TRANS_BLOCK_SIZE == _blockDataLen - 1); }
    mymemcpy(_this->data + blockIdx * MAX_TRANS_BLOCK_SIZE, _blockData + 1, _blockDataLen - 1);
    return blockIdx;
}

vr_bool vr_bs_isAllBlockComposed(struct BlocksComposer *_this) {
    int i;
    vr_bool result = vr_false;
    if (_this->blockCount > 0) {
        result = vr_true;
        for (i = 0; i < _this->blockCount; i++) {
            if (!_this->blockFlags[i]) {
                result = vr_false;
                break;
            }
        }
    }

    return result;
}

vr_bool vr_decodeZSSSIDWiFi(int _result, char *_data, int _dataLen, struct ZS_SSIDWiFiInfo *_wifi) {

    asize4(_data, _dataLen, _wifi->ssid, sizeof(_wifi->ssid), _wifi->pwd, sizeof(_wifi->pwd), _wifi->phone,
           sizeof(_wifi->phone))
    int ssidLen = (((hexChar2Int(adata(_data, 0)) & 0x01) << 4) | hexChar2Int(adata(_data, 1))) + 1;
    int costHexsLen = 2, costHexsAdd = 0;
    int dataLen = 0;
    assert(vr_decodeInfoType(_data, _dataLen) == IT_SSID_WIFI);
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->phone, 11,
                            sizeof(_wifi->phone));
    adata(_wifi->phone, dataLen) = 0;
    costHexsLen += costHexsAdd;
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->ssid, ssidLen,
                            sizeof(_wifi->ssid));
    assert(dataLen == ssidLen);
    adata(_wifi->ssid, dataLen) = 0;
    _wifi->ssidLen = ssidLen;
    costHexsLen += costHexsAdd;
    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, _wifi->pwd, -1,
                            sizeof(_wifi->pwd));
    adata(_wifi->pwd, dataLen) = 0;
    _wifi->pwdLen = dataLen;
    return vr_true;
}

vr_bool vr_decodeXZXXX(int _result, char *_data, int _dataLen, struct XZ_XXX *_xxx) {

    int portLen = (((hexChar2Int(_data[0]) & 0x01) << 4) | hexChar2Int(_data[1])) + 1;
    int costHexsLen = 2, costHexsAdd = 0;
    int dataLen = 0;
    char portBuf[15];
    FUNC_START
    assert(vr_decodeInfoType(_data, _dataLen) == 2);

    strncpy(_xxx->sn, _data + costHexsLen, 6);
    _xxx->sn[6] = 0;
    costHexsLen += 6;

    _xxx->ip[0] = (char) (hexChar2Int(_data[costHexsLen]) | hexChar2Int(_data[costHexsLen + 1]) << 4);
    costHexsLen += 2;
    _xxx->ip[1] = (char) (hexChar2Int(_data[costHexsLen]) | hexChar2Int(_data[costHexsLen + 1]) << 4);
    costHexsLen += 2;
    _xxx->ip[2] = (char) (hexChar2Int(_data[costHexsLen]) | hexChar2Int(_data[costHexsLen + 1]) << 4);
    costHexsLen += 2;
    _xxx->ip[3] = (char) (hexChar2Int(_data[costHexsLen]) | hexChar2Int(_data[costHexsLen + 1]) << 4);
    costHexsLen += 2;

    dataLen = vr_decodeData(_data + costHexsLen, _dataLen - costHexsLen, &costHexsAdd, portBuf, -1, sizeof(portBuf));
    portBuf[dataLen] = 0;
    _xxx->port = atoi(portBuf);
    FUNC_END
    return vr_true;
}

#endif
#endif


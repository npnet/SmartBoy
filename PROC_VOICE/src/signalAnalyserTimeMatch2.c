
#include "../include/common.h"

#ifdef FREQ_ANALYSE_TIME_MATCH2

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <PROC_VOICE/include/tools.h>
#include "../include/util.h"

#ifndef WIN32

#include "../include/linux.h"

#endif

#include "../include/common.h"
#include "../include/log.h"
#include "../include/signalAnalyser.h"

//#include "signalAnalyserTimeMatch2.h"

extern bool printSignalDetect;
extern bool printAnalyseEventFreq;
extern bool printAnalyseSignal;
extern bool printAnalyseSignalMatch;
extern bool printAnalyseEventPeak;

const char *TAG_SignalAnalyser = "SignalAnalyser";

#define preciseMatchCount 5.6
#define maxMainMatchCount 6
#define RangeMatchCount 5
#define maxBlockMatchCount 6
#define searchMinRangeCount 7
static const int freqCountLong = maxMainMatchCount / 2;
static const int maxMatchDeviation = 2;
#define unknownMatchDeviation 99
#define MAX_RANGE_SIGNAL_CACHE_COUNT ((VOICE_BLOCK_SIZE + 2) * searchMinRangeCount)
static const int minBigMatchCount = 2;
#define RANGE_AVG_SIZE maxMainMatchCount
#define RANGE_TOP2AVGINITER_AVG_SIZE maxMainMatchCount
#define MAX_SIGNAL_SHORT_VALID_SIZE maxMainMatchCount * 1.6
#define MAX_RNAGE_SIGNAL_LOW_SIZE maxMainMatchCount * 1.6
#define MAX_LONG_RNAGE_SIGNAL_LOW_SIZE 30
#define SNR_INIT_STAT_SIZE 20
#define MAX_SHORT_SIGNAL_SIZE maxMainMatchCount * 2
#define MIN_START_SIGNAL_SIZE (MAX_SHORT_SIGNAL_SIZE+maxMainMatchCount * 1)
#define maxShortFreqCount 2
#define LONG_RANGE_MIN_SNR 1.4f
#define MIN_SIGNAL_BIG_MATCH 3
#define MinBlockOrderMatchCount 2

#define TIME_MATCH_SIGNAL
#define MARK_MORE_MAYBE_ERROR

#define dbgprint(format, ...)
#define printPeak(format, ...)
#define printSignals(format, ...)
#define printFreqDiscard(format, ...)
#define printRemoveSmallPeaks(format, ...)
#define printEventsIdxOff(format, ...)

int checkRecog = 0;

int getIdxFromFrequencyFromCache(struct SignalAnalyser *_this, int _pitch);

int intComparator(const void *_o1, const void *_o2) {
    return (*(int *) _o1) - (*(int *) _o2);
}

#ifdef FS_8136

int fsFreqs[] = {88 , 92 , 96 , 100, 105, 109, 113, 118, 122, 126, 131, 135, 140, 144, 148, 153, 157, 161, 166};
#define maxFSFreqYDistance 2
#define FSFreqYStart 84
#define FSFreqYEnd 170
int getFSIdxFromFrequency(int _pitch)
{
    int pos = mybinarySearch_(&_pitch, fsFreqs + 1, DEFAULT_CODE_FREQUENCY_SIZE - 1 - 1, sizeof(int), intComparator);

    if (pos >= 0)
    {
        pos += 1;
    }
    else
    {
        pos = -(-(pos + 1)+1)-1;
    }
    if (pos >= 0)
        return pos;
    else
    {
        int insertPos = -(pos + 1);
        int leftDistance = (insertPos == 0)?1000:(_pitch - fsFreqs[insertPos - 1]);
        int rightDistance = (insertPos >= DEFAULT_CODE_FREQUENCY_SIZE)?1000:(fsFreqs[insertPos] - _pitch);
        if (leftDistance <= rightDistance)
        {
            if (leftDistance <= maxFSFreqYDistance)
                return insertPos - 1;
            else
                return -1;
        }
        else
        {
            if (rightDistance <= maxFSFreqYDistance)
                return insertPos;
            else
                return -1;
        }
    }
}

inline int correctFreqIdx(struct FreqAmpsEventStat *_this, struct SignalAnalyser *_sa, int _freqIdx)
{
    int pos;
    assert(_freqIdx >= FSFreqYStart && _freqIdx < FSFreqYEnd);
    assert(_this->freqY2FreqIdx != NULL && _this->freqY2FreqIdxSize > 0);
    pos = _this->freqY2FreqIdx[_freqIdx-FSFreqYStart];
    if(pos >= 0 && pos < DEFAULT_CODE_FREQUENCY_SIZE)
    {
        return _sa->CODE_Y[pos];
    }
    return _freqIdx;
}
#endif


//==================by sine=========================
#ifndef AA_INLINE

void trs_reset(struct TimeRangeSignal *_this) {
    memset(_this->idxes, -1, sizeof(_this->idxes));
    _this->flags = 0;
}

bool trs_maybeError(struct TimeRangeSignal *_this) {
    return _this->flags;
}

void trs_setMaybeError(struct TimeRangeSignal *_this, bool _val) {
    _this->flags = _val;
}

struct FrequencyMatch *fi_realTimes(struct FrequencyInfo *_this) {
    return _this->realTimes;
}

int fi_realTimesCount(struct FrequencyInfo *_this) {
    return _this->realEventCount;
}

struct FrequencyMatch *fi_realTimesFirst(struct FrequencyInfo *_this) {
    assert(_this->realEventCount > 0);
    return _this->realTimes;
}

struct FrequencyMatch *fi_realTimesLast(struct FrequencyInfo *_this) {
    return _this->realTimes + (_this->realEventCount - 1);
}

struct FrequencyMatch *fi_times(struct FrequencyInfo *_this) {
    return _this->realTimes + _this->timesStart;
}

int fi_timesCount(struct FrequencyInfo *_this) {
    return _this->timesEnd - _this->timesStart;
}

struct FrequencyMatch *fi_timesFirst(struct FrequencyInfo *_this) {
    assert(_this->realEventCount > 0 && _this->timesStart >= 0 && _this->timesStart < _this->timesEnd &&
           _this->timesStart < _this->realEventCount && _this->timesEnd <= _this->realEventCount);
    return _this->realTimes + _this->timesStart;
}

struct FrequencyMatch *fi_timesLast(struct FrequencyInfo *_this) {
    assert(_this->realEventCount > 0 && _this->timesStart >= 0 && _this->timesStart < _this->timesEnd &&
           _this->timesStart < _this->realEventCount && _this->timesEnd <= _this->realEventCount);
    return _this->realTimes + (_this->timesEnd - 1);
}

void fi_timesPollFirst(struct FrequencyInfo *_this) {
    assert(_this->timesStart < _this->timesEnd);
    _this->timesStart++;
}

void fi_timesClear(struct FrequencyInfo *_this) {
    _this->realEventCount = _this->timesStart = _this->timesEnd = 0;
}

int sa_getMinFreqIdx(struct SignalAnalyser *_this) {
    return _this->ampsRange.minFreqIdx;
}

int sa_getMaxFreqIdx(struct SignalAnalyser *_this) {
    return _this->ampsRange.maxFreqIdx;
}

TRecognitionListener sa_getListener(struct SignalAnalyser *_this) {
    return _this->listener;
}

void sa_setListener(struct SignalAnalyser *_this, TRecognitionListener listener) {
    _this->listener = listener;
}


 tidx iei_idx(struct Idx2EventInfo *_this)
{
    return _this->end;
}

 void iei_add(struct Idx2EventInfo *_this, const struct EventInfo *_event)
{
    int idx = _this->end % _this->size;
    _this->events[idx] = *_event;
    _this->events[idx].timeIdx = ++_this->end;
}

 struct EventInfo *iei_curr(struct Idx2EventInfo *_this)
{
    return _this->events + ((_this->end-1) % _this->size);
}

 tidx iei_minIdx(struct Idx2EventInfo *_this)
{
    return _this->end - _this->size + 1;
}

 tidx iei_currIdx(struct Idx2EventInfo *_this)
{
    return _this->end - 1;
}

struct EventInfo* iei_get(struct Idx2EventInfo *_this, tidx _idx)
{
    _idx = _idx - 1;
    if(_idx < _this->end && _idx >= (_this->end - _this->size))
    {
        return _this->events + (_idx % _this->size);
    }
    return NULL;
}

#endif
//===================by sine==========================

struct EventInfo *ei_init2(struct EventInfo *_this, tidx timeIdx, struct FreqAmplitude *p1, struct FreqAmplitude *p2) {
    _this->timeIdx = timeIdx;
    _this->p1 = *p1;
    _this->p2 = *p2;
    return _this;
}

struct EventInfo *ei_init(struct EventInfo *_this) {
    memset(_this, 0, sizeof(struct EventInfo));
    return _this;
}

struct Idx2EventInfo *iei_init(struct Idx2EventInfo *_this, int _size) {
    FUNC_START
    _this->size = _size;
    _this->end = 0;
    _this->events = (struct EventInfo *) mymalloc(sizeof(struct EventInfo) * _this->size);
    FUNC_END
    return _this;
}

void iei_finalize(struct Idx2EventInfo *_this) {
    if (_this->events != NULL)myfree(_this->events);
}

struct FreqAmplitude *fa_init(struct FreqAmplitude *_this, int frequencyY, float amplitude) {
    _this->frequencyY = frequencyY;
    _this->amplitude = amplitude;
    return _this;
}

struct FreqAmplitude *fa_init2(struct FreqAmplitude *_this, struct FreqAmplitude *_clone) {
    _this->frequencyY = _clone->frequencyY;
    _this->amplitude = _clone->amplitude;
    _this->peakType = _clone->peakType;
    _this->order = _clone->order;
    return _this;
}

struct FrequencyInfo *
fi_init(struct FrequencyInfo *_this, int frequencyY, struct EventInfo *_event, struct FreqAmplitude *_freq) {
    _this->realEventCount = 0;
    _this->startTimeIdx = 0;
    _this->timesStart = _this->timesEnd = 0;
    fi_InitializeInstanceFields(_this);
    _this->frequencyY = frequencyY;
    fi_addTime2(_this, _event, _freq);
    return _this;
}

struct FrequencyInfo *fi_init2(struct FrequencyInfo *_this, int frequencyY) {
    _this->realEventCount = 0;
    _this->startTimeIdx = 0;
    _this->timesStart = _this->timesEnd = 0;
    fi_InitializeInstanceFields(_this);
    _this->frequencyY = frequencyY;
    return _this;
}

tidx fi_howLongTime(struct FrequencyInfo *_this) {
    return (fi_timesLast(_this)->event->timeIdx - fi_timesFirst(_this)->event->timeIdx) + 1;
}

tidx fi_realHowLongTime(struct FrequencyInfo *_this) {
    return (fi_realTimesLast(_this)->event->timeIdx - fi_realTimesFirst(_this)->event->timeIdx) + 1;
}

bool fi_checkFreq2(struct FrequencyInfo *_this, struct EventInfo *_event, float _minMatchFreqYDistance) {
    if (_event == NULL) {
        _this->endReason = 11;
        return false;
    } else {
        int minSep = MAX_INT;
        int i, sep;
        int times_size;
        struct FrequencyMatch *times_last, *times_last2;
        bool downing;

        struct FreqAmplitude *_freq = 0;
        struct FreqAmplitude *freqs = _event->points;
        int _freqs_size = MAX_EVENT_AMP;
        assert(_freqs_size > 0);
        for (i = _freqs_size - 1; i >= 0; i--) {
            if (freqs[i].frequencyY == 0)continue;
            sep = abs(_this->frequencyY - freqs[i].frequencyY);
            if (sep <= _minMatchFreqYDistance && sep < minSep) {
                _freq = &freqs[i];
                minSep = sep;
            }
        }
        if (_freq != 0) {
            if (fi_timesCount(_this) >= maxFIEventCount) {
                _this->endReason = 11;
                return false;
            }
            times_size = fi_timesCount(_this);
            times_last = (times_size > 0 ? fi_timesLast(_this) : NULL);
            times_last2 = (times_size > 1 ? (fi_timesFirst(_this) + (times_size - 2)) : NULL);
            if (times_size >= 2 * maxMainMatchCount
                && _freq->amplitude > times_last->frequency->amplitude
                && _freq->amplitude > times_last2->frequency->amplitude
                && (times_last->frequency->amplitude * 3 < _freq->amplitude ||
                    times_last2->frequency->amplitude * 3 < _freq->amplitude)) {
                _this->endReason = 5;
                return false;
            }
            downing = false;

            if (times_size >= 2 && times_last2->frequency->amplitude > times_last->frequency->amplitude &&
                ((times_last->frequency->amplitude > _freq->amplitude) ||
                 times_last2->frequency->amplitude > _freq->amplitude)) {
                downing = true;
            }

            fi_addTime2(_this, _event, _freq);
            _this->preDowning = downing;
            return true;
        }
        _this->preDowning = true;
        ++_this->shortFreqCount;

        if (_this->shortFreqCount >= maxShortFreqCount) {
            _this->endReason = 1;
            return false;
        } else {
            return true;
        }
    }
}

void fi_addTime2(struct FrequencyInfo *_this, struct EventInfo *_event, struct FreqAmplitude *_freq) {
    struct FrequencyMatch fm = {_event, _freq};
    fi_addTime(_this, &fm);
}

struct FrequencyInfo *fi_removeSmallMatch(struct FrequencyInfo *_this, int _maxMatchCount) {
    struct FrequencyMatch *matchTimes;
    int times_size, leftIdx, rightIdx;
    struct FreqAmplitude *leftFA = 0, *rightFA = 0;
    int i;

    if (fi_timesLast(_this)->event->timeIdx - fi_timesFirst(_this)->event->timeIdx + 1 > _maxMatchCount) {
        matchTimes = fi_timesFirst(_this);
        times_size = fi_timesCount(_this);
        leftIdx = _this->maxAmplitudeIdx;
        rightIdx = _this->maxAmplitudeIdx;

        for (; (leftIdx >= 0 || rightIdx < times_size);) {
            leftFA = 0;
            rightFA = 0;
            if (leftIdx - 1 >= 0) {
                leftFA = matchTimes[leftIdx - 1].frequency;
            }
            if (rightIdx + 1 < times_size) {
                rightFA = matchTimes[rightIdx + 1].frequency;
            }
            if (leftFA != 0 && rightFA == 0) {
                leftIdx = leftIdx - 1;
            } else if (leftFA == 0 && rightFA != 0) {
                rightIdx = rightIdx + 1;
            } else if (leftFA != 0 && rightFA != 0) {

                if (leftFA->order == 1 && rightFA->order >= 2) {
                    leftIdx = leftIdx - 1;
                } else if (leftFA->order >= 2 && rightFA->order == 1) {
                    rightIdx = rightIdx + 1;
                } else {

                    if (leftFA->amplitude >= rightFA->amplitude) {
                        leftIdx = leftIdx - 1;
                    } else {
                        rightIdx = rightIdx + 1;
                    }
                }
            } else {
                break;
            }
            if ((matchTimes[rightIdx].event->timeIdx - matchTimes[leftIdx].event->timeIdx + 1) >= _maxMatchCount) {
                break;
            }
        }
        _this->timesStart = leftIdx;
        _this->timesEnd = rightIdx + 1;
        assert(leftIdx >= 0 && rightIdx < _this->realEventCount);

    }

    return _this;
}

void fi_addTime(struct FrequencyInfo *_this, const struct FrequencyMatch *_fm) {
    struct FrequencyMatch *matchTimes;
    int times_size;
    struct FreqAmplitude *_freq;
    if (_this->realEventCount >= maxFIEventCount)return;
    _this->realTimes[_this->realEventCount++] = *_fm;
    _this->timesEnd++;
    assert(_this->timesEnd == _this->realEventCount);
    times_size = fi_timesCount(_this);
    matchTimes = fi_times(_this);
    if (_this->maxAmplitudeIdx < 0 || _this->maxAmplitudeIdx >= times_size ||
        _fm->frequency->amplitude > matchTimes[_this->maxAmplitudeIdx].frequency->amplitude) {
        _this->maxAmplitudeIdx = times_size - 1;
    }
    if (_this->startTimeIdx <= 0)_this->startTimeIdx = _fm->event->timeIdx;
    _freq = _fm->frequency;
    if (_freq->peakType == 1)
        _this->bigMatchCount++;
    if (_freq->order == 1)
        _this->orderMatchCount++;
    _this->shortFreqCount = 0;
    _this->prePeakType = _freq->peakType;
    _this->preOrderType = _freq->order;
    if (abs(_this->frequencyY - _freq->frequencyY) <= 1)
        _this->preciseFreqCount++;
}

void fi_InitializeInstanceFields(struct FrequencyInfo *_this) {
    _this->preciseFreqCount = 0;
    _this->shortFreqCount = 0;
    _this->prePeakType = 1;
    _this->preOrderType = 1;
    _this->idx = 0;
    _this->bigMatchCount = 0;
    _this->orderMatchCount = 0;
    _this->maxAmplitudeIdx = -1;
    _this->preDowning = false;
    _this->endReason = 0;
    _this->blockSignalCount = 0;
}

int FrequencyInfo_compare(const void *p_data_left, const void *p_data_right) {
    return (*((struct FrequencyInfo **) p_data_left))->frequencyY -
           (*((struct FrequencyInfo **) p_data_right))->frequencyY;
}

float sumFIAmplitude(struct FrequencyInfo *_this) {
    int ic = fi_timesCount(_this);
    struct FrequencyMatch *match = fi_times(_this);
    int i;
    float sumAmp = 0;
    for (i = 0; i < ic; i++) {
        sumAmp += match[i].frequency->amplitude;
    }
    return sumAmp;
}

int FreqAmplitudeComparator(const void *_o1, const void *_o2) {
    struct FreqAmplitude *o1 = *((struct FreqAmplitude **) _o1);
    struct FreqAmplitude *o2 = *((struct FreqAmplitude **) _o2);
    float r = o1->amplitude - o2->amplitude;
    if (r > 0)
        return 1;
    else if (r == 0) {
        if (o1 == o2)
            return 0;
        else
            return o1 - o2;
    } else
        return -1;
}

int ppFrequencyInfoComparator(const void *_o1, const void *_o2) {
    struct FrequencyInfo *o1 = *((struct FrequencyInfo **) _o1);
    struct FrequencyInfo *o2 = *((struct FrequencyInfo **) _o2);
    return fi_timesFirst(o1)->event->timeIdx - fi_timesFirst(o2)->event->timeIdx;
}

int frequencyToBin(struct SignalAnalyser *_this, float const frequency) {
    const float minFrequency = 0;
    const float maxFrequency = _this->sampleRate / 2;
    int bin = 0;
    static float minCent, maxCent, absCent;
    if (frequency != 0 && frequency >= minFrequency && frequency <= maxFrequency) {
        float binEstimate = (frequency - minFrequency) / maxFrequency * (_this->fftSize / 2);
        bin = ((int) binEstimate) + 1;
        if (bin > _this->fftSize / 2)bin = _this->fftSize / 2;
    }
    return bin;
}

void far_init(struct FFTAmpsRange *_this, int _fftSize, int _minFreqIdx, int _maxFreqIdx) {
    _this->fftSize = _fftSize;
    _this->freqAmplitudeBits = (char *) mymalloc(_fftSize / 2);
    _this->freqAmplitudes = (struct FreqAmplitude *) mymalloc(sizeof(struct FreqAmplitude) * (_fftSize / 2));
    _this->amplitudes = NULL;
    _this->minFreqIdx = _this->maxFreqIdx = 0;
    far_setRange(_this, _minFreqIdx, _maxFreqIdx);
}

void far_setRange(struct FFTAmpsRange *_this, int _minFreqIdx, int _maxFreqIdx) {
#ifdef FS_8136
    _this->minFreqIdx = FSFreqYStart;
    _this->maxFreqIdx = FSFreqYEnd;
#else
    _this->minFreqIdx = _minFreqIdx;
    _this->maxFreqIdx = _maxFreqIdx;
#endif
}

void far_finalize(struct FFTAmpsRange *_this) {
    myfree(_this->freqAmplitudeBits);
    myfree(_this->freqAmplitudes);
}

void far_resetAmps(struct FFTAmpsRange *_this, float *_amplitudes) {
    _this->amplitudes = _amplitudes;
    mymemset(_this->freqAmplitudeBits, 0, _this->fftSize / 2);
}

void fari_init(struct FFTAmpsRangeTop2AvgIniter *_this, int _minEventCount, float _initTop2Avg) {
    _this->inited = false;
    _this->minNormalAmplitudeCount = _minEventCount;
    _this->normalAmplitudeTop2Avg = _initTop2Avg;
    _this->minPeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg * 1.35f;
    _this->minRangePeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg * 1.35f;
    _this->minWeakPeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg;
    _this->rangAvgSNR = 0;
    _this->rangeCurrAvgCount = 0;

}

void fari_onFFTAmpsRange(struct FFTAmpsRangeTop2AvgIniter *_this, struct FFTAmpsRange *_amps,
                         struct Idx2EventInfo *_idx2EventInfo, struct FreqAmpsEventStat *_eventStat) {
    if (!_this->inited && _eventStat->avgAmplitude > 0.0005) {

        float amplitudeTop2Avg = _eventStat->amplitudeTop2Avg;
        if (_this->rangeCurrAvgCount >= RANGE_TOP2AVGINITER_AVG_SIZE) {
            assert(_this->rangeCurrAvgCount == RANGE_TOP2AVGINITER_AVG_SIZE);
            _this->rangAvgSNR =
                    (_this->rangAvgSNR * (_this->rangeCurrAvgCount - 1) + amplitudeTop2Avg) / _this->rangeCurrAvgCount;
        } else {
            _this->rangAvgSNR =
                    (_this->rangAvgSNR * _this->rangeCurrAvgCount + amplitudeTop2Avg) / (_this->rangeCurrAvgCount + 1);
            _this->rangeCurrAvgCount++;
        }
        if (_this->rangAvgSNR < _this->minPeakAmplitudeTop2Avg) {
            _this->sumPeakAmplitudeTop2Avg += amplitudeTop2Avg;
            _this->sumPeakAmplitudeTop2AvgCount++;

            if (_this->sumPeakAmplitudeTop2AvgCount >= _this->minNormalAmplitudeCount) {
                _this->normalAmplitudeTop2Avg = _this->sumPeakAmplitudeTop2Avg / _this->sumPeakAmplitudeTop2AvgCount;
                _this->minPeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg * 1.35f;
                _this->minRangePeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg * 1.35f;
                _this->minLongRangePeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg * LONG_RANGE_MIN_SNR;
                _this->minWeakPeakAmplitudeTop2Avg = _this->normalAmplitudeTop2Avg;
                _this->inited = true;
            }
        } else {
            _this->sumPeakAmplitudeTop2Avg = 0;
            _this->sumPeakAmplitudeTop2AvgCount = 0;
        }
    }
}

void fats_init(struct FreqAmpsEventStat *_this, struct SignalAnalyser *_sa) {
   FUNC_START
    _this->freqY2FreqIdx = NULL;
    _this->freqY2FreqIdxSize = 0;
    _this->longTermAvgAmpCount = 0;
    _this->longTermAvgAmplitude = 0;
    _this->sa = _sa;
    FUNC_END
}

void fats_finalize(struct FreqAmpsEventStat *_this) {
    if (_this->freqY2FreqIdx != NULL)myfree(_this->freqY2FreqIdx);
}

void
fis_init(struct FrequencyInfoSearcher *_this, struct Idx2EventInfo *_idx2Times, struct PoolAllocator *_freqInfoAlloc) {
    _this->idx2Times = _idx2Times;
    _this->freqInfoAlloc = _freqInfoAlloc;
    vector_init(&_this->checkedFreqs, sizeof(struct FrequencyInfo *));
    vector_init(&_this->checkingFreq, sizeof(struct FrequencyInfo *));
}

bool fis_isSignalNeedDiscard(struct FrequencyInfoSearcher *_this, struct FrequencyInfo *_pfi, int _maxFreqYDistance) {

    struct FrequencyInfo **pfi = (struct FrequencyInfo **) vector_nativep(&_this->checkingFreq);
    struct FrequencyInfo *fi = NULL;
    int ii, iic = (int) vector_size(&_this->checkingFreq);
    bool needDiscard = false;

    for (ii = 0; ii < iic; ii++) {
        fi = pfi[ii];
        if (fi->frequencyY == _pfi->frequencyY)continue;

        if (abs(fi->frequencyY - _pfi->frequencyY) <= _maxFreqYDistance
            && (fi_realTimesCount(_pfi) - fi_realTimesCount(fi) <= 1)
            && fi->bigMatchCount * 50 + fi->preciseFreqCount > _pfi->bigMatchCount * 50 + _pfi->preciseFreqCount) {
            needDiscard = true;
            break;
        }
    }

    if (!needDiscard && (iic = vector_size(&_this->checkedFreqs)) > 0) {

        pfi = (struct FrequencyInfo **) vector_nativep(&_this->checkedFreqs);
        for (ii = 0; ii < iic; ii++) {
            fi = pfi[ii];

            if (abs(fi->frequencyY - _pfi->frequencyY) <= _maxFreqYDistance
                && fi_realTimesCount(fi) == fi_realTimesCount(_pfi)
                &&
                fi->realBigMatchCount * 50 + fi->preciseFreqCount > _pfi->bigMatchCount * 50 + _pfi->preciseFreqCount) {
                needDiscard = true;
                break;
            }
        }
    }
    return needDiscard;
}

void fis_onFFTEvent(struct FrequencyInfoSearcher *_this, struct EventInfo *_ei, float _maxFreqYDistance) {
    bool freqTruncate = false;
    bool signalTruncate = false;
    bool checkingFreqHasSignal = false;
    bool erased = false;
    int i, c, x, f, fc;
    struct FrequencyInfo *pfi;
    struct FrequencyInfo **fiCheckingFreq;
    struct FrequencyInfo *checkedFreq;
    struct FreqAmplitude *tempAmp;
    struct FrequencyInfo fi;
    struct FrequencyInfo **fiCheckedFreqs;
    struct FreqAmplitude *faValidAmplitudes;
    struct FreqAmplitude *swapBuf;
    bool freqRemoved = false;
    struct FrequencyInfo *tempFI;
    tidx earliesTime = MAX_TIME_IDX;
    struct FrequencyInfo *similarFI;
    tidx firstFITime;
    tidx li;
    struct EventInfo *ei;

    if (vector_size(&_this->checkedFreqs) > 0)vector_clear(&_this->checkedFreqs);

    {

        fiCheckingFreq = (struct FrequencyInfo **) vector_nativep(&_this->checkingFreq);
        for (i = 0; i < (int) vector_size(&_this->checkingFreq); (!erased) ? i++ : i) {
            erased = false;
            pfi = fiCheckingFreq[i];
            if (!fi_checkFreq2(pfi, _ei, _maxFreqYDistance)) {

                if (fi_howLongTime(pfi) >= freqCountLong) {

                    struct FrequencyInfo *tempCheckingFreq = NULL;
                    int ii;
                    bool hasMoreLongSignal = fis_isSignalNeedDiscard(_this, pfi, _maxFreqYDistance);
                    if (!hasMoreLongSignal) {
                        vector_push_back(&_this->checkedFreqs, &pfi);
                    } else {

                    }
                }

                vector_erase(&_this->checkingFreq, i);
                erased = true;
            }
        }

        fiCheckingFreq = (struct FrequencyInfo **) vector_nativep(&_this->checkingFreq);

        fi_init2(&fi, 0);
        fiCheckedFreqs = (struct FrequencyInfo **) vector_nativep(&_this->checkedFreqs);
        faValidAmplitudes = _ei->points;
        for (f = 0, fc = MAX_EVENT_AMP; f < fc; f++) {
            tempAmp = &faValidAmplitudes[f];
            if (tempAmp == NULL)break;
            fi.frequencyY = tempAmp->frequencyY;
            freqRemoved = false;
            for (i = 0; i < (int) vector_size(&_this->checkedFreqs); i++) {
                if (abs(fi.frequencyY - fiCheckedFreqs[i]->frequencyY) <= _maxFreqYDistance) {
                    freqRemoved = true;
                    break;
                }
            }
            if (freqRemoved)
                continue;
            pfi = &fi;
            if (vector_oindexOf(&_this->checkingFreq, &pfi, FrequencyInfo_compare) < 0) {
                tempFI = (struct FrequencyInfo *) pa_new(_this->freqInfoAlloc);
                fi_init2(tempFI, tempAmp->frequencyY);

                earliesTime = MAX_TIME_IDX;
                fiCheckingFreq = (struct FrequencyInfo **) vector_nativep(&_this->checkingFreq);
                for (i = 0, c = vector_size(&_this->checkingFreq); i < c; i++) {
                    similarFI = fiCheckingFreq[i];
                    if (abs(similarFI->frequencyY - tempFI->frequencyY) <= _maxFreqYDistance) {
                        firstFITime = fi_timesFirst(similarFI)->event->timeIdx;
                        if (firstFITime < earliesTime) {
                            earliesTime = firstFITime;
                        }
                    }
                }
                if (earliesTime < MAX_TIME_IDX) {
                    for (li = earliesTime; li < iei_idx(_this->idx2Times); li++) {
                        ei = iei_get(_this->idx2Times, li);
                        if (ei == NULL)continue;
                        if (!fi_checkFreq2(tempFI, ei, _maxFreqYDistance))
                            fi_timesClear(tempFI);
                    }
                }
                fi_addTime2(tempFI, _ei, tempAmp);
                vector_oinsert(&_this->checkingFreq, &tempFI, FrequencyInfo_compare);
            }
        }
    }
}

void fis_truncateSignal(struct FrequencyInfoSearcher *_this, float _maxFreqYDistance) {
    int i, c;

    struct FrequencyInfo **fiCheckingFreq = (struct FrequencyInfo **) vector_nativep(&_this->checkingFreq);
    struct FrequencyInfo *fi;
    for (i = 0, c = vector_size(&_this->checkingFreq); i < c; i++) {
        fi = fiCheckingFreq[i];
        if (fi_howLongTime(fi) >= freqCountLong) {
            bool hasMoreLongSignal = fis_isSignalNeedDiscard(_this, fi, _maxFreqYDistance);
            if (!hasMoreLongSignal) {
                vector_push_back(&_this->checkedFreqs, &fi);
            } else {

            }
        }
    }
    vector_clear(&_this->checkingFreq);
}

void fis_finalize(struct FrequencyInfoSearcher *_this) {
    vector_finalize(&_this->checkedFreqs);
    vector_finalize(&_this->checkingFreq);
}

void bs_init(struct BlockSearcher *_this, struct Idx2EventInfo *_idx2Times, short _blockFreqY, float _maxFreqYDistance,
             struct SignalAnalyser *_sa) {
    _this->idx2Times = _idx2Times;
    _this->blockFreqY = _blockFreqY;
    _this->maxFreqYDistance = _maxFreqYDistance;
    _this->allBlockFinished = true;
    _this->blockCount = 0;
    assert(BLOCKSEARCH_BLOCK_CACHE_COUNT >= 3);
    bs_reset(_this);
    pd_init(&_this->peakDetector, _blockFreqY, _sa);

    _this->onFFTEvent = bs_onFFTEvent;
}

void bs_reset2(struct BlockSearcher *_this, short _blockFreqY) {
    _this->blockFreqY = _blockFreqY;
    pd_reset2(&_this->peakDetector, _blockFreqY);
    bs_reset(_this);
}

void bs_reset(struct BlockSearcher *_this) {
    int i;
    _this->unsedBlockIdx = -1;
    _this->okBlockIdx = 1;
    _this->blockCount = 0;
    fi_init2(&_this->blockFIs[0], _this->blockFreqY);
    for (i = 1; i < BLOCKSEARCH_BLOCK_CACHE_COUNT; i++) {
        fi_init2(&_this->blockFIs[i], 0);
    }
}

bool bs_hasBlock(struct BlockSearcher *_this, struct FrequencyInfo **_blockHead, struct FrequencyInfo **_blockTail,
                 bool *_isEndBlock) {
    int minUnsedBlockIdx = (_this->allBlockFinished ? _this->okBlockIdx : (_this->okBlockIdx + 1));
    if (_this->unsedBlockIdx >= minUnsedBlockIdx) {
        assert(_this->unsedBlockIdx >= 0 && _this->unsedBlockIdx < BLOCKSEARCH_BLOCK_CACHE_COUNT - 1);
        *_blockHead = &_this->blockFIs[_this->unsedBlockIdx + 1];
        *_blockTail = &_this->blockFIs[_this->unsedBlockIdx];
        _this->unsedBlockIdx--;
        *_isEndBlock = (_this->unsedBlockIdx < _this->okBlockIdx);
        assert(*_isEndBlock ? _this->allBlockFinished : true);
        return true;
    }
    return false;
}

int bs_removeWeakestBlock(struct BlockSearcher *_this, int _startBlockIdx, int _blockCount,
                          struct FrequencyInfo *_removedFI) {
    float peaks[BLOCKSEARCH_BLOCK_CACHE_COUNT];
    int minPeakIdx;
    int i;
    assert(_blockCount <= BLOCKSEARCH_BLOCK_CACHE_COUNT);
    for (i = 0; i < _blockCount; i++) {
        peaks[i] = sumFIAmplitude(&_this->blockFIs[_startBlockIdx + i]);
        if (_this->blockFIs[_startBlockIdx + i].orderMatchCount == 0)peaks[i] = peaks[i] * 0.1f;
    }
    minPeakIdx = searchFmin(peaks, _blockCount);
    assert(minPeakIdx >= 0 && minPeakIdx < _blockCount);
    *_removedFI = _this->blockFIs[_startBlockIdx + minPeakIdx];
    for (i = minPeakIdx - 1; i >= 0; i--) {
        _this->blockFIs[_startBlockIdx + i + 1] = _this->blockFIs[_startBlockIdx + i];
    }
    return _startBlockIdx + minPeakIdx;
}

void bs_onFFTEvent(struct BlockSearcher *_this, struct EventInfo *_ei) {
    bool hasBlockSignal = false;
    if (_this->allBlockFinished) {
        if (_this->blockCount > 0 || fi_realTimesCount(&_this->blockFIs[0]) > 0)
            bs_reset(_this);
    } else {

        if (_this->okBlockIdx == 0) {
            int i;
            for (i = BLOCKSEARCH_BLOCK_CACHE_COUNT - 2; i >= 0; i--) {
                _this->blockFIs[i + 1] = _this->blockFIs[i];
            }
            fi_init2(&_this->blockFIs[0], _this->blockFreqY);
            _this->okBlockIdx = 1;
            if (_this->blockCount == 2)_this->unsedBlockIdx = 1;
            else if (_this->blockCount > 2)
                _this->unsedBlockIdx++;
        }
        assert(_this->unsedBlockIdx < BLOCKSEARCH_BLOCK_CACHE_COUNT &&
               (_this->unsedBlockIdx < 0 || _this->unsedBlockIdx == _this->okBlockIdx));
    }

    if (_ei != NULL) {
        if ((_ei->p1.frequencyY != 0 && abs(_ei->p1.frequencyY - _this->blockFreqY) <= 1 && _ei->p1.peakType != 3)
            || (_ei->p2.frequencyY != 0 && abs(_ei->p2.frequencyY - _this->blockFreqY) <= 1 && _ei->p2.peakType != 3)
            || (_ei->p3.frequencyY != 0 && abs(_ei->p3.frequencyY - _this->blockFreqY) <= 1 && _ei->p3.peakType != 3)) {
            hasBlockSignal = true;
        }
        _this->allBlockFinished = false;
    } else
        _this->allBlockFinished = true;
    if ((_ei == NULL && fi_realTimesCount(&_this->blockFIs[0]) > 0)
        || ((hasBlockSignal || fi_realTimesCount(&_this->blockFIs[0]) > 0) &&
            !fi_checkFreq2(&_this->blockFIs[0], _ei, 1))) {
        bool validBlock = false;
        struct FrequencyInfo removedFI = _this->blockFIs[0];
        int removedFIIdx = -1;

#ifdef BLOCK_SIGNAL_MUST_HAS_BIG_PEAK
        if (fi_realTimesCount(&_this->blockFIs[0]) >= freqCountLong && _this->blockFIs[0].bigMatchCount > 0 && _this->blockFIs[0].preciseFreqCount > 1)
#else
        if (fi_realTimesCount(&_this->blockFIs[0]) >= freqCountLong

            && _this->blockFIs[0].preciseFreqCount > 1
                )
#endif
        {
            assert(_this->okBlockIdx == 1);
            fi_removeSmallMatch(&_this->blockFIs[0], maxBlockMatchCount);

            if (_this->blockCount > 0

                && fi_timesFirst(&_this->blockFIs[0])->event->timeIdx -
                   fi_timesFirst(&_this->blockFIs[1])->event->timeIdx < ((1 + RS_CORRECT_SIZE) * preciseMatchCount)
                    ) {

                removedFIIdx = bs_removeWeakestBlock(_this, 0, 2, &removedFI);
                validBlock = false;

                if (removedFIIdx > 0) {
                    assert(removedFIIdx == 1);

                }
            } else {

                validBlock = true;

                assert(BLOCKSEARCH_BLOCK_CACHE_COUNT >= 3);

                if (_this->blockCount == 2) {
                    bool s2_1OK = (fi_timesFirst(&_this->blockFIs[1])->event->timeIdx -
                                   fi_timesFirst(&_this->blockFIs[2])->event->timeIdx) >=
                                  ((RS_CORRECT_BLOCK_SIZE + RS_CORRECT_SIZE) * preciseMatchCount);
                    assert(_this->unsedBlockIdx >= 1);
                    if (!s2_1OK) {
                        removedFIIdx = bs_removeWeakestBlock(_this, 0, 3, &removedFI);
                        validBlock = false;
                    }
                } else if (_this->blockCount > 2) {
                    tidx s2_1_Len = (fi_timesFirst(&_this->blockFIs[1])->event->timeIdx -
                                     fi_timesFirst(&_this->blockFIs[2])->event->timeIdx);
                    bool s2_1OK = s2_1_Len > ((RS_CORRECT_BLOCK_SIZE + RS_CORRECT_SIZE) * preciseMatchCount);
                    assert(_this->unsedBlockIdx >= 1);
                    if (!s2_1OK) {
                        removedFIIdx = bs_removeWeakestBlock(_this, 0, 2, &removedFI);
                        validBlock = false;
                    }
                }
            }

            if (removedFIIdx != 0) {
                pd_onBlock(&_this->peakDetector, &_this->blockFIs[0]);
                _this->blockFIs[(removedFIIdx > 0 ? 1 : 0)].blockSignalCount = pd_preBlockPeakCount(
                        &_this->peakDetector);
            }
            if (removedFIIdx == 1) {

                _this->blockFIs[1].blockSignalCount += removedFI.blockSignalCount;
                _this->blockFIs[0].blockSignalCount = _this->blockFIs[1].blockSignalCount;
            }
        }

        if (validBlock) {
            pinfo(TAG_SignalAnalyser, "bs_onFFTEvent valid block\n");
            _this->okBlockIdx = 0;
            _this->blockCount++;
        }
        assert(fi_realTimesCount(&_this->blockFIs[0]) > 0);
        if (!validBlock) {
            fi_init2(&_this->blockFIs[0], _this->blockFreqY);
        }
    }

    pd_onFFTEvent(&_this->peakDetector, _ei);
    if (_ei == NULL
        && _this->blockCount >= 2
        && ((fi_timesFirst(&_this->blockFIs[_this->okBlockIdx])->event->timeIdx -
             fi_timesFirst(&_this->blockFIs[_this->okBlockIdx + 1])->event->timeIdx) <
            ((RS_CORRECT_BLOCK_SIZE + RS_CORRECT_SIZE - 1) * preciseMatchCount))) {

        bool lastBlockInvalid = _this->blockFIs[_this->okBlockIdx].blockSignalCount < 2
                                || (_this->blockFIs[_this->okBlockIdx].blockSignalCount == 2 &&
                                    sumFIAmplitude(&_this->blockFIs[_this->okBlockIdx]) <
                                    sumFIAmplitude(&_this->blockFIs[_this->okBlockIdx + 1]) * 0.2);

        if (lastBlockInvalid) {
            fi_init2(&_this->blockFIs[_this->okBlockIdx], _this->blockFreqY);
            _this->okBlockIdx++;
            _this->blockCount--;
        }
    }

    if (_ei == NULL || _this->allBlockFinished) {
        pd_reset(&_this->peakDetector);
    }
}

void pd_init(struct PeakDetector *_this, int _blockFreqY, struct SignalAnalyser *_sa) {
    _this->blockFreqY = _blockFreqY;
    _this->sa = _sa;
    pd_reset(_this);
}

void pd_onFFTEvent(struct PeakDetector *_this, struct EventInfo *_ei) {
    if (_ei != NULL && _ei->p1.frequencyY != 0) {
        int idx = getIdxFromFrequencyFromCache(_this->sa, _ei->p1.frequencyY);
        if (idx >= 0 && idx < DEFAULT_CODE_FREQUENCY_SIZE) {
            if ((_ei->timeIdx - _this->lastPosTimes[idx]) > (RangeMatchCount * 3 / 2)) {
                int pos = _this->nextPeakPos % (sizeof(_this->peakStartTimes) / sizeof(struct PeakInfo));
                _this->peakStartTimes[pos].timeIdx = _ei->timeIdx;
                _this->peakStartTimes[pos].frequencyY = _ei->p1.frequencyY;
                _this->nextPeakPos++;
                _this->lastPosTimes[idx] = _ei->timeIdx;

                if (idx != DEFAULT_START_TOKEN) {
                    _this->peakCount++;
                }
            } else if ((_ei->timeIdx - _this->lastPosTimes[idx]) <= 2) {
                _this->lastPosTimes[idx] = _ei->timeIdx;
            }
        }
    }
}

void pd_onBlock(struct PeakDetector *_this, struct FrequencyInfo *_blockFI) {
    tidx blockEndPos = fi_timesLast(_blockFI)->event->timeIdx;
    int i, minIdx = _this->nextPeakPos - (sizeof(_this->peakStartTimes) / sizeof(struct PeakInfo)) + 1;
    int pos;
    assert(abs(_blockFI->frequencyY - _this->blockFreqY) <= 1);
    if (minIdx < 0)minIdx = 0;

    for (i = _this->nextPeakPos - 1; i >= minIdx; i--) {
        pos = i % (sizeof(_this->peakStartTimes) / sizeof(struct PeakInfo));
        if (_this->peakStartTimes[pos].timeIdx <= blockEndPos &&
            abs(_this->peakStartTimes[pos].frequencyY - _this->blockFreqY) <= 1) {
            break;
        }
    }
    if (i < minIdx) {

        i = _this->nextPeakPos - 1;
    }
    _this->preBlockPeakCount = _this->peakCount - (_this->nextPeakPos - 1 - i);

    _this->peakCount = _this->nextPeakPos - 1 - i;
    _this->nextPeakPos = 0;
}

void pd_reset2(struct PeakDetector *_this, int _blockFreqY) {
    _this->blockFreqY = _blockFreqY;
    pd_reset(_this);
}

void pd_reset(struct PeakDetector *_this) {
    _this->peakCount = 0;
    _this->preBlockPeakCount = 0;
    _this->nextPeakPos = 0;
    mymemset(_this->peakStartTimes, 0, sizeof(_this->peakStartTimes));
    mymemset(_this->lastPosTimes, 0, sizeof(_this->lastPosTimes));
}

int pd_blockPeakCount(struct PeakDetector *_this) {
    return _this->peakCount;
}

int pd_preBlockPeakCount(struct PeakDetector *_this) {
    return _this->preBlockPeakCount;
}

void fats_onFFTAmpsRange(struct FreqAmpsEventStat *_this, struct FFTAmpsRange *_amps, float _minPeak2Avg,
                         float _minStatPeak2Avg, float _maxFreqYDistance) {
    struct FreqAmplitude *maxFreqAmplitude = 0;
    struct FreqAmplitude *max2FreqAmplitude = 0;
    struct FreqAmplitude *max3FreqAmplitude = 0;
    struct FreqAmplitude *max4FreqAmplitude = 0;
    int i, maxFreqIdxIn19 = -1, max2FreqIdxIn19 = -1, max3FreqIdxIn19 = -1;
    int freqY;
    bool hasValidAmplitude = false;
    float avgAmplitude;
    float amplitudeTop2Avg;
    struct FreqAmplitude tempAmp;
    int maxAmpPos1, maxAmpPos2, avgAmpCount;
    int minFreqIdx = _amps->minFreqIdx, maxFreqIdx = _amps->maxFreqIdx;
    struct FreqAmplitude *freqAmplitudes = _this->freqAmplitudes;
    float *amplitudes = _amps->amplitudes;
    float eventTotalAmplitude = 0.00001;
    tidx maxAmpIdx = 0, max2AmpIdx = 0, max3AmpIdx = 0;
    float minValidPeak = 0, minStatPeak = 0;

    ei_init(&_this->event);

    mymemset(_this->event.points, 0, MAX_EVENT_AMP * sizeof(struct FreqAmplitude *));
    _this->avgAmplitude = 0;
    _this->hasValidAmplitude = false;
    _this->p1 = NULL;
    assert(_amps->maxFreqIdx <= _amps->fftSize / 2);

    if (_this->freqY2FreqIdxSize != (maxFreqIdx - minFreqIdx + 1)) {
        if (_this->freqY2FreqIdx != NULL) {
            myfree(_this->freqY2FreqIdx);
        }
        _this->freqY2FreqIdxSize = maxFreqIdx - minFreqIdx + 1;
        _this->freqY2FreqIdx = (char *) mymalloc(_this->freqY2FreqIdxSize);
        for (i = minFreqIdx; i < maxFreqIdx; i++) {
#ifdef FS_8136
            _this->freqY2FreqIdx[i-minFreqIdx] = getFSIdxFromFrequency(i);
#else
            _this->freqY2FreqIdx[i - minFreqIdx] = getIdxFromFrequencyFromCache(_this->sa, i);
#endif
        }
    }
    assert(_this->freqY2FreqIdx != NULL && _this->freqY2FreqIdxSize > 0);

    for (i = minFreqIdx; i < maxFreqIdx; i++) {
        eventTotalAmplitude += amplitudes[i];
        freqY = i;
#ifdef FS_8136
        freqY = correctFreqIdx(_this, _this->sa, freqY);
#endif
        dbgprint("%.4f,", amplitudes[i]);
        if (maxAmpIdx == 0 || amplitudes[i] > amplitudes[maxAmpIdx]) {
            maxAmpIdx = i;
        }
    }
    maxFreqIdxIn19 = _this->freqY2FreqIdx[maxAmpIdx - minFreqIdx];
    dbgprint("%s", "\n");
    fa_init(&freqAmplitudes[0], maxAmpIdx, amplitudes[maxAmpIdx]);
    maxFreqAmplitude = &freqAmplitudes[0];
    maxFreqAmplitude->peakType = 1;
    maxFreqAmplitude->order = 1;
    _this->p1 = maxFreqAmplitude;

    for (i = minFreqIdx; i < maxFreqIdx; i++) {
        freqY = i;
#ifdef FS_8136
        freqY = correctFreqIdx(_this, _this->sa, freqY);
#endif

        if (max2AmpIdx == 0 || amplitudes[i] > amplitudes[max2AmpIdx]) {
            if (_this->freqY2FreqIdx[i - minFreqIdx] == maxFreqIdxIn19
                || (i > minFreqIdx && _this->freqY2FreqIdx[i - minFreqIdx - 1] == maxFreqIdxIn19)
                || (i < maxFreqIdx - 1 && _this->freqY2FreqIdx[i - minFreqIdx + 1] == maxFreqIdxIn19))
                continue;

            max2AmpIdx = i;
        }
    }
    max2FreqIdxIn19 = _this->freqY2FreqIdx[max2AmpIdx - minFreqIdx];
    fa_init(&freqAmplitudes[1], max2AmpIdx, amplitudes[max2AmpIdx]);
    max2FreqAmplitude = &freqAmplitudes[1];
    max2FreqAmplitude->peakType = 2;
    max2FreqAmplitude->order = 2;

    for (i = minFreqIdx; i < maxFreqIdx; i++) {
        freqY = i;
#ifdef FS_8136
        freqY = correctFreqIdx(_this, _this->sa, freqY);
#endif

        if (max3AmpIdx == 0 || amplitudes[i] > amplitudes[max3AmpIdx]) {
            if (_this->freqY2FreqIdx[i - minFreqIdx] == maxFreqIdxIn19
                || (i > minFreqIdx && _this->freqY2FreqIdx[i - minFreqIdx - 1] == maxFreqIdxIn19)
                || (i < maxFreqIdx && _this->freqY2FreqIdx[i - minFreqIdx + 1] == maxFreqIdxIn19))
                continue;
            if (_this->freqY2FreqIdx[i - minFreqIdx] == max2FreqIdxIn19
                || (i > minFreqIdx && _this->freqY2FreqIdx[i - minFreqIdx - 1] == max2FreqIdxIn19)
                || (i < maxFreqIdx - 1 && _this->freqY2FreqIdx[i - minFreqIdx + 1] == max2FreqIdxIn19))
                continue;

            max3AmpIdx = i;
        }
    }
    max3FreqIdxIn19 = _this->freqY2FreqIdx[max3AmpIdx - minFreqIdx];
    fa_init(&freqAmplitudes[2], max3AmpIdx, amplitudes[max3AmpIdx]);
    max3FreqAmplitude = &freqAmplitudes[2];
    max3FreqAmplitude->peakType = 2;
    max3FreqAmplitude->order = 3;

    maxAmpPos1 = maxFreqAmplitude->frequencyY;
    maxAmpPos2 = max2FreqAmplitude->frequencyY;
    avgAmpCount = maxFreqIdx - minFreqIdx;
    avgAmplitude = eventTotalAmplitude / avgAmpCount;
    if (_this->longTermAvgAmplitude == 0)_this->longTermAvgAmplitude = avgAmplitude;
    if (_this->longTermAvgAmplitude == 0)_this->longTermAvgAmplitude = 0.000001f;
    assert(_this->longTermAvgAmplitude != 0);
    hasValidAmplitude = maxFreqAmplitude->amplitude /
                        ((_this->longTermAvgAmplitude > avgAmplitude) ? avgAmplitude : _this->longTermAvgAmplitude) >
                        _minPeak2Avg;

    if (hasValidAmplitude) {
#define MINUS_BOUND_SIZE 2
        int maxMinPos = maxAmpPos1 - MINUS_BOUND_SIZE, maxMaxPos = maxAmpPos1 + MINUS_BOUND_SIZE;
        int max2MinPos = maxAmpPos2 - MINUS_BOUND_SIZE, max2MaxPos = maxAmpPos2 + MINUS_BOUND_SIZE;
        int k;
        for (k = maxMinPos; k < maxMaxPos; k++) {
            if (k > minFreqIdx && k < maxFreqIdx) {
                eventTotalAmplitude = eventTotalAmplitude - amplitudes[k];
                avgAmpCount--;
            }
        }
        for (k = max2MinPos; k < max2MaxPos; k++) {
            if (k > minFreqIdx && k < maxFreqIdx && !(k >= maxMinPos && k <= maxMaxPos)) {
                eventTotalAmplitude = eventTotalAmplitude - amplitudes[k];
                avgAmpCount--;
            }
        }
    }
    avgAmplitude = eventTotalAmplitude / avgAmpCount;

    if (_this->longTermAvgAmpCount < LONG_TERM_AVG_AMPLITUDE) {
        _this->longTermAvgAmplitude = (_this->longTermAvgAmplitude * _this->longTermAvgAmpCount + avgAmplitude) /
                                      (_this->longTermAvgAmpCount + 1);
        _this->longTermAvgAmpCount++;
    } else {
        _this->longTermAvgAmplitude = (_this->longTermAvgAmplitude * (_this->longTermAvgAmpCount - 1) + avgAmplitude) /
                                      _this->longTermAvgAmpCount;
    }
    avgAmplitude = ((_this->longTermAvgAmplitude > avgAmplitude) ? avgAmplitude : _this->longTermAvgAmplitude);

    if (avgAmplitude == 0)avgAmplitude = 0.000001f;
    _this->avgAmplitude = avgAmplitude;
    _this->amplitudeTop2Avg = amplitudeTop2Avg = maxFreqAmplitude->amplitude / avgAmplitude;
    _this->hasValidAmplitude = amplitudeTop2Avg >= _minPeak2Avg;
    _this->eventTotalAmplitude = eventTotalAmplitude;
    minValidPeak = avgAmplitude * _minPeak2Avg;
    minStatPeak = avgAmplitude * _minStatPeak2Avg;

    if (maxFreqAmplitude->amplitude >= minValidPeak) {
        _this->event.p1 = *maxFreqAmplitude;

        if (max2FreqAmplitude != NULL && max2FreqAmplitude->amplitude >= minValidPeak) {
            float max2_max = max2FreqAmplitude->amplitude / maxFreqAmplitude->amplitude;
            _this->event.p2 = *max2FreqAmplitude;
            if (max2_max > 0.5) {
                _this->event.p1.peakType = 2;
            }
            if (max3FreqAmplitude != NULL && max3FreqAmplitude->amplitude >= minValidPeak) {
                float max3_max2 = max3FreqAmplitude->amplitude / max2FreqAmplitude->amplitude;

                {
                    _this->event.p3 = *max3FreqAmplitude;
                }
            } else if (max3FreqAmplitude != NULL && max3FreqAmplitude->amplitude >= minStatPeak) {
                _this->event.p3 = *max3FreqAmplitude;
                _this->event.p3.peakType = 3;
            }
        } else if (max2FreqAmplitude != NULL && max2FreqAmplitude->amplitude >= minStatPeak) {
            _this->event.p2 = *max2FreqAmplitude;
            _this->event.p2.peakType = 3;
        }
    }
#ifdef STAT_WEAK_PEAK
    else if(maxFreqAmplitude->amplitude >= minStatPeak)
    {
        _this->event.p1 = *maxFreqAmplitude;
        _this->event.p1.peakType = 3;
    }
#endif
}

/**
 * 信号分析器
 * @param _this
 * @param _sampleRate
 * @param _channel
 * @param _bits
 * @param _fftSize
 * @param _overlap
 * @return
 */
struct SignalAnalyser *sa_init(struct SignalAnalyser *_this, int _sampleRate, int _channel, int _bits, int _fftSize, int _overlap)
{
    FUNC_START
    _this->maxFreqYDistance = (FREQ_DISTANCE / (((float) _sampleRate) / (_fftSize / 2)));
    _this->fftSize = _fftSize;
    _this->sampleRate = _sampleRate;

    fats_init(&_this->eventStat, _this);
    iei_init(&_this->idx2Times, MAX_EVENT_COUNT);
#ifdef SEARCH_SIMILAR_SIGNAL
    vector_init(&_this->similarSignals, sizeof(struct TimeRangeSignal));
    _this->blockCount = 0;
    memset(_this->blocks, 0, sizeof(_this->blocks));
#endif
    far_init(&_this->ampsRange, _this->fftSize, 0, 0);
    fari_init(&_this->top2AvgIniter, SNR_INIT_STAT_SIZE, 5);
#ifdef MATCH_FI
    pa_init(&_this->freqInfoAlloc, sizeof(struct FrequencyInfo), NULL);
    fis_init(&_this->fiSearcher, &_this->idx2Times, &_this->freqInfoAlloc);
#endif
    bs_init(&_this->blockSearcher, &_this->idx2Times, 0, _this->maxFreqYDistance, _this);

    _this->overlap = _overlap;
    _this->preAnalyTotalTime = 0;
    _this->analyFreqTotalTime = 0;
    _this->anyseSignalTotalTime = 0;
    _this->analyseFreqTop3TotalTime = 0;
    _this->analyseFreqValidTotalTime = 0;
    _this->recoging = false;
    _this->signalOK = false;
    _this->finished = true;
    _this->newSignal = true;
    _this->freqY2IdxCaches = NULL;
    _this->freqY2IdxCacheCount = 0;
    _this->freqY2IdxCacheMinIdx = 0;
    _this->oneSignalLen = preciseMatchCount;
    _this->preTargetIdx = -1;
    _this->prepreTargetIdx = -1;
    _this->preTaretPeakType = 0;
    _this->prepreTaretPeakType = 0;

    _this->freqRanges = NULL;
    _this->listener = NULL;
    _this->CODE_Y = NULL;
    _this->totalSignalCount = 0;
    _this->okSignalCount = 0;
    _this->preSignalTime = 0;
    _this->preValidFreqAmplitudeTime = 0;
    _this->firstSignalTime = 0;
    _this->firstValidFreqAmplitudeTime = -1;
    _this->minRegcoSignalCount = 10;
    _this->rangAvgSNR = 0;
    _this->rangeCurrAvgCount = 0;
    _this->preValidRangeAvgSNRIdx = 0;
    _this->preValidLongRangeAvgSNRIdx = 0;

    sa_setFreqs(_this, DEFAULT_CODE_FREQUENCY);

    FUNC_END
    return _this;
}

void sa_finalize(struct SignalAnalyser *_this) {
    if (_this->CODE_Y != NULL) {
        myfree(_this->CODE_Y);
    }
    fats_finalize(&_this->eventStat);
    iei_finalize(&_this->idx2Times);
    far_finalize(&_this->ampsRange);
#ifdef SEARCH_SIMILAR_SIGNAL
    vector_finalize(&_this->similarSignals);
#endif
#ifdef MATCH_FI
    fis_finalize(&_this->fiSearcher);
    pa_finalize(&_this->freqInfoAlloc);
#endif

    if (_this->freqY2IdxCaches != NULL)myfree(_this->freqY2IdxCaches);
    if (_this->freqRanges != NULL)myfree(_this->freqRanges);
}

void initCodeY(struct SignalAnalyser *_this, bool _force) {
    int i;
    if (_this->CODE_Y == 0 || _force) {
        if (_this->CODE_Y == 0)_this->CODE_Y = (int *) mymalloc(sizeof(int) * DEFAULT_CODE_FREQUENCY_SIZE);
        for (i = 0; i < DEFAULT_CODE_FREQUENCY_SIZE; i++) {
            _this->CODE_Y[i] = frequencyToBin(_this, _this->CODE_FREQUENCY[i]);
        }
        _this->pitchDistance = _this->CODE_Y[2] - _this->CODE_Y[1];

    }
}

int getIdxFromFrequency(struct SignalAnalyser *_this, int _pitch) {

    int pos = mybinarySearch_(&_pitch, _this->CODE_Y + 1, DEFAULT_CODE_FREQUENCY_SIZE - 1 - 1, sizeof(int),
                              intComparator);

    if (pos >= 0) {
        pos += 1;
    } else {
        pos = -(-(pos + 1) + 1) - 1;
    }
    if (pos >= 0)
        return pos;
    else {
        int insertPos = -(pos + 1);
        int leftDistance = (insertPos == 0) ? 1000 : (_pitch - _this->CODE_Y[insertPos - 1]);
        int rightDistance = (insertPos >= DEFAULT_CODE_FREQUENCY_SIZE) ? 1000 : (_this->CODE_Y[insertPos] - _pitch);
        if (leftDistance <= rightDistance) {
            if (leftDistance <= _this->maxFreqYDistance)
                return insertPos - 1;
            else
                return -1;
        } else {
            if (rightDistance <= _this->maxFreqYDistance)
                return insertPos;
            else
                return -1;
        }
    }
}

int getIdxFromFrequencyFromCache(struct SignalAnalyser *_this, int _pitch) {
    if (_pitch >= _this->freqY2IdxCacheMinIdx && _pitch <= _this->freqY2IdxCacheMinIdx + _this->freqY2IdxCacheCount) {
        return _this->freqY2IdxCaches[_pitch - _this->freqY2IdxCacheMinIdx];
    }
    return getIdxFromFrequency(_this, _pitch);
}

void sa_setFreqs(struct SignalAnalyser *_this, int *_freqs) {
    int i, minIdx, maxIdx;
    LOG("freqs=%d\n",*_freqs);
    _this->CODE_FREQUENCY = _freqs;
    _this->minFrequency = MAX_INT;
    _this->maxFrequency = 0;
    for (i = 0; i < DEFAULT_CODE_FREQUENCY_SIZE; i++) {
        if (_freqs[i] < _this->minFrequency) {
            _this->minFrequency = _freqs[i];
        }
        if (_freqs[i] > _this->maxFrequency) {
            _this->maxFrequency = _freqs[i];
        }
    }
    _this->minFrequency = _this->minFrequency - FREQ_DISTANCE;
    _this->maxFrequency = _this->maxFrequency + FREQ_DISTANCE;
    _this->header = frequencyToBin(_this, _this->CODE_FREQUENCY[DEFAULT_START_TOKEN]);
    _this->tail = frequencyToBin(_this, _this->CODE_FREQUENCY[DEFAULT_STOP_TOKEN]);
    minIdx = frequencyToBin(_this, _this->minFrequency);
    maxIdx = frequencyToBin(_this, _this->maxFrequency);

    initCodeY(_this, true);
    if (_this->freqY2IdxCacheMinIdx != minIdx) {
        if (_this->freqY2IdxCaches != NULL) {
            myfree(_this->freqY2IdxCaches);
            _this->freqY2IdxCaches = NULL;
            _this->freqY2IdxCacheCount = 0;
            _this->freqY2IdxCacheMinIdx = 0;
        }
    }
    _this->freqY2IdxCacheCount = maxIdx - minIdx + 1;
    _this->freqY2IdxCacheMinIdx = minIdx;
    assert(_this->freqY2IdxCacheCount < 150);
    if (_this->freqY2IdxCaches == NULL)_this->freqY2IdxCaches = (char *) mymalloc(_this->freqY2IdxCacheCount);
    for (i = minIdx; i <= maxIdx; i++) {
        _this->freqY2IdxCaches[i - _this->freqY2IdxCacheMinIdx] = getIdxFromFrequency(_this, i);
    }

    far_setRange(&_this->ampsRange, minIdx, maxIdx);
    bs_reset2(&_this->blockSearcher, _this->header);
}

void sa_analyFFTAmplitude(struct SignalAnalyser *_this, float amplitudes[], struct EventInfo *_resultEvent) {
}

bool sa_analyFFTAmplitude2FrequencyInfo(struct SignalAnalyser *_this, struct EventInfo *_ei) {
    return true;
}

#ifdef AA_INLINE
inline tidx getRangeStart(int _rangeIdx, float _matchCount, int _idxOff) {

    return _rangeIdx * _matchCount + _idxOff + 0.5;
}

inline tidx getRangeEnd(int _rangeIdx, float _matchCount, int _idxOff) {

    return ((int) (_rangeIdx * _matchCount + _idxOff + 0.5)) + RangeMatchCount;
}
#else

 tidx getRangeStart(int _rangeIdx, float _matchCount, int _idxOff) {

    return _rangeIdx * _matchCount + _idxOff + 0.5;
}

 tidx getRangeEnd(int _rangeIdx, float _matchCount, int _idxOff) {

    return ((int) (_rangeIdx * _matchCount + _idxOff + 0.5)) + RangeMatchCount;
}
#endif

int
getFrequencyInfoEventDeviation(int _freqY, tidx _eventStart, tidx _eventEnd, float _matchCount, int _signalEventIdxOff,
                               bool _printLog) {

    float eventMiddle = ((float) (_eventStart + _eventEnd + 1)) / 2;
    tidx rangeIdx = (tidx) ((eventMiddle - _signalEventIdxOff) / _matchCount);
    int absMinus;

    tidx fiRangeStart = getRangeStart(rangeIdx, _matchCount, _signalEventIdxOff), fiRangeEnd = getRangeEnd(rangeIdx,
                                                                                                           _matchCount,
                                                                                                           _signalEventIdxOff);
    if (_printLog)
        printf("range %d (%d, %d) cal diviation (%d, %d)\n", rangeIdx, fiRangeStart, fiRangeEnd, _eventStart,
               _eventEnd);

    if ((fiRangeStart - _eventStart >= 0 && fiRangeEnd - _eventEnd <= 0) ||
        (fiRangeStart - _eventStart <= 0 && fiRangeEnd - _eventEnd >= 0)) {
        absMinus = 0;
    } else
        absMinus = abs(fiRangeStart - _eventStart) + abs(fiRangeEnd - _eventEnd);

    absMinus = ((absMinus != 0) ? (0 + (absMinus <= 2 ? (absMinus * 2) : (absMinus * 5))) : 0);
    printEventsIdxOff("freq %d(%d, %d) match (%d, %d) deviation %d of eventoff %d\n", _freqY, _eventStart, _eventEnd,
                      fiRangeStart, fiRangeEnd, absMinus, _signalEventIdxOff);
    return absMinus;
}

struct FreqAmplitude *matchEventAmplitude(struct EventInfo *_event, int _freqY, float _minYDistance) {
    if (_event != NULL) {
        if (_event->p1.frequencyY != 0) {
            if (abs(_event->p1.frequencyY - _freqY) <= _minYDistance) {
                return &_event->p1;
            } else if (_event->p2.frequencyY != 0) {
                if (abs(_event->p2.frequencyY - _freqY) <= _minYDistance) {
                    return &_event->p2;
                } else if (_event->p3.frequencyY != 0) {
                    if (abs(_event->p3.frequencyY - _freqY) <= _minYDistance) {
                        return &_event->p3;
                    }
                }
            }
        }
    }

    return NULL;
}

void matchSignalEvents(struct SignalAnalyser *_sa, int _freqY, tidx _startEvent,
                       struct EventInfo **_resultStartEvent, struct EventInfo **_resultEndEvent) {
    int eventCount = 1;
    struct EventInfo *leftEvent = NULL, *rightEvent = NULL;
    struct FreqAmplitude *leftAmp = NULL, *rightAmp = NULL;
    tidx leftEventIdx = _startEvent, rightEventIdx = _startEvent;
    *_resultStartEvent = *_resultEndEvent = iei_get(&_sa->idx2Times, _startEvent);
    while (eventCount < maxMainMatchCount) {
        if (leftEvent == NULL) {
            leftEventIdx--;
            leftEvent = iei_get(&_sa->idx2Times, leftEventIdx);
        }
        if (rightEvent == NULL) {
            rightEventIdx++;
            rightEvent = iei_get(&_sa->idx2Times, rightEventIdx);
        }
        leftAmp = matchEventAmplitude(leftEvent, _freqY, _sa->maxFreqYDistance);
        rightAmp = matchEventAmplitude(rightEvent, _freqY, _sa->maxFreqYDistance);

        if (leftAmp == NULL) {
            if (rightAmp == NULL) {
                return;
            } else {
                *_resultEndEvent = rightEvent;
                rightEvent = NULL;
            }
        } else {
            if (rightAmp == NULL) {
                *_resultStartEvent = leftEvent;
                leftEvent = NULL;
            } else {
                if (leftAmp->amplitude > rightAmp->amplitude) {
                    *_resultStartEvent = leftEvent;
                    leftEvent = NULL;
                } else {
                    *_resultEndEvent = rightEvent;
                    rightEvent = NULL;
                }
            }
        }
        eventCount++;
    }
}

int sa_analyEventsIdxOff(struct SignalAnalyser *_this, tidx _startEvent, tidx _endEvent, tidx _maxEndEvent,
                         float _matchCount) {
    int i, j, sameCount = 0, spanSameCount = 0;
    int sumSignalDeviations[searchMinRangeCount];
    int minDeviationIdx = -1;
    struct EventInfo *signalStartEvent = NULL, *signalEndEvent = NULL, *siganlMaxEvent = NULL;
    struct EventInfo *preEvent = NULL, *prepreEvent = NULL, *currEvent = NULL;
    bool isEqual = false;
    printEventsIdxOff("\nanalyse events idx off from %d to %d on matchCount:%f\n", _startEvent, _endEvent, _matchCount);
    memset(sumSignalDeviations, 0, sizeof(sumSignalDeviations));
    for (i = _startEvent; i <= _maxEndEvent + 1; i++) {
        currEvent = iei_get(&_this->idx2Times, i);
        if (preEvent == NULL || preEvent->p1.frequencyY == 0 || currEvent == NULL || currEvent->p1.frequencyY == 0) {
            isEqual = false;
        } else {
            isEqual = abs(currEvent->p1.frequencyY - preEvent->p1.frequencyY) <= _this->maxFreqYDistance;
        }
        if (isEqual) {
            if (sameCount == 0) {
                signalStartEvent = currEvent;
                siganlMaxEvent = currEvent;
            } else {
                if (currEvent->p1.amplitude > siganlMaxEvent->p1.amplitude) {
                    siganlMaxEvent = currEvent;
                }
            }
            sameCount++;
            spanSameCount = 0;
        } else {
            if (sameCount + 1 >= MIN_SIGNAL_BIG_MATCH) {
                int eventDeviation;

                signalEndEvent = NULL;
                signalStartEvent = NULL;
                matchSignalEvents(_this, siganlMaxEvent->p1.frequencyY, siganlMaxEvent->timeIdx, &signalStartEvent,
                                  &signalEndEvent);
                for (j = 0; j < searchMinRangeCount; j++) {
                    eventDeviation = getFrequencyInfoEventDeviation(siganlMaxEvent->p1.frequencyY,
                                                                    signalStartEvent->timeIdx, signalEndEvent->timeIdx,
                                                                    _matchCount, j, false);
                    sumSignalDeviations[j] += eventDeviation;
                }
                siganlMaxEvent = NULL;
            } else if (spanSameCount > 0) {
                bool curr2prepreEqual;
                if (prepreEvent == NULL || prepreEvent->p1.frequencyY == 0 || currEvent == NULL ||
                    currEvent->p1.frequencyY == 0) {
                    curr2prepreEqual = false;
                } else {
                    curr2prepreEqual =
                            abs(currEvent->p1.frequencyY - prepreEvent->p1.frequencyY) <= _this->maxFreqYDistance;
                }
                if (curr2prepreEqual) {
                    assert(abs(siganlMaxEvent->p1.frequencyY - currEvent->p1.frequencyY) <= _this->maxFreqYDistance);
                    assert(abs(signalStartEvent->p1.frequencyY - currEvent->p1.frequencyY) <= _this->maxFreqYDistance);
                    if (currEvent->p1.amplitude > siganlMaxEvent->p1.amplitude) {
                        siganlMaxEvent = currEvent;
                    }
                    sameCount = spanSameCount;
                    sameCount++;
                    spanSameCount = 0;
                } else {
                    spanSameCount = 0;
                }
            } else {
                spanSameCount = sameCount;
            }
            sameCount = 0;
        }
        prepreEvent = preEvent;
        preEvent = currEvent;

        if (i == _endEvent + 1 && _maxEndEvent > _endEvent) {
            minDeviationIdx = searchImin(sumSignalDeviations, sizeof(sumSignalDeviations) / sizeof(int));
            assert(minDeviationIdx >= 0);
            if (sumSignalDeviations[minDeviationIdx] < 15) {
                break;
            }
            minDeviationIdx = -1;
        }
    }

    if (minDeviationIdx < 0)
        minDeviationIdx = searchImin(sumSignalDeviations, sizeof(sumSignalDeviations) / sizeof(int));
    return minDeviationIdx;
}

void addEventAmpToFreqRanges(struct SignalAnalyser *_this, struct FIMatch *_freqRanges, struct EventInfo *_e,
                             struct FreqAmplitude *_amp, int _idx) {
    int idx = getIdxFromFrequencyFromCache(_this, _amp->frequencyY);

    if (idx >= 0 && idx < DEFAULT_CODE_FREQUENCY_SIZE && abs(_this->CODE_Y[idx] - _amp->frequencyY) <= 1) {

        if (_amp->peakType == 1)_freqRanges[idx].bigMatchCount++; else _freqRanges[idx].orderMatchCount++;
        if (_amp->order == 3) _freqRanges[idx].tinyMatchCount++;
        if (_amp->order == 1)_freqRanges[idx].headMatchCount++;
        _freqRanges[idx].sumAmplitude += _amp->amplitude;
        if (_freqRanges[idx].firstMatch.event == NULL) {
            _freqRanges[idx].firstMatch.event = _e;
            _freqRanges[idx].firstMatch.frequency = _amp;
        }
        if (_amp->amplitude > _freqRanges[idx].topAmplitude) {
            _freqRanges[idx].topAmplitude = _amp->amplitude;
            _freqRanges[idx].topAmplitudeIdx = _idx;
        }
        _freqRanges[idx].sumQuality += (_idx + 1) * _amp->amplitude;
    }
}

void
getTimeRangeFrequencyPeak(struct SignalAnalyser *_this, tidx _timeStart, tidx _timeEnd, struct FIMatch *_freqRanges) {
    tidx i, idx;
    struct EventInfo *e;
    mymemset(_freqRanges, 0, sizeof(struct FIMatch) * DEFAULT_CODE_FREQUENCY_SIZE);
    for (i = _timeStart; i <= _timeEnd; i++) {
        e = iei_get(&_this->idx2Times, i);
        if (e->p1.frequencyY != 0)addEventAmpToFreqRanges(_this, _freqRanges, e, &e->p1, i - _timeStart);
        if (e->p2.frequencyY != 0)addEventAmpToFreqRanges(_this, _freqRanges, e, &e->p2, i - _timeStart);
        if (e->p3.frequencyY != 0)addEventAmpToFreqRanges(_this, _freqRanges, e, &e->p3, i - _timeStart);
    }
}

float getAroundAmplitudeDis(struct SignalAnalyser *_this, int _freqY, tidx _startEventIdx, int _step) {
    int i;
    struct EventInfo *e;
    float aroundSumAmplitude = 0;
    int off = 0;
    bool hasAmp = false;
    for (i = 0; i < preciseMatchCount / 2 + 1; i++) {
        e = iei_get(&_this->idx2Times, _startEventIdx + off);
        if (e != NULL) {
            if (e->p1.frequencyY != 0) {
                if (abs(e->p1.frequencyY - _freqY) <= _this->maxFreqYDistance) {
                    aroundSumAmplitude += e->p1.amplitude;
                    hasAmp = true;
                } else if (e->p2.frequencyY != 0) {
                    if (abs(e->p2.frequencyY - _freqY) <= _this->maxFreqYDistance) {
                        aroundSumAmplitude += e->p2.amplitude;
                        hasAmp = true;
                    } else if (e->p3.frequencyY != 0) {
                        if (abs(e->p3.frequencyY - _freqY) <= _this->maxFreqYDistance) {
                            aroundSumAmplitude += e->p3.amplitude;
                            hasAmp = true;
                        }
                    }
                }
            }
        }
        if (!hasAmp)break;
        off += _step;
    }
    return aroundSumAmplitude;
}

void
topNTimeRangeMatch(struct TimeRangeMatch *_matches, int _matchCount, int _rangeIdx, int _peak, struct FIMatch *_match) {
    int i;
    for (i = 0; i < _matchCount; i++) {
        if (_peak > _matches[i].peak) {

            int j;
            for (j = _matchCount - 1; j > i; j--) {
                _matches[j].peak = _matches[j - 1].peak;
                _matches[j].bigMatchCount = _matches[j - 1].bigMatchCount;
                _matches[j].smallMatchCount = _matches[j - 1].smallMatchCount;
                _matches[j].rangeIdx = _matches[j - 1].rangeIdx;
                _matches[j].sumAmplitude = _matches[j - 1].sumAmplitude;
                _matches[j].sumQuality = _matches[j - 1].sumQuality;
            }
            _matches[i].peak = _peak;
            _matches[i].bigMatchCount = _match->bigMatchCount;
            _matches[i].smallMatchCount = _match->orderMatchCount;
            _matches[i].rangeIdx = _rangeIdx;
            _matches[i].sumAmplitude = _match->sumAmplitude;
            _matches[i].sumQuality = _match->sumQuality;
            break;
        }
    }
}

#if defined(VOICE_BLOCK_ALIGN) && defined(TIME_MATCH_SIGNAL)

int sa_analyValidSignals3(struct SignalAnalyser *_this, int _eventStart, int _eventEnd, bool _isPreciseEnd,
                          bool _isFullBlock) {
    int i, l, rangeIdxStart, rangeIdxEnd;
    struct TimeRangeMatch *preMatchs = _this->preMatchs, *curMatchs = _this->curMatchs;
    int j, jPeak;

    int targetIdx = -1, preTargetIdx = _this->preTargetIdx, prepreTargetIdx = _this->prepreTargetIdx;
    int targetPeakType = 0, preTaretPeakType = _this->preTaretPeakType, prepreTaretPeakType = _this->prepreTaretPeakType;
    struct FrequencyInfo *tempFI, *startFI = NULL, *endFI = NULL;
    int signalIdx = 0, matchIdx;
    int blockSignalIdx = -1;
    float oneSignalLen = _this->oneSignalLen, targetAroundAmpRatio = 0;
    int blockSignalCount;
    bool isFullBlock = _isFullBlock;
    if (_this->freqRanges == NULL)
        _this->freqRanges = (struct FIMatch *) mymalloc(sizeof(struct FIMatch) * DEFAULT_CODE_FREQUENCY_SIZE);

    if (isFullBlock) {
        if ((_eventEnd - _eventStart) > ((VOICE_BLOCK_SIZE + VOICE_BLOCK_SIZE / 2) * preciseMatchCount)) {
            printf("%d match next block signal too long\n", _this->blockCount);
            return Recog_NotEnoughSignal;
        }
    }
    blockSignalIdx++;

    if (isFullBlock) {
        oneSignalLen = ((float) (_eventEnd - _eventStart)) / (VOICE_BLOCK_SIZE + 1);
        blockSignalCount = VOICE_BLOCK_SIZE + 1;
        _this->oneSignalLen = (oneSignalLen * _this->blockCount + _this->oneSignalLen) / (_this->blockCount + 1);
    } else {

        blockSignalCount = (_eventEnd - _eventStart) / oneSignalLen + 0.5;
        if (blockSignalCount >= 5) {
            oneSignalLen = ((float) (_eventEnd - _eventStart)) / blockSignalCount;
        }
    }

    if (oneSignalLen < preciseMatchCount)oneSignalLen = preciseMatchCount;

    {
        bool needAroundAmpTest = false;

        int blockBestIdxOff = sa_analyEventsIdxOff(_this, _eventStart,
                                                   (((_eventStart + 5 * preciseMatchCount) < _eventEnd) ? (_eventStart +
                                                                                                           5 *
                                                                                                           preciseMatchCount)
                                                                                                        : _eventEnd),
                                                   (((_eventStart + 10 * preciseMatchCount) < _eventEnd) ? (
                                                           _eventStart + 10 * preciseMatchCount) : _eventEnd),
                                                   oneSignalLen);
        float eventMiddle = ((float) (_eventStart + (maxBlockMatchCount - 1) / 2));
        tidx rangeIdx = (tidx) (((float) ((eventMiddle - blockBestIdxOff) / oneSignalLen)));
        int k = -1, ki;
        struct TimeRangeSignal currSignalIdx;
        struct TimeRangeSignal signalIdxInPreMatches, preSignalIdxInPreMatches;
        int similarCount = 0, preSimilarCount = 0;
        float curr2pre;
        int freqPeakType = 0;

        int errCount = 0;
        int matchDeviation = unknownMatchDeviation, preMatchDeviation = 0;

        if (_this->blockCount == 0) {
            preTaretPeakType = 1;
            preTargetIdx = DEFAULT_HEADER_TOKEN;
            preMatchs[0].peak = 1000;
            preMatchs[0].rangeIdx = DEFAULT_HEADER_TOKEN;
            preMatchs[0].sumAmplitude = 1000;
        }

        if (_this->blockCount >= MAX_BLOCK_COUNT) {
            return Recog_TooMuchSignal;
        }
        _this->blocks[_this->blockCount].startIdx = vector_size(&_this->similarSignals);
        _this->blocks[_this->blockCount].signalCount = 0;
        trs_reset(&preSignalIdxInPreMatches);
        for (i = 0; i < blockSignalCount; i++) {
            rangeIdxStart = getRangeStart(i + rangeIdx, oneSignalLen, blockBestIdxOff);
            rangeIdxEnd = getRangeEnd(i + rangeIdx, oneSignalLen, blockBestIdxOff);
            if (rangeIdxEnd > iei_idx(&_this->idx2Times))break;
            getTimeRangeFrequencyPeak(_this, rangeIdxStart, rangeIdxEnd, _this->freqRanges);
            for (j = 0; j < MAX_FI_MATCH_COUNT; j++) {
                curMatchs[j].rangeIdx = -1;
                curMatchs[j].peak = 0;
                curMatchs[j].preMatchIdx = -1;
            }

            for (j = 0; j < DEFAULT_CODE_FREQUENCY_SIZE; j++) {
                jPeak = _this->freqRanges[j].bigMatchCount * 10 + _this->freqRanges[j].headMatchCount * 2 +
                        (_this->freqRanges[j].orderMatchCount - _this->freqRanges[j].tinyMatchCount) * 6 +
                        _this->freqRanges[j].tinyMatchCount * 3;

                if (jPeak > 0)topNTimeRangeMatch(curMatchs, MAX_FI_MATCH_COUNT, j, jPeak, _this->freqRanges + j);
            }

            trs_reset(&currSignalIdx);
            trs_reset(&signalIdxInPreMatches);
            trs_setMaybeError(&currSignalIdx, false);
            similarCount = 0;
            targetIdx = -1;
            targetPeakType = 0;
            matchDeviation = unknownMatchDeviation;

            for (j = 0; j < MAX_FI_MATCH_COUNT; j++) {
                if (curMatchs[j].rangeIdx < 0)break;
                needAroundAmpTest = false;
                k = -1;
                for (ki = 0; ki < MAX_FI_MATCH_COUNT; ki++) {
                    if (preMatchs[ki].rangeIdx < 0)break;
                    if (preMatchs[ki].rangeIdx == curMatchs[j].rangeIdx) {
                        k = ki;
                        break;
                    }
                }

                curr2pre = 100;
                freqPeakType = 0;
                curMatchs[j].preMatchIdx = k;
                if (k >= 0) {
                    curr2pre = curMatchs[j].sumAmplitude / preMatchs[k].sumAmplitude;
                }

                if (curr2pre > 2) {
                    if (targetPeakType == 0) {
                        freqPeakType = 1;

                        if (curMatchs[j].sumQuality / curMatchs[j].sumAmplitude >=
                            (rangeIdxEnd - rangeIdxStart + 1) * 0.75) {
                            freqPeakType = 2;
                        }
                    } else {
                        freqPeakType = 2;

#ifndef PRINT_TIME_MATCH

#endif
                    }

                } else if (curr2pre < 0.5) {

                    if (curMatchs[j].rangeIdx == DEFAULT_START_TOKEN
                        && (i == 0 || i == 1)
                        && blockSignalIdx == 0

                        && ((j == 0)
                            || curMatchs[j].bigMatchCount + curMatchs[j].smallMatchCount >=
                               (rangeIdxEnd - rangeIdxStart + 1) / 2
                        )
                        && preTargetIdx == DEFAULT_START_TOKEN) {
                        freqPeakType = 2;
                    } else freqPeakType = 3;
                } else if (curr2pre < 1) {
                    if ((targetIdx >= 0 && targetPeakType == 1) || (similarCount >= 2)) {

                        {
                            freqPeakType = 3;
                        }
                    }
                    if (freqPeakType == 0) {
                        freqPeakType = 2;
                    }
                } else {

                    if (curMatchs[j].rangeIdx == prepreTargetIdx && curMatchs[j].bigMatchCount >= 3 && curr2pre > 1.5 &&
                        preMatchs[k].sumQuality / preMatchs[k].sumAmplitude >= (rangeIdxEnd - rangeIdxStart)) {
                        freqPeakType = 1;
                    }

                    if (freqPeakType == 0)freqPeakType = 2;
                }

                if (freqPeakType == 1 || freqPeakType == 2) {
                    if (curMatchs[j].rangeIdx == preTargetIdx) {
                        assert(i == 0 || (k >= 0 && preMatchs[k].rangeIdx == curMatchs[j].rangeIdx));
                        if (preTaretPeakType == 1 && preMatchs[k].peak > curMatchs[j].peak) {
                            freqPeakType = 3;
                        }

                    } else if (curMatchs[j].rangeIdx == prepreTargetIdx &&
                               curMatchs[j].rangeIdx != DEFAULT_OVERLAP_TOKEN) {
                        if (prepreTaretPeakType == 1) {
                            if (curr2pre < 1) {
                                freqPeakType = 3;
                            } else {
                                freqPeakType = 2;
                            }
                        }
                    }
                }

                if (freqPeakType == 2
                    && (!(targetIdx == DEFAULT_START_TOKEN && preTargetIdx == DEFAULT_START_TOKEN))
                    && (curMatchs[j].bigMatchCount + curMatchs[j].smallMatchCount) <
                       (rangeIdxEnd - rangeIdxStart + 1) / 2) {
                    freqPeakType = 3;
                }

                if (targetIdx == DEFAULT_START_TOKEN && preTargetIdx != DEFAULT_START_TOKEN &&
                    curMatchs[j].sumAmplitude / curMatchs[matchIdx].sumAmplitude < 0.2) {
                    freqPeakType = 3;
                }

                if (freqPeakType == 2 &&
                    curMatchs[j].sumQuality / curMatchs[j].sumAmplitude > (rangeIdxEnd - rangeIdxStart + 1) * 0.82) {
                    freqPeakType = 3;
                }

                if (freqPeakType == 1 && similarCount == 0 && curMatchs[j].rangeIdx == DEFAULT_START_TOKEN) {
                    freqPeakType = 2;
                }

                assert(freqPeakType > 0);
                if (freqPeakType == 1 || freqPeakType == 2) {
                    if (targetPeakType == 0)targetPeakType = ((freqPeakType == 1) ? 1 : 2);
                    if (targetIdx < 0
                        || (targetPeakType == 2)

                        || (targetPeakType == 1
                            && (freqPeakType == 1 || (freqPeakType == 2
                                                      && (curMatchs[j].bigMatchCount + curMatchs[j].smallMatchCount >
                                                          (rangeIdxEnd - rangeIdxStart + 1) * 0.8)
                                                      && (curMatchs[j].preMatchIdx<0 ||
                                                                                   preMatchs[curMatchs[j].preMatchIdx].sumQuality /
                                                                                   preMatchs[curMatchs[j].preMatchIdx].sumAmplitude>(
                            rangeIdxEnd - rangeIdxStart + 1) * 0.75))))
                           && similarCount < MAX_RANGE_IDX_COUNT) {

                        if (targetIdx >= 0
                            && curMatchs[j].bigMatchCount + curMatchs[j].smallMatchCount >
                               (rangeIdxEnd - rangeIdxStart + 1) * 0.8
                            && curMatchs[matchIdx].sumQuality / curMatchs[matchIdx].sumAmplitude >
                               (rangeIdxEnd - rangeIdxStart + 1) * 0.75
                            && preMatchs[curMatchs[j].preMatchIdx].sumQuality /
                               preMatchs[curMatchs[j].preMatchIdx].sumAmplitude >
                               (rangeIdxEnd - rangeIdxStart + 1) * 0.75) {
                            targetPeakType = 2;
                            matchIdx = j;
                            targetIdx = curMatchs[j].rangeIdx;
                            currSignalIdx.idxes[0] = curMatchs[j].rangeIdx;
                            signalIdxInPreMatches.idxes[0] = j;
                            similarCount = 1;

                        } else if ((targetIdx < 0 || targetPeakType == 2) && similarCount < MAX_RANGE_IDX_COUNT) {
                            currSignalIdx.idxes[similarCount] = curMatchs[j].rangeIdx;
                            signalIdxInPreMatches.idxes[similarCount] = j;
                            similarCount++;
                            if (similarCount > 1 && targetPeakType == 1)targetPeakType = 2;
                        }
                    }
                    if (targetIdx < 0) {
                        targetIdx = curMatchs[j].rangeIdx;
                        matchIdx = j;
                    }
                }

                if (similarCount >= MAX_RANGE_IDX_COUNT)break;
            }

            if (similarCount > 1 &&
                ((i > 0 && targetIdx == DEFAULT_START_TOKEN)
                 || (targetIdx == preTargetIdx || targetIdx == prepreTargetIdx))) {
                signed char tempSignalIdx = currSignalIdx.idxes[0];
                currSignalIdx.idxes[0] = currSignalIdx.idxes[1];
                currSignalIdx.idxes[1] = tempSignalIdx;
                tempSignalIdx = signalIdxInPreMatches.idxes[0];
                signalIdxInPreMatches.idxes[0] = signalIdxInPreMatches.idxes[1];
                signalIdxInPreMatches.idxes[1] = tempSignalIdx;
                targetIdx = currSignalIdx.idxes[0];
                matchIdx = signalIdxInPreMatches.idxes[0];
            }

            if ((i == 1 && targetIdx == DEFAULT_START_TOKEN && preTargetIdx == DEFAULT_START_TOKEN && similarCount <= 1)
                || (i == blockSignalCount - 1 && targetIdx == DEFAULT_START_TOKEN)) {
                int guessIdx = ((matchIdx == 0) ? 1 : 0);
                currSignalIdx.idxes[similarCount] = curMatchs[guessIdx].rangeIdx;
                if (currSignalIdx.idxes[similarCount] < 0)currSignalIdx.idxes[similarCount] = 1;
                signalIdxInPreMatches.idxes[similarCount] = guessIdx;
                similarCount++;
                trs_setMaybeError(&currSignalIdx, true);

            }

            if (targetIdx > 0 && curMatchs[matchIdx].preMatchIdx >= 0
                && ((targetPeakType == 1) || (targetPeakType == 2 && similarCount == 1))) {
                struct TimeRangeSignal *preRangeSignal = (struct TimeRangeSignal *) vector_last(&_this->similarSignals);
                assert(curMatchs[matchIdx].rangeIdx == targetIdx);
                if (preTaretPeakType == 2 && preSimilarCount > 1) {
                    int prex;
                    for (prex = 0; prex < MAX_RANGE_IDX_COUNT; prex++) {
                        if (preRangeSignal->idxes[prex] < 0)break;
                        if (preRangeSignal->idxes[prex] == targetIdx) {
                            assert(preSignalIdxInPreMatches.idxes[prex] >= 0);
                            if (preMatchs[preSignalIdxInPreMatches.idxes[prex]].sumQuality /
                                preMatchs[preSignalIdxInPreMatches.idxes[prex]].sumAmplitude >
                                (rangeIdxEnd - rangeIdxStart + 1) * 0.75) {

                                signed char oldIdx1 = preRangeSignal->idxes[prex];
                                preRangeSignal->idxes[prex] = -1;
                                if (MAX_RANGE_IDX_COUNT - prex - 1 > 0) {
                                    memmove(preRangeSignal->idxes + prex, preRangeSignal->idxes + prex + 1,
                                            MAX_RANGE_IDX_COUNT - prex - 1);
                                    preRangeSignal->idxes[MAX_RANGE_IDX_COUNT - 1] = -1;
                                }

                                if (preRangeSignal->idxes[0] == DEFAULT_START_TOKEN && preRangeSignal->idxes[1] == -1) {
                                    trs_setMaybeError(preRangeSignal, true);
                                    preRangeSignal->idxes[1] = oldIdx1;
                                }
                            } else {

                                if (prex < preSimilarCount - 1) {

                                    signed char tempChar = preRangeSignal->idxes[prex];
                                    preRangeSignal->idxes[prex] = preRangeSignal->idxes[preSimilarCount - 1];
                                    preRangeSignal->idxes[preSimilarCount - 1] = tempChar;
                                }
                            }

                            prex = preSimilarCount;
                            break;
                        }
                    }
                    assert(prex == preSimilarCount);
                }

                if (preTaretPeakType == 2 && preRangeSignal->idx1 == targetIdx) {
                    trs_setMaybeError(preRangeSignal, true);
                }
            }

            if (targetIdx == preTargetIdx && i > 1) {
                trs_setMaybeError(&currSignalIdx, true);
            }

            if (targetIdx >= 0) {
                struct EventInfo *signalStartEvent = NULL, *signalEndEvent = NULL;

                matchSignalEvents(_this, _this->CODE_Y[targetIdx],
                                  rangeIdxStart + _this->freqRanges[targetIdx].topAmplitudeIdx,
                                  &signalStartEvent, &signalEndEvent);
                if ((rangeIdxStart - signalStartEvent->timeIdx >= 0 && rangeIdxEnd - signalEndEvent->timeIdx <= 0)
                    || (rangeIdxStart - signalStartEvent->timeIdx <= 0 && rangeIdxEnd - signalEndEvent->timeIdx >= 0)) {
                    matchDeviation = 0;
                } else
                    matchDeviation = signalStartEvent->timeIdx - rangeIdxStart;

                if (matchDeviation > 1) {
                    if (matchDeviation > maxMatchDeviation)matchDeviation = maxMatchDeviation;
                    blockBestIdxOff += (matchDeviation - 1);
                } else if (matchDeviation < -1) {
                    if (matchDeviation < -maxMatchDeviation)matchDeviation = -maxMatchDeviation;
                    blockBestIdxOff -= (-matchDeviation - 1);
                } else if (matchDeviation != 0 && matchDeviation != unknownMatchDeviation
                           && preMatchDeviation != 0 && preMatchDeviation != unknownMatchDeviation) {
                    if (matchDeviation > 0 && preMatchDeviation > 0) {
                        blockBestIdxOff += 1;
                    } else if (matchDeviation < 0 && preMatchDeviation < 0) {
                        blockBestIdxOff -= 1;
                    }
                }
            }

            if (targetIdx == DEFAULT_START_TOKEN) {
                targetPeakType = 2;
            }

            if (targetIdx < 0) {
                assert(similarCount <= 0);
                matchIdx = ((curMatchs[0].rangeIdx == preTargetIdx) ? 1 : 0);
                trs_setMaybeError(&currSignalIdx, true);
                targetIdx = curMatchs[matchIdx].rangeIdx;
                currSignalIdx.idx1 = targetIdx;
                currSignalIdx.idx2 = curMatchs[((curMatchs[matchIdx + 1].rangeIdx == preTargetIdx) ? (matchIdx + 2) : (
                        matchIdx + 1))].rangeIdx;

                if (++errCount > RS_CORRECT_SIZE) {
                    printf("too many time range can not match signal\n");
                    return Recog_NotEnoughSignal;
                }

                if (targetIdx < 0 || (targetIdx == 0 && i == blockSignalCount - 1)) {

                    targetIdx = -1;
                    currSignalIdx.idx1 = ((i == blockSignalCount - 1) ? 1 : 0);
                }
                similarCount = ((currSignalIdx.idx2 >= 0) ? 2 : 1);
            }
#ifdef MARK_MORE_MAYBE_ERROR
            else {
                if (curMatchs[matchIdx].peak <= 30) {
                    trs_setMaybeError(&currSignalIdx, true);
                }
            }
#endif

            assert(similarCount > 0 || trs_maybeError(&currSignalIdx));
            vector_push_back(&_this->similarSignals, &currSignalIdx);
            prepreTargetIdx = preTargetIdx;
            prepreTaretPeakType = preTaretPeakType;
            preTargetIdx = targetIdx;
            preTaretPeakType = targetPeakType;
            preMatchDeviation = matchDeviation;
            preSimilarCount = similarCount;
            preSignalIdxInPreMatches = signalIdxInPreMatches;
            memcpy(_this->preMatchs, _this->curMatchs, sizeof(_this->preMatchs));
        }
        _this->blocks[_this->blockCount].signalCount =
                vector_size(&_this->similarSignals) - _this->blocks[_this->blockCount].startIdx;
        assert(isFullBlock ? _this->blocks[_this->blockCount].signalCount >= VOICE_BLOCK_SIZE : true);
        _this->blockCount++;
    }

    _this->prepreTargetIdx = prepreTargetIdx;
    _this->preTargetIdx = preTargetIdx;
    _this->preTaretPeakType = targetPeakType;
    _this->prepreTaretPeakType = prepreTaretPeakType;

    return Recog_Success;
}

#endif

void sa_analyFFTSignal(struct SignalAnalyser *_this) {
    int recogStatus = Recog_Success;
    int i, c;
    struct FrequencyInfo *fi;
    int signalStart = -1, signalEnd = -1;
    struct FrequencyInfo **fiValidSignals;
    tidx lastTimeIdx;
    tidx firstTimeIdx;
    int totalEvent;
    struct EventInfo *realLastTime;
    struct EventInfo *realFirstTime;
    float startSignalAmpLevel;
    int blockCount = 0;

#define MAX_CHARS 100
    char chars[MAX_CHARS + 1];
    int indexs[MAX_CHARS + 1];
    int indexLen = 0;
    tidx lastSignalIdx = 0;

    if ((int) vector_size(&_this->similarSignals) >= _this->minRegcoSignalCount && _this->blockCount > 0) {
        if (recogStatus == Recog_Success) {
            if (_this->listener != NULL && recogStatus == Recog_Success) {
                float soundTime = (_this->overlap + ((float) (lastSignalIdx)) * (_this->fftSize - _this->overlap)) /
                                  _this->sampleRate;
                bool thisTimeOK = false;
#ifdef SEARCH_SIMILAR_SIGNAL
                thisTimeOK = _this->listener->onStopRecognition2(_this->listener, soundTime, recogStatus,
                                                                 vector_nativep(&_this->similarSignals),
                                                                 vector_size(&_this->similarSignals), _this->blocks,
                                                                 _this->blockCount);
#else
                thisTimeOK = _this->listener->onStopRecognition(_this->listener, soundTime, recogStatus, indexs, indexLen);
#endif
                if (!_this->signalOK)_this->signalOK = thisTimeOK;
            }
        }
        _this->totalSignalCount++;
    } else {
        recogStatus = Recog_NotEnoughSignal;
    }
    if (_this->listener != NULL && recogStatus != Recog_Success) {
        if (_this->recoging) {
            float soundTime =
                    (_this->overlap + ((float) (iei_idx(&_this->idx2Times) - 1)) * (_this->fftSize - _this->overlap)) /
                    _this->sampleRate;
            _this->listener->onStopRecognition(_this->listener, soundTime, recogStatus, NULL, 0);
        }
    }
    _this->recoging = false;
    _this->blockCount = 0;
#ifdef SEARCH_SIMILAR_SIGNAL
    vector_clear(&_this->similarSignals);
#endif
#ifdef MATCH_FI
    pa_freeAllObjs(&_this->freqInfoAlloc);
#endif
}

int analyFFTCount = 0;

void sa_analyFFT(struct SignalAnalyser *_this, float amplitudes[]) {
    unsigned long startTime = getTickCount2();
    unsigned long analyStartTime = startTime;
    unsigned long freqStartTime = startTime;
    unsigned long analyFreqValidStartTime = freqStartTime;
    unsigned long now;
    unsigned long analyFreqTime;
    unsigned long analyFreqTop3Time;
    unsigned long analyFreqValidTime;
    bool finished = amplitudes == NULL;
    bool freqTruncate = false;
    struct EventInfo *buffedEvent = NULL;
    struct FrequencyInfo *blockHead = NULL, *blockTail = NULL;
    bool isEndBlock = false;
    tidx currEventIdx = 0;

    _this->preAnalyStartTime = startTime;
    if (_this->newSignal)_this->signalOK = false;
    _this->newSignal = finished;
    if (!finished) {
        far_resetAmps(&_this->ampsRange, amplitudes);
        initCodeY(_this, false);
        fats_onFFTAmpsRange(&_this->eventStat, &_this->ampsRange, _this->top2AvgIniter.minPeakAmplitudeTop2Avg,
                            _this->top2AvgIniter.minWeakPeakAmplitudeTop2Avg, _this->maxFreqYDistance);

        iei_add(&_this->idx2Times, &_this->eventStat.event);
        buffedEvent = iei_curr(&_this->idx2Times);
        currEventIdx = iei_idx(&_this->idx2Times);
        if (!_this->top2AvgIniter.inited)
            fari_onFFTAmpsRange(&_this->top2AvgIniter, &_this->ampsRange, &_this->idx2Times, &_this->eventStat);

        if (_this->eventStat.hasValidAmplitude
            && ((currEventIdx - _this->preValidFreqAmplitudeTime >= MAX_SIGNAL_SHORT_VALID_SIZE) ||
                _this->firstValidFreqAmplitudeTime < 0))
            _this->firstValidFreqAmplitudeTime = currEventIdx;

        assert(_this->eventStat.p1 != NULL);
        if (_this->rangeCurrAvgCount >= RANGE_AVG_SIZE) {
            assert(_this->rangeCurrAvgCount == RANGE_AVG_SIZE);
            _this->rangAvgSNR =
                    (_this->rangAvgSNR * (_this->rangeCurrAvgCount - 1) + _this->eventStat.amplitudeTop2Avg) /
                    _this->rangeCurrAvgCount;
        } else {
            _this->rangAvgSNR = (_this->rangAvgSNR * _this->rangeCurrAvgCount + _this->eventStat.amplitudeTop2Avg) /
                                (_this->rangeCurrAvgCount + 1);
            _this->rangeCurrAvgCount++;
        }
        if (_this->rangAvgSNR >= _this->top2AvgIniter.minRangePeakAmplitudeTop2Avg)
            _this->preValidRangeAvgSNRIdx = currEventIdx;
        if (_this->rangAvgSNR >= _this->top2AvgIniter.minLongRangePeakAmplitudeTop2Avg) {
            _this->preValidLongRangeAvgSNRIdx = currEventIdx;
        }

        analyFreqValidStartTime = getTickCount2();
        _this->analyseFreqTop3TotalTime += (analyFreqValidStartTime - freqStartTime);

        freqTruncate =
                ((_this->recoging && _this->firstRangeValidFreqAmplitudeTime < (currEventIdx - (MAX_EVENT_COUNT - 10)))
                 || vector_size(&_this->similarSignals) > 100
                 || ((_this->firstValidFreqAmplitudeTime < 0 ||
                      currEventIdx - _this->firstValidFreqAmplitudeTime <= MAX_SHORT_SIGNAL_SIZE) ? false :
                     (
                             (currEventIdx - _this->preValidFreqAmplitudeTime) >= MAX_SIGNAL_SHORT_VALID_SIZE
                             || (currEventIdx - _this->preValidRangeAvgSNRIdx) >= MAX_RNAGE_SIGNAL_LOW_SIZE
                             || (currEventIdx - _this->preValidLongRangeAvgSNRIdx >= MAX_LONG_RNAGE_SIGNAL_LOW_SIZE)
                     ))
                );

        if (freqTruncate)buffedEvent = NULL;
        if (_this->eventStat.hasValidAmplitude)
            _this->preValidFreqAmplitudeTime = currEventIdx;
    }

#ifdef MATCH_FI
    fis_onFFTEvent(&_this->fiSearcher, buffedEvent, _this->maxFreqYDistance);
#endif
    _this->blockSearcher.onFFTEvent(&_this->blockSearcher, buffedEvent);

    _this->finished = finished || freqTruncate;

    while (bs_hasBlock(&_this->blockSearcher, &blockHead, &blockTail, &isEndBlock)) {
        assert(blockHead->frequencyY > 0 && blockTail->frequencyY > 0);
        if (_this->recoging) {
            pinfo(TAG_SignalAnalyser, "before sa_analyValidSignals3\n");
            sa_analyValidSignals3(_this, fi_timesFirst(blockHead)->event->timeIdx,
                                  fi_timesFirst(blockTail)->event->timeIdx, true, !isEndBlock);
            pinfo(TAG_SignalAnalyser, "after sa_analyValidSignals3\n");
        }
    }

    now = getTickCount2();
    analyFreqTime = now - startTime;
    analyFreqTop3Time = analyFreqValidStartTime - freqStartTime;
    analyFreqValidTime = now - analyFreqValidStartTime;
    assert(analyFreqTop3Time + analyFreqValidTime == analyFreqTime);
    _this->analyFreqTotalTime += analyFreqTime;
    _this->analyseFreqValidTotalTime += analyFreqValidTime;
    assert(_this->analyseFreqTop3TotalTime + _this->analyseFreqValidTotalTime == _this->analyFreqTotalTime);
    startTime = now;

    if ((finished || freqTruncate) && _this->recoging) {
        assert(_this->firstRangeValidFreqAmplitudeTime >= (currEventIdx - (MAX_EVENT_COUNT - 10 + 1)));
        pinfo(TAG_SignalAnalyser, "freqTruncate(before onEndRecognition)\n");
        sa_analyFFTSignal(_this);
        pinfo(TAG_SignalAnalyser, "after onEndRecognition\n");
    }

    if (!finished && !_this->recoging && _this->firstValidFreqAmplitudeTime >= 0
        && (currEventIdx - _this->firstValidFreqAmplitudeTime) > MIN_START_SIGNAL_SIZE && !freqTruncate) {
        assert(_this->firstRangeValidFreqAmplitudeTime < _this->firstValidFreqAmplitudeTime);
        assert(!_this->recoging);
        pinfo(TAG_SignalAnalyser, "before onStartRecognition\n");

        _this->recoging = true;
        _this->firstRangeValidFreqAmplitudeTime = _this->firstValidFreqAmplitudeTime;
        if (_this->listener != NULL) {
            float soundTime = (_this->overlap + ((float) (_this->firstRangeValidFreqAmplitudeTime)) *
                                                (_this->fftSize - _this->overlap)) / _this->sampleRate;
            _this->listener->onStartRecognition(_this->listener, soundTime);
            _this->oneSignalLen = preciseMatchCount;
            _this->preTargetIdx = DEFAULT_HEADER_TOKEN;
            _this->blockCount = 0;
            _this->prepreTargetIdx = -1;
            _this->preTaretPeakType = 0;
            _this->prepreTaretPeakType = 0;
            memset(_this->preMatchs, 0, sizeof(_this->preMatchs));
            memset(_this->curMatchs, 0, sizeof(_this->curMatchs));
        }
    }
    if (_this->finished) {
        _this->firstValidFreqAmplitudeTime = -1;
    }

    now = getTickCount2();
    _this->anyseSignalTotalTime += (now - startTime);
    _this->preAnalyEndTime = now;
    _this->preAnalyTotalTime = (now - analyStartTime);
}

#endif


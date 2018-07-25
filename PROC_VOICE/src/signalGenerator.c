
#include "../include/log.h"
#include "../include/common.h"
#include "../include/signalGenerator.h"

const char *TAG_SignalGenerator = "Generator";

static const int STATE_START = 1;
static const int STATE_STOP = 2;

struct SignalGenerator *slg_init2(struct SignalGenerator *_this, struct SignalGeneratorCallback *_callback, int _sampleRate, int _bits, int _bufferSize)
{
	_this->volume = 1;
	_this->mCallback = _callback;

	_this->mBufferSize = _bufferSize;
	_this->mSampleRate = _sampleRate;
	_this->mBits = _bits;
	_this->mDuration = 0;

	_this->mFilledSize = 0;
	_this->mState = STATE_STOP;
	_this->mListener = NULL;

	return _this;
}

void slg_setVolume(struct SignalGenerator *_this, double _volume)
{
	_this->volume = _volume;
}

void slg_setListener(struct SignalGenerator *_this, struct SignalGeneratorListener *listener)
{
	_this->mListener = listener;
}

void slg_stop(struct SignalGenerator *_this)
{
	if (STATE_START == _this->mState)
	{
		_this->mState = STATE_STOP;
	}
}

void slg_start(struct SignalGenerator *_this)
{
	if (STATE_STOP == _this->mState)
	{
		_this->mState = STATE_START;
	}
}

int genBufferDebugIdx = 0;
int genDataLen = 0;

typedef float tgenDouble;

/**
 *
 * @param _this
 * @param _genRates:声音的频率，单位Hz
 * @param _volumes:
 * @param _genCount
 * @param duration:持续的时间
 */
void slg_gen(struct SignalGenerator *_this, int *_genRates, double *_volumes, int _genCount, int duration)
{
	#define MAX_GEN_COUNT MULTI_TONE
	int n[MAX_GEN_COUNT], totalCount;
	tgenDouble per[MAX_GEN_COUNT], d[MAX_GEN_COUNT];
	int lowTotalCount, lowDistance[MAX_GEN_COUNT];
	struct BufferData *buffer;
	int i, genIdx, out, lowN[MAX_GEN_COUNT];

	assert(_genCount <= MAX_GEN_COUNT);
	if (STATE_START == _this->mState)
	{
		_this->mDuration = duration;

		if (0 != _this->mListener)
		{
			_this->mListener->onStartGen(_this->mListener);
		}

		//？某段时间内采样的数据分成1000份？例如：采样频率为44100，2秒内采样数据就是44100×2
		//再分成1000份就是44100*2/1000约等于88个数据
		totalCount = (_this->mDuration * _this->mSampleRate) / 1000;
		lowTotalCount = (int)(totalCount * 0.4);
		for (genIdx = 0; genIdx < _genCount; genIdx ++)
		{
			//
			n[genIdx] = (int)(_this->mBits * _this->volume * _volumes[genIdx]);
			//_genRates:设定的声音频率,mSampleRate:采样频率
			per[genIdx] = (_genRates[genIdx] / (tgenDouble)(_this->mSampleRate)) * 2 * M_PI;
			d[genIdx] = 0;
			lowDistance[genIdx] = n[genIdx]/(lowTotalCount+1);
		}

		if (0 != _this->mCallback)
		{
			_this->mFilledSize = 0;
			buffer = _this->mCallback->getGenBuffer(_this->mCallback);
			if (0 != buffer)
			{
				//分配空间
				checkPointer(bd_getData(buffer));
				for (i = 0; i < totalCount; ++i)
				{
					if (STATE_START == _this->mState)
					{

						out = 0;
						for (genIdx = 0; genIdx < _genCount; genIdx ++)
						{
							//sqrt:平方根，pow(x,y)求x的y次方
							lowN[genIdx] = n[genIdx] * sqrt( 1.0 - (pow(i - (totalCount / 2), 2)
								/ pow((totalCount / 2), 2)));

							//sin:正弦
							//赋值
							out += (int)(sin(d[genIdx]) * lowN[genIdx]);
						}					

						if (_this->mFilledSize >= _this->mBufferSize - 1)
						{
							assert(_this->mFilledSize <= _this->mBufferSize);
							
							bd_setFilledSize(buffer, _this->mFilledSize);
							genDataLen += _this->mFilledSize;
							buffer->debugIdx = genBufferDebugIdx++;
							_this->mCallback->freeGenBuffer(_this->mCallback, buffer);

							_this->mFilledSize = 0;
							buffer = _this->mCallback->getGenBuffer(_this->mCallback);
							if (0 == buffer)
							{
								error(TAG_SignalGenerator, "get null buffer");
								break;
							}
							checkPointer(bd_getData(buffer));
						}

						bd_getData(buffer)[_this->mFilledSize++] = (char)(out & 0xff);
						if (BITS_16 == _this->mBits)
						{
							bd_getData(buffer)[_this->mFilledSize++] = (char)((out >> 8) & 0xff);
						}

						for (genIdx = 0; genIdx < _genCount; genIdx ++)
						{
							d[genIdx] += per[genIdx];
						}
					}
					else
					{
						debug(TAG_SignalGenerator, "signal gen stop");
						break;
					}
				}
			}
			else
			{
				error(TAG_SignalGenerator, "get null buffer");
			}

			if (0 != buffer)
			{
				assert(_this->mFilledSize <= _this->mBufferSize);
				bd_setFilledSize(buffer, _this->mFilledSize);
				genDataLen += _this->mFilledSize;
				buffer->debugIdx = genBufferDebugIdx++;
				_this->mCallback->freeGenBuffer(_this->mCallback, buffer);
			}
			_this->mFilledSize = 0;

			if (0 != _this->mListener)
			{
				_this->mListener->onStopGen(_this->mListener);
			}
		}
	}
}


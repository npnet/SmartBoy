
#include "../include/encoder.h"
#include "../include/log.h"
#include "../include/util.h"

const char *TAG_Encoder = "struct Encoder";

struct Encoder *enc_init(struct Encoder *_this, struct EncoderBufferSource *_callback, int _sampleRate, int _bits, int _bufferSize)
{
	_this->CODE_FREQUENCY = DEFAULT_CODE_FREQUENCY;
	_this->mCallback = _callback;
	_this->mListener = NULL;
	_this->mState = STATE_STOPED;
	_this->signalGenCallback.freeGenBuffer = enc_freeGenBuffer;
	_this->signalGenCallback.getGenBuffer = enc_getGenBuffer;
	slg_init2(&_this->mSignalGenerator, (struct SignalGeneratorCallback *)_this, _sampleRate, _bits, _bufferSize);

	return _this;
}

void enc_finalize(struct Encoder *_this)
{
}

void enc_setFreqs(struct Encoder *_this, int *_freqs)
{
	_this->CODE_FREQUENCY = _freqs;
}

void enc_setListener(struct Encoder *_this, struct EncoderListener *listener)
{
	_this->mListener = listener;
}

int enc_getMaxCodeCount(struct Encoder *_this)
{
	return DEFAULT_CODE_BOOK_SIZE;
}

bool enc_isStoped(struct Encoder *_this)
{
	return (STATE_STOPED == _this->mState);
}

void enc_setVolume(struct Encoder *_this, double _volume)
{
	slg_setVolume(&_this->mSignalGenerator, _volume);
}

/**
 * 声音编码
 * @param _this
 * @param codes
 * @param _blockDuration
 * @param duration
 * @param _volume
 */
void enc_encode(struct Encoder *_this, struct myvector *codes, int _blockDuration, int duration, float _volume)
{
	enc_encode2(_this, codes, _this->CODE_FREQUENCY, _blockDuration, duration, 0, _volume, false);
}

void enc_encode2(struct Encoder *_this, struct myvector *codes, int *_frequencies, int _blockDuration, int duration, int muteInterval, float _volume, bool _dualTone)
{
	int codeSize = vector_size(codes);
	if(codeSize > 0)
	{
		enc_encode3(_this, codes, true, _frequencies, _blockDuration, duration, muteInterval, _volume, _dualTone);
	}
}

void enc_encode3(struct Encoder *_this, struct myvector *_freqs, bool _isFreqIdx, int *_frequencies, 
	int _blockDuration, int duration, int muteInterval, float _volume, bool _dualTone)
{
	#define MAX_ENCODE_COUNT MULTI_TONE
	int codeSize, i, j;
	int *pfreqs = NULL;
	double volumes[MAX_ENCODE_COUNT+1];
	int genRates[MAX_ENCODE_COUNT+1];
	asize3_1(volumes, MAX_ENCODE_COUNT+1, genRates, MAX_ENCODE_COUNT+1, _frequencies, DEFAULT_CODE_FREQUENCY_SIZE, "pfreqs", sizeof(int));

	if (STATE_STOPED == _this->mState)
	{
		_this->mState = STATE_ENCODING;

		if (0 != _this->mListener)
		{
			pinfo(TAG_Encoder, "onStartEncode\n");
			_this->mListener->onStartEncode(_this->mListener);
		}

		slg_start(&_this->mSignalGenerator);		
		codeSize = vector_size(_freqs);
		if(codeSize > 0)
		{
			pfreqs = (int *)vector_nativep(_freqs); 
			areset(pfreqs, vector_size(_freqs));
#ifdef MUTE_START_MS
			adata(volumes, 0) = 0;
			adata(genRates, 0) = 0;

			slg_gen(&_this->mSignalGenerator, genRates, volumes, 1, MUTE_START_MS);
#endif
			for (i = 0; i < codeSize; i ++)
			{
				if (STATE_ENCODING == _this->mState)
				{
					if(!_dualTone)
					{
#ifdef FREQ_ANALYSE_MULTI_4TONE
						if (adata(pfreqs, i) != DEFAULT_START_TOKEN)
						{
							
							for (j = 0; j < MAX_ENCODE_COUNT; j ++)
							{
								adata(genRates, j) = adata(_frequencies, (((adata(pfreqs, i)-1) >> j) & 0x1) + j * 2 + 1);
								adata(volumes, j) = ((double)_volume)/MAX_ENCODE_COUNT;
							}
						}
						else
						{
							assert(i > 0);
							
							adata(genRates, 0) = adata(_frequencies, DEFAULT_START_TOKEN);
						}
						slg_gen(&_this->mSignalGenerator, genRates, volumes, MAX_ENCODE_COUNT, duration);
#else
						adata(volumes, 0) = _volume;
						adata(genRates, 0) = (_isFreqIdx?adata(_frequencies, adata(pfreqs, i)):adata(pfreqs, i));
						slg_gen(&_this->mSignalGenerator, genRates, volumes, 1, 
							((_isFreqIdx&&adata(pfreqs, i)==DEFAULT_START_TOKEN)?_blockDuration:duration));
#endif
					}
					else
					{

						adata(volumes, 0) = adata(volumes, 1) = _volume/2;
						adata(genRates, 0) = (_isFreqIdx?adata(_frequencies, adata(pfreqs, i)):adata(pfreqs, i));
						adata(genRates, 1) = (_isFreqIdx?adata(_frequencies, adata(pfreqs, i+1)):adata(pfreqs, i+1));
						slg_gen(&_this->mSignalGenerator, genRates, volumes, 2, duration);
						i ++;
					}					
				}
				else
				{
					debug(TAG_Encoder, "encode force stop");
					break;
				}
			}
#ifdef MUTE_END_MS
			adata(volumes, 0) = 0;
			adata(genRates, 0) = 0;
			slg_gen(&_this->mSignalGenerator, genRates, volumes, 1, MUTE_END_MS);
#endif
		}
		
		if (STATE_ENCODING == _this->mState)
		{
			if (muteInterval > 0)
			{
				adata(volumes, 0) = 0;
				adata(genRates, 0) = 0;
				slg_gen(&_this->mSignalGenerator, genRates, volumes, 1, muteInterval);
			}
		}
		else
		{
			debug(TAG_Encoder, "encode force stop");
		}
		enc_stop(_this);

		if (0 != _this->mListener)
		{
			pinfo(TAG_Encoder, "onEndEncode\n");
			_this->mListener->onEndEncode(_this->mListener);
		}
	}
}

void enc_stop(struct Encoder *_this)
{
	if (STATE_ENCODING == _this->mState)
	{
		_this->mState = STATE_STOPED;

		slg_stop(&_this->mSignalGenerator);
	}
}

struct BufferData *enc_getGenBuffer(struct SignalGeneratorCallback *_this_)
{
	struct Encoder *_this = (struct Encoder *)_this_;
	if (0 != _this->mCallback)
	{
		return _this->mCallback->getEncodeBuffer(_this->mCallback);
	}
	return 0;
}

void enc_freeGenBuffer(struct SignalGeneratorCallback *_this_, struct BufferData *buffer)
{
	struct Encoder *_this = (struct Encoder *)_this_;
	if (0 != _this->mCallback)
	{
		_this->mCallback->freeEncodeBuffer(_this->mCallback, buffer);
	}
}

int *enc_getCodeFrequency(struct Encoder *_this)
{
	return _this->CODE_FREQUENCY;
}


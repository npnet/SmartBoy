
#include "../include/pcmPlayer.h"
#include "../include/log.h"

const char * TAG_PcmPlayer = "struct PcmPlayer";
static const int STATE_START = 1;
static const int STATE_STOP = 2;

struct PcmPlayer *pcmp_init(struct PcmPlayer *_this, struct AudioPlayer *_audioPlayer, 
	struct PcmPlayerDataSource *callback, int sampleRate, int channel, int format, int bufferSize)
{
	_this->player = NULL;
	_this->mCallback = callback;	
	_this->mState = STATE_STOP;
	_this->mPlayedLen = 0;
	_this->sampleRate = sampleRate;
	_this->channel = channel;
	_this->format = format;
	_this->bufferSize = bufferSize;
#ifdef MIX_WAVE
	wd_reset(&_this->mixWave);
	_this->mixWavePlayPos = 0;
	_this->mixWaveMutePos = 0;
	_this->mixWaveMuteInterval = 0;
	_this->mixWaveVolume = 0;
#endif
	pcmp_setAudioPlayer(_this, _audioPlayer);

	return _this;
}

void pcmp_finalize(struct PcmPlayer *_this)
{

#ifdef MIX_WAVE
	if (_this->mixWave.data != NULL)
	{
		myfree(_this->mixWave.data);
		_this->mixWave.data = NULL;
	}
#endif
}

void pcmp_setAudioPlayer(struct PcmPlayer *_this, struct AudioPlayer *_audioPlayer)
{
	_this->audioPlayer = _audioPlayer;
}

void pcmp_setListener(struct PcmPlayer *_this, struct PcmPlayerListener *listener)
{
	_this->mListener = listener;
}

void pcmp_setMixPlaySound(struct PcmPlayer *_this, const char *_filename, float _volume, int _muteInterval)
{
#ifdef MIX_WAVE
	if (_this->mixWave.data != NULL)
	{
		myfree(_this->mixWave.data);
		_this->mixWave.data = NULL;
	}
	wd_reset(&_this->mixWave);
	readWave(_filename, &_this->mixWave);
	printf("%s read pcm size:%d\n", _filename, _this->mixWave.size);
	_this->mixWaveVolume = _volume;
	_this->mixWaveMuteInterval = _muteInterval;
#endif
}

void pcmp_setMixPlaySound2(struct PcmPlayer *_this, char *_waveData, int _wavDataLen, float _volume, int _muteInterval)
{
#ifdef MIX_WAVE
	if (_this->mixWave.data != NULL)
	{
		myfree(_this->mixWave.data);
		_this->mixWave.data = NULL;
	}
	wd_reset(&_this->mixWave);
	_this->mixWave.data = _waveData;
	_this->mixWave.size = _wavDataLen;
	_this->mixWaveVolume = _volume;
	_this->mixWaveMuteInterval = _muteInterval;
#endif
}

int bufferGetDataLen = 0;
void pcmp_start(struct PcmPlayer *_this)
{
	int playBufferCount = 0;
	int playBytes = 0;
	struct BufferData *data;
	signed char *dataBuffer = NULL;
	int i;
	int len;
#ifdef MIX_WAVE
	int maixWaveVolume;
#endif

	assert(_this->player == NULL);
	_this->audioPlayer->createPlayer(_this->sampleRate, _this->channel, _this->format, _this->bufferSize, &_this->player);
	if (STATE_STOP == _this->mState && 0 != _this->player)
	{
		_this->mPlayedLen = 0;
#ifdef MIX_WAVE
		_this->mixWavePlayPos = 0;
		_this->mixWaveMutePos = 0;
		maixWaveVolume = _this->mixWaveVolume * 100;
		assert(maixWaveVolume >= 0 && maixWaveVolume <= 100);
#endif

		if (0 != _this->mCallback)
		{
			_this->mState = STATE_START;
			if (0 != _this->mListener)
			{
				_this->mListener->onPlayStart(_this->mListener);
			}
			playBufferCount = 0;
			playBytes = 0;
			while (STATE_START == _this->mState)
			{
				data = _this->mCallback->getPlayBuffer(_this->mCallback);
				if (0 != data)
				{
					dataBuffer = NULL;
					if (0 != (dataBuffer = (signed char *)bd_getData(data)))
					{
						int dataLen = bd_getFilledSize(data);
#ifdef MIX_WAVE
						signed char *mixWaveData = (signed char *)_this->mixWave.data;
						int mixWave = 0;
						if (_this->mixWave.data != NULL)
						{

							int frame;
							for (i = 0; i < dataLen; i++)
							{
								if (_this->mixWavePlayPos >= _this->mixWave.size)
								{
									_this->mixWavePlayPos = 0;
									_this->mixWaveMutePos = (_this->mixWaveMuteInterval * _this->sampleRate * (_this->format/8)) / 1000;										
								}
								if (_this->mixWaveMutePos > 0)
								{
									_this->mixWaveMutePos --;
									mixWave = 0;
								}
								else
								{
									mixWave = (int)(mixWaveData[_this->mixWavePlayPos++]);
								}
								                                    
								frame = dataBuffer[i];

								frame = (frame * (100 - maixWaveVolume) + mixWave * maixWaveVolume) / 100;
								
								dataBuffer[i] = (signed char)frame;
							}
						}
#endif

						if (0 == _this->mPlayedLen)
						{
							_this->audioPlayer->startPlay(_this->player);
							pinfo(TAG_PcmPlayer, "startPlay start writePlayer");
						}

						bufferGetDataLen += dataLen;
						len = _this->audioPlayer->writePlayer(_this->player,(char *) dataBuffer, dataLen);
						playBytes += len;
						_this->mPlayedLen += len;
						_this->mCallback->freePlayData(_this->mCallback, data);
					}
					else
					{
						
						debug(TAG_PcmPlayer, "it is the end of input, so need stop");
						break;
					}
				}
				else
				{
					error(TAG_PcmPlayer, "get null data");
					break;
				}
			}

			if (0 != _this->player)
			{
				_this->audioPlayer->flushPlayer(_this->player);
				_this->audioPlayer->stopPlay(_this->player);
			}
			_this->mState = STATE_STOP;
			if (0 != _this->mListener)
			{
				_this->mListener->onPlayStop(_this->mListener);
			}
		}

		_this->audioPlayer->releasePlayer(_this->player);
		_this->player = NULL;
	}
}

void pcmp_stop(struct PcmPlayer *_this)
{
	if (STATE_START == _this->mState && 0 != _this->player)
	{
		_this->mState = STATE_STOP;
	}
}


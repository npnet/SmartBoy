//
// Created by sine on 18-7-21.
//

#ifndef SMARTBOY_VOICECONNECTIF_H
#define SMARTBOY_VOICECONNECTIF_H

 typedef unsigned char boolean;
#if 0
#include <string>
using namespace std;

//==============播放识别音频部分============
void playerStart(void *_listener);
void playerEnd(void *_listener);
void voice_encoder_VoicePlayer_init(int _sampleRate);
void voice_encoder_VoicePlayer_play(string _text, int _playCount, int _muteInterval);
void voice_encoder_VoicePlayer_playex(intArray _freqs, int _duration, int _playCount, int _muteInterval, bool _dualTone);
void voice_encoder_VoicePlayer_save(string _fileName, string _text, int _playCount, int _muteInterval);
void voice_encoder_VoicePlayer_setVolume(double _volume);
void voice_encoder_VoicePlayer_setFreqs(intArray _freqs);
void voice_encoder_VoicePlayer_stop();
bool voice_encoder_VoicePlayer_isStopped();
void voice_encoder_VoicePlayer_setPlayerType(int _type);
void voice_encoder_VoicePlayer_setWavPlayer(string _fileName);
void voice_encoder_VoicePlayer_mixWav(string _wavFileName, float _volume, int _muteInterval);
void voice_encoder_VoicePlayer_mixAssetWav(jobject _assMgr, string _wavFileName, float _volume, int _muteInterval);



#endif
//==============录音识别部分================
void recognizerStart(void *_listener, float _soundTime);
void recognizerEnd(void *_listener, float _soundTime, int _recogStatus, char *_data, int _dataLen);

void recognizerMatch(void *_listener, int _timeIdx, struct VoiceMatch *_matches, int _matchesLen);

void voice_decoder_VoiceRecognizer_init(int _sampleRate);
void voice_decoder_VoiceRecognizer_setFreqs(int _freqs[],int n);
int  recorderShortWrite(void *_writer, const void *_data, unsigned long _sampleCout);
void *runRecorderVoiceRecognize(void *_recognizer);
void voice_decoder_VoiceRecognizer_start(int _minBufferSize);
void voice_decoder_VoiceRecognizer_pause(int _microSeconds);
void voice_decoder_VoiceRecognizer_stop();
boolean voice_decoder_VoiceRecognizer_isStopped();
int  voice_decoder_VoiceRecognizer_writeBuf(char *_audio, int _dataSize);


#endif //SMARTBOY_VOICECONNECTIF_H
/************************************************************************ 
android/iphone/windows/linux声波通讯库
声波通讯库特征： 
准确性95%以上，其实一般是不会出错的。 
接口非常简单，有完整的示例，3分钟就可以让你的应用增加声波通讯功能 
抗干扰性强，基本上无论外界怎么干扰，信号都是准确的 
基本的编码为16进制，而通过编码可传输任何字符 
性能非常强，没有运行不了的平台，而且通过内存池优化，长时间解码不再分配新内存，可7*24小时运行 
可支持任何平台，常见的平台android, iphone, windows, linux, arm, mipsel都有示例 
详情可查看：http://blog.csdn.net/softlgh 
作者: 夜行侠 QQ:3116009971 邮件：3116009971@qq.com 
************************************************************************/  

#ifdef WIN32
#include <Windows.h>
#include <process.h>
#else
#include<pthread.h>
#include <unistd.h>
#define scanf_s scanf 
#endif
#include <stdio.h>
#include "../include/voiceRecog.h"
#include "../include/audioRecorder.h"
#include "md5.h"
#include "string.h"


void *recorder = NULL;
void *recognizer=NULL;
const char *recorderRecogErrorMsg(int _recogStatus)
{
	char *r = (char *)"unknow error";
	switch(_recogStatus)
	{
	case VR_ECCError:
		r = (char *)"ecc error";
		break;
	case VR_NotEnoughSignal:
		r = (char *)"not enough signal";
		break;
	case VR_NotHeaderOrTail:
		r = (char *)"signal no header or tail";
		break;
	case VR_RecogCountZero:
		r = (char *)"trial has expires, please try again";
		break;
	}
	return r;
}

//识别开始回调函数
void recorderRecognizerStart(void *_listener, float _soundTime)
{
	printf("------------------recognize start\n");
}

//识别结束回调函数
void recorderRecognizerEnd(void *_listener, float _soundTime, int _recogStatus, char *_data, int _dataLen)
{
#if 1
	struct SSIDWiFiInfo wifi;
	struct WiFiInfo macWifi;
	int i;
	enum InfoType it;
	struct PhoneInfo phone;
	char s[100];
	if (_recogStatus == VR_SUCCESS)
	{		
		enum InfoType infoType = vr_decodeInfoType(_data, _dataLen);
		if(infoType == IT_PHONE)
		{
		    printf("%s==>IT_PHONE\n",__FUNCTION__);
			vr_decodePhone(_recogStatus, _data, _dataLen, &phone);
			printf("imei:%s, phoneName:%s", phone.imei, phone.phoneName);
		}
		else if(infoType == IT_SSID_WIFI)
		{
            printf("%s==>IT_SSID_WIFI\n",__FUNCTION__);
			vr_decodeSSIDWiFi(_recogStatus, _data, _dataLen, &wifi);
			printf("ssid:%s, pwd:%s\n", wifi.ssid, wifi.pwd);
		}
		else if(infoType == IT_STRING)
		{
            printf("%s==>IT_STRING\n",__FUNCTION__);
			vr_decodeString(_recogStatus, _data, _dataLen, s, sizeof(s));
			printf("string:%s\n", s);
		}
		else if(infoType == IT_WIFI)
		{
            printf("%s==>IT_WIFI\n",__FUNCTION__);
			vr_decodeWiFi(_recogStatus, _data, _dataLen, &macWifi);
			printf("mac wifi:");
			for (i = 0; i < macWifi.macLen; i ++)
			{
				printf("0x%.2x ", macWifi.mac[i] & 0xff);
			}
			printf(", %s\n", macWifi.pwd);
		}
		else
		{
			printf("------------------recognized data:%s\n", _data);
		}
	}
	else
	{

		printf("------------------recognize invalid data, errorCode:%d, error:%s\n", _recogStatus, recorderRecogErrorMsg(_recogStatus));
	}


#else


	char *data;
	struct WiFiInfo wifiInfo;
	int result=0;
	char resData[4096];
	enum InfoType infoType=vr_decodeInfoType(_data,_dataLen);
	if(infoType==IT_WIFI) {
		vr_decodeWiFi(result,_data,_dataLen,&wifiInfo);
	} else if(infoType==IT_SSID_WIFI){


	} else if(infoType==IT_PHONE){

	} else if(infoType==IT_STRING) {
		vr_decodeString(result, _data, _dataLen, resData, 4096);
	}

    else
    {
        printf("------------------recognize invalid data, errorCode:%d, error:%s\n", _recogStatus, recorderRecogErrorMsg(_recogStatus));
    }
#endif
}

#ifdef WIN32
void runRecorderVoiceRecognize( void * _recognizer)  
#else
void *runRecorderVoiceRecognize( void * _recognizer) 
#endif
{
	vr_runRecognizer(_recognizer);
}

//录音机回调函数
int recorderShortWrite(void *_writer, const void *_data, unsigned long _sampleCout)
{
	char *data = (char *)_data;
	void *recognizer = _writer;
	//return vr_writeData(recognizer, data, ((int)_sampleCout) * 2);
    return vr_writeData(recognizer, data, ((int)_sampleCout) );
}

int freqs[] = {6500,6700,6900,7100,7300,7500,7700,7900,8100,8300,8500,8700,8900,9100,9300,9500,9700,9900,10100};
void test_recorderVoiceRecog()
{
	//void *recorder = NULL;
	int sampleRate = 44100;
	//创建识别器，并设置监听器
	//void *recognizer = vr_createVoiceRecognizer2(MemoryUsePriority, sampleRate);
	recognizer = vr_createVoiceRecognizer2(MemoryUsePriority, sampleRate);
	int r;
	char ccc = 0;
	int i;
	int baseFreq;
	
	baseFreq = 16000;
	for(i = 0; i < sizeof(freqs)/sizeof(int); i ++)
	{
		freqs[i] = baseFreq + i * 150;
	}
	
	vr_setRecognizeFreqs(recognizer, freqs, sizeof(freqs)/sizeof(int));
	vr_setRecognizerListener(recognizer, NULL, recorderRecognizerStart, recorderRecognizerEnd);
	//创建录音机
    //貌似一通道不成功，只能双通道
	r = initRecorder(sampleRate, 2, 16, 512, &recorder);//要求录取short数据
//    r = initRecorder(recorder,sampleRate, 2, 16, 512);//要求录取short数据
	if(r != 0)
	{
		printf("recorder init error:%d", r);
		return;
	}
	//开始录音
	r = startRecord(recorder, recognizer, recorderShortWrite);//short数据
	if(r != 0)
	{
		printf("recorder record error:%d", r);
		return;
	}
	//开始识别

	pthread_t ntid;
	pthread_create(&ntid, NULL, runRecorderVoiceRecognize, recognizer);

	printf("recognize start !!!\n");	
	//do 
	{
		printf("press q to end recognize\n");
		scanf_s("%c", &ccc);
	} 
	//while (ccc != 'q');

	//停止录音
	r = stopRecord(recorder);
	if(r != 0)
	{
		printf("recorder stop record error:%d", r);
	}
	r = releaseRecorder(recorder);
	if(r != 0)
	{
		printf("recorder release error:%d", r);
	}

	//通知识别器停止，并等待识别器真正退出
	vr_stopRecognize(recognizer);
	do 
	{		
		printf("recognizer is quiting\n");
#ifdef WIN32
		Sleep(1000);
#else
		sleep(1);
#endif
	} while (!vr_isRecognizerStopped(recognizer));

	//销毁识别器
	vr_destroyVoiceRecognizer(recognizer);
}

int md5(char  *input,char  *output)
{
    MD5_CTX ctx;
   // char  *data= "123" ;
    unsigned  char  md[16];
    char  buf[33]={ '\0' };
    char  tmp[3]={ '\0' };
    int  i;
    MD5Init(&ctx);
    MD5Update(&ctx,(unsigned char *)input, strlen ((char *)input));
    MD5Final(md,&ctx);
    for ( i=0; i<16; i++ ){
        sprintf (tmp, "%02X" ,md[i]);
        strcat (buf,tmp);
    }
   // printf ( "%s\n" ,buf);
    strcpy(output,buf);
}


int main(int argc, char* argv[])
{
	//test_recorderVoiceRecog();
    /*
    MD5_CTX ctx;
     char  *data= "123" ;
    unsigned  char  md[16];
    char  buf[33]={ '\0' };
    char  tmp[3]={ '\0' };
    int  i;
    MD5Init(&ctx);
    MD5Update(&ctx,(unsigned char *)data, strlen ((char *)data));
    MD5Final(md,&ctx);
    for ( i=0; i<16; i++ ){
        sprintf (tmp, "%02X" ,md[i]);
        strcat (buf,tmp);
    }
    printf ( "%s\n" ,buf);*/
    char  *data= "123" ;
    char output[256];
    md5(data,output);
    printf("%s\n",output);
    return  0;

}
	
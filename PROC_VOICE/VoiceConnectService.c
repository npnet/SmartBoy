//
// Created by sine on 18-7-21.
//

//
// Created by sine on 18-7-19.
//

#include "linux/input.h"
#include "stddef.h"
#include "stdlib.h"

#include <sys/inotify.h>
#include "poll.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <audioRecorder.h>

#include "cJSON.h"
#include "pthread.h"

#include "VoiceConnectIf.h"
#include "voiceRecog.h"
#include "asoundlib.h"


#include "AI_PKTHEAD.h"
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"


int					 server_sock;		// 服务器SOCKET

int wifiStatus;//0:断开 1:连接


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
    struct SSIDWiFiInfo wifi;
    struct WiFiInfo macWifi;
    int i;
    enum InfoType it;
    struct PhoneInfo phone;
    char s[100];
   // NetConfig_Info_Data voice;
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

}

void *runRecorderVoiceRecognize( void * _recognizer)
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
void RecorderVoiceRecog()
{
    void *recorder = NULL;
    int sampleRate = 44100;
    //创建识别器，并设置监听器
    void *recognizer = vr_createVoiceRecognizer2(MemoryUsePriority, sampleRate);
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
}




int  main(int argc, char *argv[])
{

    char			sService[30],sLog[300],sServerIP[30];
    int				read_sock, numfds;
    fd_set			readmask;
    struct timeval	timeout_select;
    int             nError;


    AIcom_ChangeToDaemon();

    // 重定向输出
    SetTraceFile((char *)"VOICECONNECT",(char *)CONFIG_FILE);

    /* 与主进程建立联接 */
    char *sMsg = AIcom_GetConfigString((char *)"Config", (char *)"Socket",(char *)CONFIG_FILE);
    if(sMsg==NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        return(AI_NG);
    };
    strcpy(sService,sMsg);

    server_sock=AIEU_DomainEstablishConnection(sService);
    printf("server_sock=%d\n",server_sock);
    if(server_sock<0) {
        sprintf(sLog,"VOICECONNECT Process : AIEU_DomainEstablishConnection %s error!",sService);
        WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
        return AI_NG;
    };

    // 把本进程的标识送给主进程
    PacketHead stHead;
    memset((char *)&stHead,0,sizeof(PacketHead));
    stHead.iPacketID = PKT_CLIENT_IDENTITY;
    stHead.iRecordNum = NETCONFIG_PROCESS_IDENTITY;
    stHead.lPacketSize = sizeof(PacketHead);
    AAWANTSendPacket(server_sock, (char *)&stHead);

    // 初始化本程序中重要的变量

    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;

    for(;;) {

        FD_ZERO(&readmask);
        FD_SET(server_sock, &readmask);
        read_sock = server_sock;

        /* 检查通信端口是否活跃 */
        numfds = select(read_sock + 1, (fd_set *) &readmask, 0, 0, &timeout_select);

        if (numfds <= 0) {
            continue;
        };

        /* 主控程序发来包 */
        if (FD_ISSET(server_sock, &readmask)) {
            char *lpInBuffer = AAWANTGetPacket(server_sock, &nError);
            if (lpInBuffer == NULL) {
                if (nError == EINTR || nError == 0) {  /* 因信号而中断 */
                    printf("signal interrupt\n");
                    continue;
                };
                /* 主进程关闭了联接，本程序也终止！ */
                //  WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"Upgrade Process : Receive disconnect info from Master Process!");
                printf("close sock\n");
                AIEU_TCPClose(server_sock);

            };
            PacketHead *pHead = (PacketHead *) lpInBuffer;
            switch (pHead->iPacketID) {

                case PKT_SYSTEM_RECEIVE_WIFI_INFO: {
                    RecorderVoiceRecog();

                    break;
                }
                case PKT_ROBOT_WIFI_CONNECT:{
                    wifiStatus=1;
                    break;
                }
                case PKT_ROBOT_WIFI_DISCONNECT:{
                    wifiStatus=0;
                    break;
                }
                default:
                    //WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"upgraded Process : Receive unknown message from Master Process!");printf("upgraded Process : Receive unknown message from Master Process!\n");
                    break;
            };
            free(lpInBuffer);
        }/* if( FD_ISSET(IOT_sock, &readmask) ) */

    }

}


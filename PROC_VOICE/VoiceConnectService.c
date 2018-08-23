//
// Created by sine on 18-7-21.
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
#include <PROC_UPGRADE/include/systool.h>

#include "cJSON.h"
#include "pthread.h"

#include "VoiceConnectIf.h"
#include "voiceRecog.h"
#include "alsa/asoundlib.h"


#include "AI_PKTHEAD.h"
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"
#include "curl/curl.h"
#include "md5.h"

typedef enum{
    NOTHING=0,
    RECOGING,
    FINISH
}WHATRECOGDOING;
int server_sock;		// 服务器SOCKET

int wifiStatus;//0:断开 1:连接
struct NetConfig_Info_Data wifiInfo;
struct Robot_Binding_Data bindData;
int flags_WifiName=0;
int flags_WiFiPassWd=0;
int flags_sUserID=0;
void *recognizer=NULL;
void *recorder = NULL;
int regstatus=NOTHING;
int  regIsTimeOut=0; //0:没有超时;1:超时
int isRegCancel=0;//用作判断是否在识别过程中是否有中断信号，0：没有中断;1:有中断信号

pthread_mutex_t reg_mutex;
pthread_cond_t reg_cond;

#define REQUEST_BIND_PATH  "www.aawant.com/speaker/0.0.1/release/ServiceServlet?serviceId=101"
#define INTERFACE_VER "72"
#define DEVICE_NAME "mt8516"

char BIND_PATH[256];

const char *REPORT_BIND_PATH=REQUEST_BIND_PATH;

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

int Parse(char * s){
    int r;
    char *p=NULL;
    if(s[0]=='1'){
        p=s+1;
        strcpy(wifiInfo.sWifiName,p);
        printf("sWifiName=%s\n",wifiInfo.sWifiName);
        flags_WifiName=1;
    } else if(s[0]=='2'&&flags_WifiName==1){
        p=s+1;
        strcpy(wifiInfo.sWiFiPassWd,p);
        printf("sWiFiPassWd=%s\n",wifiInfo.sWiFiPassWd);
        flags_WiFiPassWd=1;
    } else if(s[0]=='3'&&flags_WifiName==1&&flags_WiFiPassWd==1){
        p=s+1;
        strcpy(wifiInfo.sUserID,p);
        printf("sWiFiPassWd=%s\n",wifiInfo.sUserID);
        flags_sUserID=1;
    }

    if(flags_WifiName==1&&flags_WiFiPassWd==1&&flags_sUserID==1&&strlen(wifiInfo.sWifiName)!=0){
        printf("Sucess to get wifiInfo\n");
        wifiInfo.sIsTimeOut='0';
        AAWANTSendPacket(server_sock, PKT_SYSTEM_RECEIVE_WIFI_INFO, (char *) &wifiInfo, sizeof(struct MediaStatus_Iot_Data));
        flags_sUserID=0;
        flags_WiFiPassWd=0;
        flags_sUserID=0;
        regIsTimeOut=0;
#if 0
        //停止录音
        r = stopRecord(recorder);
        if(r != 0)
        {
            printf("recorder stop record error:%d\n", r);
        }
        /*
        r = releaseRecorder(recorder);
        if(r != 0)
        {
            printf("recorder release error:%d", r);
        }
         */

        //通知识别器停止，并等待识别器真正退出

        printf("%s,识别器地址=%x\n",__FUNCTION__,recognizer);

        vr_stopRecognize(recognizer);
        do
        {
            printf("recognizer is quiting\n");

            sleep(1);
        } while (!vr_isRecognizerStopped(recognizer));

        printf("------>recognizer quit\n");
        //销毁识别器
        vr_destroyVoiceRecognizer(recognizer);
#endif
         pthread_cond_signal(&reg_cond);
    }

    return 0;
}

/**
 *参考数据
 * {"status":1,"tips":"操作成功","notified":0,"bindMac":"00:08:22:D4:82:FB",
 * "bindUserId":"449b1ee65cf24876b9bdd7844f3a21f8","bindPushFlag":"449b1ee65cf24876b9bdd7844f3a21f8",
 * "pushKey":"JM:471fd8aaae384e6f95c772c57e958e63:449b1ee65cf24876b9bdd7844f3a21f8",
 * "name":"mt8516","hardwareId":"1"}
 *只提取pushKey
 *
 */
int ParseBindData(char *s){
    cJSON *root = cJSON_Parse(s);
    if(!root) {
        printf("get root faild !\n");
        return -1;
    }
    cJSON *status = cJSON_GetObjectItem(root, "status");

    if(!status) {
        printf("No status !\n");
        return -1;
    }

    if(status->valueint==1){
        struct Robot_Binding_Data bindData;
        cJSON *pushkey = cJSON_GetObjectItem(root, "pushKey");
        if(!pushkey) {
            printf("No pushkey !\n");
            return -1;
        }
        printf("%s,pushkey=%s\n",__FUNCTION__,pushkey->valuestring);
        strcpy(bindData.sBindID,pushkey->valuestring);

        AAWANTSendPacket(server_sock, PKT_ROBOT_BIND_OK, (char *) &bindData, sizeof(struct Robot_Binding_Data));
        return 0;

    } else if(status->valueint==0){
        AAWANTSendPacketHead(server_sock,PKT_ROBOT_BIND_FAILED);
        return 0;
    }
}

/**
 * curl 回调处理
 * @param buffer
 * @param size
 * @param nmemb
 * @param useless
 * @return
 */
int COnWriteData(void* buffer, size_t size, size_t nmemb, char * useless)
{
    char value[BUFSIZE] = {0};

    memcpy(value, (char *)buffer, size*nmemb);
    printf("-------%s\n", value);
    ParseBindData(value);
    return 0;
}

/**
 * 获取md5的值
 * @param input
 * @param output
 * @return
 */
int GetMd5(char  *input,char  *output)
{
    MD5_CTX ctx;
    unsigned  char  md[16];
    char  buf[33]={ '\0' };
    char  tmp[3]={ '\0' };
    int  i;
    MD5Init(&ctx);
    MD5Update(&ctx,(unsigned char *)input, strlen ((char *)input));
    MD5Final(md,&ctx);
    for( i=0; i<16; i++ ){
        sprintf (tmp, "%02X" ,md[i]);
        strcat (buf,tmp);
    }
    printf("md5 is %s\n",buf);
    strcpy(output,buf);
}


/**
 * 通过curl来处理http get
 * @param url
 * @param Response
 * @return
 */
int CGet(const char *url, char * Response)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    /*
    if(m_bDebug)
    {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
     */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    //返回数据写进文件
    if(NULL!=Response){
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)Response);
    } else{
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, COnWriteData);
    }

    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 4);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}


/**
 * 获取的当前时间
 * @return
 */
int GetCurrentTime(){
    struct timeval currentTime;
    uint32 ms = 0;
    gettimeofday(&currentTime, NULL);
    ms = currentTime.tv_sec*1000000  + (currentTime.tv_usec );
    return ms;
}

int GetDevName(char *name){

}

int GetIfVersion(){

}


void sig_timer(int signo)
{
    if(signo == SIGALRM)
    {
        printf("time over\n");
        regIsTimeOut=1;
        pthread_cond_signal(&reg_cond);

        //exit(0);
    }
    else
        printf("other signal\n");
}

/**
 * 开启计时器
 */
void StartTimer(){
    struct itimerval timer = {0};
    struct sigaction st_sig={0};

    st_sig.sa_handler=sig_timer;

    timer.it_value.tv_sec=20;
    timer.it_value.tv_usec=10;
    timer.it_interval.tv_sec=0;
    timer.it_interval.tv_usec=0;

    if(sigaction(SIGALRM ,&st_sig,NULL) < 0)
        printf("error to set sig proc\n");

    if(setitimer(ITIMER_REAL,&timer,NULL)<0){
        printf("set timer err\n");
    }
}


void *timerHandler(void *arg){
    struct timeval curTime;
    struct timeval startTime;

    gettimeofday(&startTime,NULL);

    while (regIsTimeOut==0){
        gettimeofday(&curTime,NULL);
        if(curTime.tv_sec-startTime.tv_sec>15){
            regIsTimeOut=1;
            pthread_cond_signal(&reg_cond);
        }
    }
}


typedef struct RequestBindInfo_T{
    char hardwareId[256];//Mac地址md5值
    char mac[256];
    char name[256];
    char pushFlag[256];//Userid md5值
    char serviceId[256];
    char time[256];
    char userId[256];
    char version[256];
}RequestBindInfo;

int RequestBinding(){
    char pack[256];
    uint32 t;
    int rs;
    char *sMsg=NULL;
    char *sPath=NULL;
    char *sVer=NULL;
    char bPath[256];

    RequestBindInfo info;
    FUNC_START
    memset(&info,0, sizeof(info));
    GetMac(info.mac,"wlan0");
    //mac--->md5--->hardwareId
    GetMd5(info.mac,info.hardwareId);


    sVer = AIcom_GetConfigString((char *)"Interface", (char *)"serverVer",(char *)CONFIG_FILE);
    if(sVer==NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        strcpy(info.version,INTERFACE_VER);
    } else{
        strcpy(info.version,sVer);
    }

    sMsg = AIcom_GetConfigString((char *)"Config", (char *)"Mode",(char *)CONFIG_FILE);
    if(sMsg==NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        strcpy(info.name,DEVICE_NAME);
    } else{
        strcpy(info.name,sMsg);
    }

    sPath = AIcom_GetConfigString((char *)"Interface", (char *)"server101",(char *)CONFIG_FILE);
    if(sPath==NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        strcpy(bPath,REQUEST_BIND_PATH);
    } else{
        strcpy(bPath,sMsg);
    }

    t=GetCurrentTime();
    sprintf(info.time,"%u",t);

    strcpy(info.userId,wifiInfo.sUserID);
    //userId--->md5--->pushFlag
    GetMd5(info.userId,info.pushFlag);
    strcpy(info.serviceId,"101");

#if 0
    printf("%s\n",info.mac);
    printf("%s\n",info.version);
    printf("%s\n",info.name);
    printf("%s\n",info.time);
    printf("%s\n",info.userId);
    printf("%s\n",info.serviceId);
    printf("%s\n",info.pushFlag);
    printf("%s\n",info.hardwareId);
#endif
    //strcpy(pack,REPORT_BIND_PATH);
    /*
    strcat(pack,info.hardwareId);
    strcat(pack,info.mac);
    strcat(pack,info.name);
    strcat(pack,info.pushFlag);
    strcat(pack,info.serviceId);
    strcat(pack,info.time);
    strcat(pack,info.userId);
    strcat(pack,info.version);
    */

    sprintf(pack,"%s&hardwareId=%s&mac=%s&name=%s&pushFlag=%s&serviceId=%s&time=%s&userId=%s&version=%s",
            bPath,info.hardwareId,info.mac,info.name,info.pushFlag,info.serviceId,info.time,info.userId,info.version);
    printf("pack==>%s\n",pack);
    //
    rs=CGet(pack,NULL);
    if(rs==28){
        printf("%s:上报绑定出错,code=%d\n",__FUNCTION__,rs);
        AAWANTSendPacketHead(server_sock,PKT_ROBOT_BIND_FAILED);
    }

    FUNC_END
    return rs;
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
            LOG("%s==>IT_STRING\n",__FUNCTION__);
            vr_decodeString(_recogStatus, _data, _dataLen, s, sizeof(s));
            printf("string:%s\n", s);
            if(Parse(s)){

            };
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


/**
 * 这线程开启后处于挂起状态，等待信号来触发
 * 触发执行的信号有1、识别到信息 ;2、超时
 * @param _recognizer
 * @return
 */
void *ctrlRecorderVoiceRecognize( void * _recognizer)
{
    FUNC_START
    int r=0;
    struct timeval now;
    struct timespec outtime;
    while (1) {
#if 1
        pthread_mutex_lock(&reg_mutex);
        pthread_cond_wait(&reg_cond, &reg_mutex);
        pthread_mutex_unlock(&reg_mutex);
#else
        gettimeofday(&now,NULL);
        outtime.tv_sec = now.tv_sec + 20;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_mutex_lock(&reg_mutex);
        pthread_cond_timedwait(&reg_cond, &reg_mutex,&outtime);
        pthread_mutex_unlock(&reg_mutex);

#endif
        r = stopRecord(recorder);
        if(r != 0)
        {
            printf("recorder stop record error:%d\n", r);
        }
        vr_stopRecognize(recognizer);
        do {
            printf("recognizer is quiting\n");
            sleep(1);
        } while (!vr_isRecognizerStopped(recognizer));

        printf("------>recognizer quit\n");
        //销毁识别器
        vr_destroyVoiceRecognizer(recognizer);

        //超时发送
        if(regIsTimeOut&&(isRegCancel!=1)){
            strcpy(wifiInfo.sWifiName,"");
            strcpy(wifiInfo.sWiFiPassWd,"");
            strcpy(wifiInfo.sUserID,"");
            wifiInfo.sIsTimeOut='1';
            AAWANTSendPacket(server_sock, PKT_SYSTEM_RECEIVE_WIFI_INFO, (char *) &wifiInfo, sizeof(struct MediaStatus_Iot_Data));
        }

        break;
    }
    regstatus=FINISH;
    FUNC_END
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
int RecorderVoiceRecog()
{
    int sampleRate = 44100;
    //创建识别器，并设置监听器
    recognizer = vr_createVoiceRecognizer2(MemoryUsePriority, sampleRate);
    printf("%s,识别器地址=%x\n",__FUNCTION__,recognizer);
    int r;
    char ccc = 0;
    int i;
    int baseFreq;
    int ret=0;

    memset(&wifiInfo,0,sizeof(wifiInfo));
    baseFreq = 16000;
    for(i = 0; i < sizeof(freqs)/sizeof(int); i ++)
    {
        freqs[i] = baseFreq + i * 150;
    }
    usleep(110000);
    StartTimer();
    //创建录音机
    //貌似一通道不成功，只能双通道
    r = initRecorder(sampleRate, 2, 16, 512, &recorder);//要求录取short数据
    //r = initRecorder(recorder,sampleRate, 2, 16, 512);//要求录取short数据
    if(r != 0)
    {
        printf("recorder init error:%d", r);
        goto err;
    }

    vr_setRecognizeFreqs(recognizer, freqs, sizeof(freqs)/sizeof(int));
    vr_setRecognizerListener(recognizer, NULL, recorderRecognizerStart, recorderRecognizerEnd);

    //开始录音
    r = startRecord(recorder, recognizer, recorderShortWrite);//short数据
    if(r != 0)
    {
        printf("startRecord error:%d\n", r);
      //  exit(0);
        goto err;

    }
    //开始识别
    pthread_t ntid;
    pthread_t mtid;
    ret=pthread_create(&ntid, NULL, runRecorderVoiceRecognize, recognizer);
    if(ret!=0){
        printf("Create pthread runRecorderVoiceRecognize err\n");
        goto err;
    }

    pthread_create(&mtid, NULL, ctrlRecorderVoiceRecognize, recognizer);
    if(ret!=0){
        printf("Create pthread ctrlRecorderVoiceRecognize err\n");
        goto err;
    }


    return 0;
err:
    regstatus=FINISH;
    vr_stopRecognize(recognizer);
    do {
        printf("recognizer is quiting\n");
        sleep(1);
    } while (!vr_isRecognizerStopped(recognizer));

    printf("------>recognizer quit\n");
    //销毁识别器
    vr_destroyVoiceRecognizer(recognizer);

    return -1;
}



int  main(int argc, char *argv[])
{

    char			sService[30],sLog[300],sServerIP[30];
    int				read_sock, numfds;
    fd_set			readmask;
    struct timeval	timeout_select;
    int             nError;
    int ret;


    AIcom_ChangeToDaemon();

    // 重定向输出
    // SetTraceFile((char *)"VOICECONNECT",(char *)CONFIG_FILE);

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

    reg_mutex=PTHREAD_MUTEX_INITIALIZER;
    reg_cond=PTHREAD_COND_INITIALIZER;

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
                return -1;

            };
            PacketHead *pHead = (PacketHead *) lpInBuffer;
            switch (pHead->iPacketID) {

                case PKT_SYSTEM_READY_NETCONFIG:{
                    printf("PKT_SYSTEM_READY_NETCONFIG1:regstatus=%d\n",regstatus);
                    if(regstatus==NOTHING||regstatus==FINISH) {
                        printf("PKT_SYSTEM_READY_NETCONFIG2\n");
                        regstatus=RECOGING;

                        ret=RecorderVoiceRecog();
                        isRegCancel = 0;
                    }
                    break;
                }

                case PKT_SYSTEM_QUIT_NETCONFIG:{
                    isRegCancel=1;
                    pthread_cond_signal(&reg_cond);
                    break;
                }

                /*
                //voice--->main process
                case PKT_SYSTEM_RECEIVE_WIFI_INFO: {
                    printf("PKT_SYSTEM_RECEIVE_WIFI_INFO\n");


                    break;
                }
                */
                case PKT_NETCONFIG_SUCCESS:{
                    printf("start binding\n");
                    RequestBinding();

                    break;
                }
                case PKT_NETCONFIG_FAILED:{
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


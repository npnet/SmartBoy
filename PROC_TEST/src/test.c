/********************************************************
* FILE     : test.c
* CONTENT  : IOT服务主程序
*********************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "AI_PKTHEAD.h"
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"

static int server_sock;

void test_bind_ok(char *sBindID) {
    struct Robot_Binding_Data stRobot_Binding_Data;
    strcpy(stRobot_Binding_Data.sBindID, sBindID);

    AAWANTSendPacket(server_sock, PKT_ROBOT_BIND_OK, (char *) &stRobot_Binding_Data, sizeof(struct Robot_Binding_Data));
}

void test_wifi_ok() {
    AAWANTSendPacketHead(server_sock, PKT_ROBOT_WIFI_CONNECT);
}

void test_alarm_file(char *szFileName) {
    struct sJSON_Data sData;

    memset((char *) &sData, 0, sizeof(struct sJSON_Data));
    AIcom_GetFile(szFileName, sData.sJsonString);

    AAWANTSendPacket(server_sock, PKT_ALARM_SETUP, (char *) &sData, sizeof(struct sJSON_Data));
}

// 媒体点播状态
void test_media_status() {
    struct MediaStatus_Iot_Data stData;
    stData.action = 1;
    strcpy(stData.mediaName, "11111111111111111111111111111");
    strcpy(stData.artist, "2222222222222222222");
    stData.secondTimes = 280;
    strcpy(stData.imgUrl, "http://127.0.0.1/abcdefg");

    AAWANTSendPacket(server_sock, PKT_MEDIA_STATUS, (char *) &stData, sizeof(struct MediaStatus_Iot_Data));

}

#if 0
//下载
void test_download(){
    TO_UPGRADE_DATA upData;
    upData.action=DOWNLOAD_START;
    strcpy(upData.url,"http://192.168.1.118/update.bin");
    AAWANTSendPacket(server_sock,PKT_UPGRADE_CTRL,(char *)&upData,sizeof(TO_UPGRADE_DATA));

}

//下载取消
void test_download_cancel(){
    TO_UPGRADE_DATA upData;
    upData.action=DOWNLOAD_CANCEL;
    strcpy(upData.url,"http://192.168.1.118/update.bin");
    AAWANTSendPacket(server_sock,PKT_UPGRADE_CTRL,(char *)&upData,sizeof(TO_UPGRADE_DATA));

}

//升级
void test_upgrade(){
    TO_UPGRADE_DATA upData;
    upData.action=UPGRADE_START;
    strcpy(upData.url,"http://192.168.1.118/update.bin");
    AAWANTSendPacket(server_sock,PKT_UPGRADE_CTRL,(char *)&upData,sizeof(TO_UPGRADE_DATA));
}

#endif

void test_update() {
    struct UpdateInfoMsg_Iot_Data updata;

    strcpy(updata.updateUrl, "http://192.168.1.118/update.zip");
    strcpy(updata.id, "1");
    strcpy(updata.model, "mt8516");
    updata.nowVersion = 2;
    updata.toVersion = 4;
    AAWANTSendPacket(server_sock, PKT_VERSION_UPDATE, (char *) &updata,
                     sizeof(struct UpdateInfoMsg_Iot_Data));

}

/*
//空闲
AAWANT_SYSTEM_IDLE_TASK = 601,

//播放音乐
        AAWANT_SYSTEM_AUDIO_TASK,

//播放TTS
        AAWANT_SYSTEM_TTS_TASK,

//播放闹铃
        AAWANT_SYSTEM_ALARM_TASK,

//系统正在配置网络
        AAWANT_SYSTEM_NETCONFIG_TASK,

//系统正在拾音
        AAWANT_SYSTEM_MSC_RECOGNIZE,

//系统正在请求服务器
        AAWANT_SYSTEM_REQUEST_SERVLET,

//系统正在执行指令动作
        AAWANT_SYSTEM_COMMAND_CONTROL
        */

/*
 * typedef enum {
    //错误状态
    BLNS_ERROR_STATUS = 401,

    //声音控制状态
    BLNS_VOLUME_STATUS,

    //关闭状态
    BLNS_OFF_STATUS,

    //开机状态
    BLNS_STARTUP_STATUS,

    //唤醒状态
    BLNS_WAKEUP_STATUS,

    //从服务器获取信息状态
    BLNS_GET_SERVER_INFO_STATUS,

    //播报状态
    BLNS_BROADCAST_STATUS,

    //播放状态
    BLNS_PLAY_STATUS,

    //网络配置状态
    BLNS_NET_CONFIG_STATUS,

    //OTA升级状态
    BLNS_UPDATE_STATUS,

    //测试灯状态
    BLNS_TEST_STATUS,

    //其他状态转换成播放状态
    BLNS_SWITCH_PLAY_STATUS

}System_Blns_Status;

 */
void menu() {
    printf("==========Cmd menu==========\n"
           "01:AAWANT_SYSTEM_IDLE_TASK       601\n"
           "02:AAWANT_SYSTEM_AUDIO_TASK      602\n"
           "03:AAWANT_SYSTEM_ALARM_TASK      603\n"
           "04:AAWANT_SYSTEM_NETCONFIG_TASK  604\n"
           "05:AAWANT_SYSTEM_MSC_RECOGNIZE   605\n"
           "06:AAWANT_SYSTEM_REQUEST_SERVLET 606\n"
           "07:AAWANT_SYSTEM_COMMAND_CONTROL 607\n"
           "08:BLNS_ERROR_STATUS             401\n"
           "09:BLNS_VOLUME_STATUS            402\n"
           "10:BLNS_OFF_STATUS               403\n"
           "11:BLNS_STARTUP_STATUS           404\n"
           "12:BLNS_WAKEUP_STATUS            405\n"
           "13:BLNS_GET_SERVER_INFO_STATUS   406\n"
           "14:BLNS_BROADCAST_STATUS         407\n"
           "15:BLNS_PLAY_STATUS              408\n"
           "16:BLNS_NET_CONFIG_STATUS        409\n"
           "17:BLNS_UPDATE_STATUS            410\n"
           "18:BLNS_TEST_STATUS              411\n"
           "19:BLNS_SWITCH_PLAY_STATUS       412\n"
            //"20:BLNS_SWITCH_PLAY_STATUS            413\n"
    );
}


void test_systemtask() {
    char buf[256];
    char buf1[3][50];
    char buf2[50];
    char buf3[50];
    int a;
    int b;
    int i;
    int j;
    char c;
    char *word;
    char *next;
    char *left;

    menu();

    while (1) {

        //memset(buf1,0, sizeof(buf1));
        // memset(buf1,0, sizeof(buf1));
        // memset(buf1,0, sizeof(buf1));
        printf("sine@:");

        //scanf("%s%s%s",buf1,buf2,buf3);
        //b=scanf("%s",buf1);
        gets(buf);

        word = strtok_r(buf, " ", &left);
        if (word != NULL) {
            //printf("word=%s\n",word);
            //printf("left=%s\n",left);

            if (strcmp(word, "menu") == 0) {
                menu();
            } else if (strcmp(word, "task") == 0) {
                //strcpy(next,left);
                word = strtok_r(left, " ", &next);
                if (word != NULL) {
                    printf("word=%s\n", word);
                    System_Task_Status sysStatus;
                    if (strcmp(word, "601") == 0) {

                        sysStatus = AAWANT_SYSTEM_IDLE_TASK;
                    } else {
                        sysStatus = AAWANT_SYSTEM_AUDIO_TASK;
                    }
                    AAWANTSendPacket(server_sock, PKT_SYSTEMTASK_STATUS, (char *) &sysStatus,
                                     sizeof(System_Task_Status));

                }
            } else if (strcmp(word, "auto") == 0) {

            }
        }


        fflush(stdin);
        //printf("sine@:%s\n",buf1);
    }

}


void test_all() {
    char buf[256];
    char buf1[3][50];
    char buf2[50];
    char buf3[50];
    int a;
    int b;
    int i;
    int j;
    char c;
    char *word;
    char *next;
    char *left;

    menu();

    while (1) {


        printf("sine@:");


        gets(buf);

        word = strtok_r(buf, " ", &left);
        if (word != NULL) {
            if (strcmp(word, "menu") == 0) {
                menu();
            } else if (strcmp(word, "task") == 0) {
                //strcpy(next,left);
                word = strtok_r(left, " ", &next);
                if (word != NULL) {
                    printf("word=%s\n", word);
                    System_Task_Status sysStatus;
                    if (strcmp(word, "601") == 0) {

                        sysStatus = AAWANT_SYSTEM_IDLE_TASK;
                    } else {
                        sysStatus = AAWANT_SYSTEM_AUDIO_TASK;
                    }
                    AAWANTSendPacket(server_sock, PKT_SYSTEMTASK_STATUS, (char *) &sysStatus,
                                     sizeof(System_Task_Status));

                }
            } else if (strcmp(word, "perip")==0) {
                word = strtok_r(left, " ", &next);
                System_Blns_Status blns;
                if (word != NULL) {
                    if (strcmp(word, "401") == 0) { blns = BLNS_ERROR_STATUS; }
                    else if (strcmp(word, "402") == 0) { blns = BLNS_VOLUME_STATUS; }
                    else if (strcmp(word, "403") == 0) { blns = BLNS_OFF_STATUS; }
                    else if (strcmp(word, "404") == 0) { blns = BLNS_STARTUP_STATUS; }
                    else if (strcmp(word, "405") == 0) { blns = BLNS_WAKEUP_STATUS; }
                    else if (strcmp(word, "406") == 0) { blns = BLNS_GET_SERVER_INFO_STATUS; }
                    else if (strcmp(word, "407") == 0) { blns = BLNS_BROADCAST_STATUS; }
                    else if (strcmp(word, "408") == 0) { blns = BLNS_PLAY_STATUS; }
                    else if (strcmp(word, "409") == 0) { blns = BLNS_NET_CONFIG_STATUS; }
                    else if (strcmp(word, "410") == 0) { blns = BLNS_UPDATE_STATUS; }
                    else if (strcmp(word, "411") == 0) { blns = BLNS_TEST_STATUS; }
                    else if (strcmp(word, "412") == 0) { blns = BLNS_SWITCH_PLAY_STATUS; }
                }
                //AAWANTSendPacketHead(server_sock,blns);
                PacketHead stPacketHead;

                memset(&stPacketHead, 0, sizeof(PacketHead));
              //  PKT_BLNS_SYSTEM_STATUS

                stPacketHead.iPacketID = PKT_BLNS_SYSTEM_STATUS;
                stPacketHead.lPacketSize = sizeof(PacketHead);
                stPacketHead.iRecordNum = blns;

                if (AAWANTSendPacket(server_sock, (char *) &stPacketHead) < 0) {
                    //AIcom_SetErrorMsg(ERROR_SOCKET_WRITE, NULL, NULL);
                    printf("Send packet err\n");
                   // return -1;
                };

               // return AI_OK;
            } else if (strcmp(word, "auto") == 0) {
                printf("auto \n");
                int i=0;
                PacketHead stPacketHead;
                for(i=0;i<12;i++) {
                    sleep(3);
                    memset(&stPacketHead, 0, sizeof(PacketHead));
                    //  PKT_BLNS_SYSTEM_STATUS

                    stPacketHead.iPacketID = PKT_BLNS_SYSTEM_STATUS;
                    stPacketHead.lPacketSize = sizeof(PacketHead);
                    stPacketHead.iRecordNum = 401+i;

                    if (AAWANTSendPacket(server_sock, (char *) &stPacketHead) < 0) {
                        //AIcom_SetErrorMsg(ERROR_SOCKET_WRITE, NULL, NULL);
                        printf("Send packet err\n");
                        // return -1;
                    };
                }

                for(i=0;i<6;i++) {
                    sleep(3);
                    memset(&stPacketHead, 0, sizeof(PacketHead));
                    //  PKT_BLNS_SYSTEM_STATUS

                    stPacketHead.iPacketID = PKT_BLNS_VALUE_STATUS;
                    stPacketHead.lPacketSize = sizeof(PacketHead);
                    stPacketHead.iRecordNum = 1+i;

                    if (AAWANTSendPacket(server_sock, (char *) &stPacketHead) < 0) {
                        //AIcom_SetErrorMsg(ERROR_SOCKET_WRITE, NULL, NULL);
                        printf("Send packet err\n");
                        // return -1;
                    };
                }
            } else if(strcmp(word, "upgrade")==0){
                struct UpdateInfoMsg_Iot_Data updata;

                strcpy(updata.updateUrl, "http://192.168.1.118/update.zip");
                strcpy(updata.id, "1");
                strcpy(updata.model, "mt8516");
                updata.nowVersion = 2;
                updata.toVersion = 4;
                AAWANTSendPacket(server_sock, PKT_VERSION_UPDATE, (char *) &updata,
                                 sizeof(struct UpdateInfoMsg_Iot_Data));

            } else if(strcmp(word, "voice")==0){
                PacketHead stPacketHead;

                memset(&stPacketHead, 0, sizeof(PacketHead));
                //  PKT_BLNS_SYSTEM_STATUS

                stPacketHead.iPacketID = PKT_SYSTEM_READY_NETCONFIG;
                stPacketHead.lPacketSize = sizeof(PacketHead);
                stPacketHead.iRecordNum = 1;

                if (AAWANTSendPacket(server_sock, (char *) &stPacketHead) < 0) {
                    //AIcom_SetErrorMsg(ERROR_SOCKET_WRITE, NULL, NULL);
                    printf("Send packet err\n");
                    // return -1;
                };
            }

        }

        fflush(stdin);
        //printf("sine@:%s\n",buf1);
    }

}

void test_single(){
    PacketHead stPacketHead;

       // sleep(3);
        memset(&stPacketHead, 0, sizeof(PacketHead));
        //  PKT_BLNS_SYSTEM_STATUS

        stPacketHead.iPacketID = PKT_BLNS_SYSTEM_STATUS;
        stPacketHead.lPacketSize = sizeof(PacketHead);
        stPacketHead.iRecordNum = 401;
        printf("server_sock=%d\n",server_sock);
        if (AAWANTSendPacket(server_sock, (char *) &stPacketHead) < 0) {
            //AIcom_SetErrorMsg(ERROR_SOCKET_WRITE, NULL, NULL);
            printf("Send packet err\n");
            // return -1;
        };


}

#if 0
void test_led(){
    LED_CTL ledCtl;
    ledCtl.mode=1;
    ledCtl.value=0;
    AAWANTSendPacket(server_sock,PKT_LED_CTRL,(char *)&ledCtl,sizeof(LED_CTL));

}

void test_key(){
    KEY_EVENT keyEvent;
    //keyEvent.type=0;
    keyEvent.code=0x1;
    AAWANTSendPacket(server_sock,PKT_LED_CTRL,(char *)&keyEvent,sizeof(KEY_EVENT));
}
#endif

void test_voice_connect() {

}

void test_bt_connect() {

}

static pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

void *Func_test1(void *arg){
    printf("%s:start\n",__FUNCTION__);
    pthread_mutex_lock(&mtx);
    pthread_cond_wait(&cond,&mtx);
    pthread_mutex_unlock(&mtx);
    printf("%s:end\n",__FUNCTION__);
}

void *Func_test2(void *arg){
    printf("%s:start\n",__FUNCTION__);
    pthread_mutex_lock(&mtx);
    pthread_cond_wait(&cond,&mtx);
    pthread_mutex_unlock(&mtx);
    printf("%s:end\n",__FUNCTION__);
}


int test_pthread(){
    pthread_t pid1;
    pthread_t pid2;

    int ret;
    ret=pthread_create(&pid1,NULL,Func_test1,NULL);
    if(ret){
        printf("create pthread 1 failed\n");
        return -1;
    }
    ret=pthread_create(&pid1,NULL,Func_test2,NULL);
    if(ret){
        printf("create pthread 2 failed\n");
        return -1;
    }

    while (1){
        for(int i=0;i<10;i++){
            printf("--%d--\n",i);
            sleep(2);
            if(i==9){
                //pthread_cond_broadcast(&cond);
                pthread_cond_signal(&cond);
            }

        }
    }

}




void kill_master() {
    AAWANTSendPacketHead(server_sock, PKT_SYSTEM_SHUTDOWN);
}

/********************************************************************
 * NAME         : main
 * FUNCTION     : 中间件主函数
 * PARAMETER    : argc,argv
 * RETURN       : 
 * PROGRAMMER   : WTQ
 * DATE(ORG)    : 2018/06/16
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/
int main(int argc, char *argv[]) {
    char sService[30], sLog[300], sServerIP[30], *sMsg;
    int read_sock, numfds;
    fd_set readmask;
    struct timeval timeout_select;
    int nError;
    // 重定向输出
    //nError = SetTraceFile((char *)"TEST",(char *)CONFIG_FILE);
  //  test_pthread();
    /* 与主进程建立联接 */
    sMsg = AIcom_GetConfigString((char *) "Config", (char *) "Socket", (char *) CONFIG_FILE);
    if (sMsg == NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        return (AI_NG);
    };
    strcpy(sService, sMsg);

    server_sock = AIEU_DomainEstablishConnection(sService);
    if (server_sock < 0) {
        sprintf(sLog, "TEST Process : AIEU_DomainEstablishConnection %s error!", sService);
        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
        return AI_NG;
    };

    // 把本进程的标识送给主进程
    PacketHead stHead;
    memset((char *) &stHead, 0, sizeof(PacketHead));
    stHead.iPacketID = PKT_CLIENT_IDENTITY;
    stHead.iRecordNum = TEST_PROCESS_IDENTITY;
    stHead.lPacketSize = sizeof(PacketHead);
    AAWANTSendPacket(server_sock, (char *) &stHead);

    // WIFI连接成功后发该消息到主控进程
    if (argc < 2 || strcasecmp(argv[1], "wifi") == 0) {
        test_wifi_ok();
    };

    // 绑定成功后发该消息到主控进程，必须保证绑定成功后得到的推送标识的正确性
    if (argc >= 2 && strcasecmp(argv[1], "bind") == 0) {
        char sBindID[BUFSIZE];
        if (argc > 2) {
            strcpy(sBindID, argv[2]);
        } else {
            strcpy(sBindID, "1357924680");
        };
        test_bind_ok(sBindID);
    };

    // 媒体点播状态发送到服务器
    if (argc >= 2 && strcasecmp(argv[1], "media") == 0) {
        test_media_status();
    };

    // 媒体点播状态发送到服务器
    if (argc >= 2 && strcasecmp(argv[1], "alarm") == 0) {
        test_alarm_file(argv[2]);
    };

    //发送升级命令给主程序
    if (argc >= 2 && strcasecmp(argv[1], "upgrade") == 0) {
        test_update();
    }

    //发送升级命令给主程序
    if (argc >= 2 && strcasecmp(argv[1], "cli") == 0) {
        //test_systemtask();
        test_all();
        //test_single();
    }

    if (argc >= 2 && strcasecmp(argv[1], "pthread") == 0) {

    };

#if 0
    //发送升级命令给主程序
    if(argc>=2&& strcasecmp(argv[1],"upgrade")==0){
        test_upgrade();
    }

    //发送升级命令给主程序
    if(argc>=2&& strcasecmp(argv[1],"download")==0){
        test_download();
    }


    //发送升级命令给主程序
    if(argc>=2&& strcasecmp(argv[1],"upcancel")==0){
        test_download_cancel();
    }
#endif
    // 终止系统
    if (argc >= 2 && strcasecmp(argv[1], "kill") == 0) {
        kill_master();
    };
}

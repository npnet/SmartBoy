/********************************************************
* FILE     : AawantMain.c
* CONTENT  : 主进程
*********************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "AIprofile.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AI_PKTHEAD.h"
#include "AIUComm.h"
#include "AIEUComm.h"
#include "AawantData.h"

#define  CLIENT_SOCKET_NUM  10

// 得到设备的各种参数数据
int Get_Equipment_Data(struct EquipmentRegister_Iot_Data *pData) {
    memset((char *) pData, 0, sizeof(struct EquipmentRegister_Iot_Data));

    strcpy(pData->mac, "00:56:17:08:18:0b");
    strcpy(pData->name, "IOT TEST ROBOT");
    strcpy(pData->version, "version2");
    strcpy(pData->wifiSSID, "wifiSSID3");
    strcpy(pData->wifiPasswd, "wifiPasswd4");
    strcpy(pData->address_province, "GUANGDONG");
    strcpy(pData->address_city, "GUANGZHOU");
    strcpy(pData->address_district, "TIANHE");
    strcpy(pData->address_street, "LINGHE");
    strcpy(pData->address_position, "TIYU DONG");
    strcpy(pData->address_gis, "123.456 789.012");
    strcpy(pData->address_remained, "TIYU DONG YANGCHENG");
    strcpy(pData->address_setting_remained, "3304");
    strcpy(pData->phoneNumber, "12345678901");
    strcpy(pData->iccid, "iccid1234567890");
    strcpy(pData->netType, "0");

    return AI_OK;
}

/********************************************************************
 * NAME         : StartAawantServer
 * FUNCTION     : 启动主进程
 * PARAMETER    : 
 * PROGRAMMER   : 
 * DATE(ORG)    : 
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/
void StartAawantServer() {
    char sLog[300], sService[30], sClientAddr[20];
    fd_set readmask;
    int server_sock;
    int numfds, read_sock, client_sock;
    int iClientSocketList[CLIENT_SOCKET_NUM], i, nError;
    struct timeval timeout_select;

    int iot_socket;
    int alarm_socket;
    int upgrade_socket;

    char *sMsg = AIcom_GetConfigString((char *) "Config", (char *) "Socket", (char *) CONFIG_FILE);
    if (sMsg == NULL) {
        printf("Fail to get Socket in %s!\n", (char *) CONFIG_FILE);
        exit(1);
    };
    strcpy(sService, sMsg);

    sprintf(sLog, "Master Process : AawantServer start successfully!");
    WriteLog((char *) RUN_TIME_LOG_FILE, sLog);

    /* 监听网络 */
    server_sock = AIEU_DomainListenForConnection(sService);
    if (server_sock < 0) {
        sprintf(sLog, "Master Process : AIEU_DomainListenForConnection %s error!", sService);
        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
        exit(1);
    };

    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;

    // 初始化那些需要主动发消息的socket号
    iot_socket = -1;
    alarm_socket = -1;
    upgrade_socket =-1;


    memset(iClientSocketList, 0, sizeof(int) * CLIENT_SOCKET_NUM);

    /* 接收其他进程的连接请求处理 */
    while (1) {
        FD_ZERO(&readmask);
        FD_SET(server_sock, &readmask);
        read_sock = server_sock;
        for (i = 0; i < CLIENT_SOCKET_NUM; i++) {
            if (iClientSocketList[i]) {
                FD_SET(iClientSocketList[i], &readmask);
                if (iClientSocketList[i] > read_sock) {
                    read_sock = iClientSocketList[i];
                };
            };
        };

        /* 检查通信端口是否活跃 */
        numfds = select(read_sock + 1, (fd_set *) &readmask, 0, 0, &timeout_select);

        if (numfds <= 0)
            continue;

        /* 有新的SOCKET连接 */
        if (FD_ISSET(server_sock, &readmask)) {
            client_sock = AIEU_DomainDoAccept(server_sock, sClientAddr);
            if (client_sock < 0) {
                sprintf(sLog, "Master Process : AIEU_DomainDoAccept Error!");
                WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                exit(1);
            };
            for (i = 0; i < CLIENT_SOCKET_NUM; i++) {
                if (iClientSocketList[i] == 0) {
                    iClientSocketList[i] = client_sock;
                    sprintf(sLog, "Master Process : Connect with %s, socket Number is %d", sClientAddr, client_sock);
                    WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                    break;
                };
            };
        }/* if( FD_ISSET(server_sock, &readmask) ) */

        /* 有SOCKET通讯需求 */
        for (i = 0; i < CLIENT_SOCKET_NUM; i++) {
            if (iClientSocketList[i] && FD_ISSET(iClientSocketList[i], &readmask)) { // iClientSocketList[i]有消息发到
                char *lpInBuffer = AAWANTGetPacket(iClientSocketList[i], &nError);

                if (lpInBuffer == NULL) {
                    if (nError == EINTR || nError == 0) {  /* 因信号而中断 */
                        continue;
                    };

                    /* 对方进程关闭了 */
                    sprintf(sLog, "Master Process : socket %d closed", iClientSocketList[i]);
                    WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                    AIEU_TCPClose(iClientSocketList[i]);
                    iClientSocketList[i] = 0;
                    continue;
                }
                PacketHead *pHead = (PacketHead *) lpInBuffer;

                sprintf(sLog, "Master Process : Receive packet from socket %d, ID is %d", iClientSocketList[i],
                        pHead->iPacketID);
                WriteLog((char *) RUN_TIME_LOG_FILE, sLog);

                switch (pHead->iPacketID) {
                    case PKT_CLIENT_IDENTITY:   // 各进程发给主控进程，标识身份，包头中iRecordNum存放各进程标识值
                        if (pHead->iRecordNum == IOT_PROCESS_IDENTITY) {
                            iot_socket = iClientSocketList[i];
                        };
                        if (pHead->iRecordNum == ALARM_PROCESS_IDENTITY) {
                            alarm_socket = iClientSocketList[i];
                        };
                        if(pHead->iRecordNum== UPGRADE_PROCESS_IDENTITY){
                            upgrade_socket =iClientSocketList[i];
                        }

                        break;
                    case PKT_ROBOT_BIND_OK:            // 收到绑定成功的消息
                        // 转发给IOT进程
                        if (iot_socket > 0) {
                            AAWANTSendPacket(iot_socket, lpInBuffer);
                        };
                        break;
                    case PKT_ROBOT_WIFI_CONNECT:    // 收到WIFI联接成功的消息
                        // 转发给IOT进程
                        if (iot_socket > 0) {
                            AAWANTSendPacket(iot_socket, lpInBuffer);
                        };
                        break;
                    case PKT_FACTORY_RESET:            // 恢复出厂设置

                        break;
                    case PKT_SYSTEM_SHUTDOWN:        // 系统停止
                        exit(0);
                    case PRK_GET_DEVICEINFO:        // 得到设备信息
                    {
                        struct EquipmentRegister_Iot_Data stData;

                        Get_Equipment_Data(&stData);
                        AAWANTSendPacket(iClientSocketList[i], PRK_GET_DEVICEINFO, (char *) &stData,
                                         sizeof(struct EquipmentRegister_Iot_Data));
                        break;
                    };
                    case PKT_LANGUAGE_CHANGE:        // 语言改变
                    {
                        struct LanguageChange_Iot_Data *pData;

                        pData = (struct LanguageChange_Iot_Data *) (lpInBuffer + sizeof(PacketHead));
                        sprintf(sLog, "Master Process : language is %s", pData->language == 1 ? "普通话" : "粤语");
                        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                    };
                        break;
                    case PRK_MEDIA_PLAY_LIST:        // 收到媒体点播列表
                    {

                    }
                        break;
                    case PKT_MEDIA_ON_DEMAND:        // 收到媒体点播消息
                    {
                        struct MediaOnDemand_Iot_Data *pData;

                        pData = (struct MediaOnDemand_Iot_Data *) (lpInBuffer + sizeof(PacketHead));
                        sprintf(sLog, "Master Process : MediaName[%s] Artist[%s] Author[%s] PlayUrl[%s]",
                                pData->mediaName, pData->artist, pData->author, pData->mediaPlayUrl);
                        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                    };
                        break;
                    case PKT_MEDIA_STATUS:            // 收到媒体点播状态消息
                        // 转发给IOT进程
                        if (iot_socket > 0) {
                            AAWANTSendPacket(iot_socket, lpInBuffer);
                        };
                        break;
                    case PKT_VERSION_UPDATE:        // 收到版本更新包
                    {
                        struct UpdateInfoMsg_Iot_Data *pData;

                        pData = (struct UpdateInfoMsg_Iot_Data *) (lpInBuffer + sizeof(PacketHead));
                        sprintf(sLog, "Master Process : 当前版本[%d]升级版本[%d]机型[%s]URL[%s]", pData->nowVersion,
                                pData->toVersion, pData->model, pData->updateUrl);
                        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                    };
                        break;
                    case PKT_ALARM_SETUP:            // 闹钟设置数据包
                        // 转发给闹钟进程
                        if (alarm_socket > 0) {
                            sprintf(sLog, "Master Process : Recv a alarm and send to Alarm Process");
                            WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                            AAWANTSendPacket(alarm_socket, lpInBuffer);
                        };
                        break;
                    case PKT_ALARM_REMIND:            // 闹钟
                        struct Alarm_Remind_Data *pData;
                        pData = (struct Alarm_Remind_Data *) (lpInBuffer + sizeof(PacketHead));
                        sprintf(sLog,
                                "Master Process : Recv a alarm, parameter sTime[%s]sEvent[%s]sAsk[%s]sSentence[%s]",
                                pData->sTime, pData->sEvent, pData->sAsk, pData->sSentence);
                        WriteLog((char *) RUN_TIME_LOG_FILE, sLog);
                        break;

                    case PKT_UPGRADE_FEEDBACK: {
                        printf("main<==upgrade\n");
                        FROM_UPGRADE_DATA *updata;
                        updata = (FROM_UPGRADE_DATA *) (lpInBuffer + sizeof(PacketHead));
                        printf("get upgrade status code:%d\n",updata->code);
                        break;
                    }
                    //用做测试
                    case PKT_UPGRADE_CTRL:{
                        printf("PKT_UPGRADE_CTRL\n");
                        TO_UPGRADE_DATA *upgData;
                        upgData = (TO_UPGRADE_DATA *)(lpInBuffer+sizeof(PacketHead));
                        if(upgData->action==DOWNLOAD_START){
                            printf("get msg==>download start\n");

                            if(upgrade_socket>0) {
                                AAWANTSendPacket(upgrade_socket, lpInBuffer);
                            }
                        }
                        else if(upgData->action==DOWNLOAD_PAUSE){
                            printf("get msg==>download pause\n");
                        } else if(upgData->action==DOWNLOAD_CONTINUE){
                            printf("get msg==>download continue\n");
                        } else if(upgData->action==DOWNLOAD_CANCEL){
                            printf("get msg==>download cancel\n");

                            if(upgrade_socket>0) {
                                AAWANTSendPacket(upgrade_socket, lpInBuffer);
                            }
                        } else if(upgData->action==UPGRADE_START){
                            printf("get msg==>upgrade start\n");

                            if(upgrade_socket>0) {
                                AAWANTSendPacket(upgrade_socket, lpInBuffer);
                            }
                        }

                        break;
                    }


                } /* switch */;
                free(lpInBuffer);
            }; /* if */
        };/* for */
    }/* while(1) */
    exit(0);
}

/********************************************************************
 * NAME         : main
 * FUNCTION     : 主函数
 * PARAMETER    : argc,argv
 * RETURN       : 
 * PROGRAMMER   : Lenovo-AI
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/
int main(int argc, char *argv[]) {
    AIcom_ChangeToDaemon();

    // 重定向输出
   // SetTraceFile((char *) "MAIN", (char *) CONFIG_FILE);

    StartAawantServer();
    return 0;
}

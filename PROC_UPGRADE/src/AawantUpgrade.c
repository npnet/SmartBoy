//
// Created by sine on 18-7-10.
//

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
#include "curl/curl.h"

#define  CLIENT_SOCKET_NUM  10


typedef enum{
    BEING=0,
    PAUSE,
    FAIL,
    UPDATE
}UpdateStatus;


// 得到设备的各种参数数据

/********************************************************************
 * NAME         : StartUdpgradeServer
 * FUNCTION     : 启动主进程
 * PARAMETER    :
 * PROGRAMMER   :
 * DATE(ORG)    :
 * UPDATE       :
 * MEMO         :
 ********************************************************************/


void StartUpgradeServer()
{
    char	sLog[300],sService[30],sClientAddr[20];
    fd_set	readmask;
    int		server_sock,iot_socket;
    int     numfds,read_sock,client_sock;
    int     iClientSocketList[CLIENT_SOCKET_NUM],i,nError;
    struct timeval	timeout_select;

    AIcom_GetProfileString((char *)"Config", (char *)"Port", (char *)"NONE",sService, 30, (char *)SYSTEM_INI_FILE);
    if(strcmp(sService, "NONE") == 0) {
        printf("Fail to get Port in %s!\n", (char *)SYSTEM_INI_FILE);
        exit(1);
    };

    sprintf(sLog,"Master Process : AawantServer start successfully!");
    WriteLog((char *)RUN_TIME_LOG_FILE,sLog);

    /* 监听网络 */
    server_sock=AIEU_TCPListenForConnection(sService);
    if(server_sock<0) {
        sprintf(sLog,"Master Process : AIEU_TCPListenForConnection %s error!",sService);
        WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
        exit(1);
    };

    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;

    // 初始化那些需要主动发消息的socket号
    iot_socket = -1;

    memset(iClientSocketList,0,sizeof(int)*CLIENT_SOCKET_NUM);

    /* 接收其他进程的连接请求处理 */
    while(1) {
        FD_ZERO(&readmask);
        FD_SET(server_sock,&readmask);
        read_sock = server_sock;
        for(i=0;i<CLIENT_SOCKET_NUM;i++) {
            if(iClientSocketList[i]) {
                FD_SET(iClientSocketList[i],&readmask);
                if(iClientSocketList[i]>read_sock) {
                    read_sock = iClientSocketList[i];
                };
            };
        };

        /* 检查通信端口是否活跃 */
        numfds=select(read_sock+1,(fd_set *)&readmask,0,0,&timeout_select);

        if(numfds<=0)
            continue;

        /* 有新的SOCKET连接 */
        if( FD_ISSET(server_sock, &readmask))	{
            client_sock=AIEU_TCPDoAccept(server_sock,sClientAddr);
            if(client_sock<0) {
                sprintf(sLog,"Master Process : AIEU_TCPDoAccept Error!");
                WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
                exit(1);
            };
            for(i=0;i<CLIENT_SOCKET_NUM;i++) {
                if(iClientSocketList[i]==0) {
                    iClientSocketList[i]=client_sock;
                    sprintf(sLog,"Master Process : Connect with %s, socket Number is %d",sClientAddr,client_sock);
                    WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
                    break;
                };
            };
        }/* if( FD_ISSET(server_sock, &readmask) ) */

        /* 有SOCKET通讯需求 */
        for(i=0;i<CLIENT_SOCKET_NUM;i++) {
            if(iClientSocketList[i] && FD_ISSET(iClientSocketList[i], &readmask)) { // iClientSocketList[i]有消息发到
                char *lpInBuffer=AAWANTGetPacket(iClientSocketList[i], &nError);

                if(lpInBuffer==NULL) {
                    if(nError == EINTR ||nError ==0) {  /* 因信号而中断 */
                        continue;
                    };

                    /* 对方进程关闭了 */
                    sprintf(sLog,"Master Process : socket %d closed",iClientSocketList[i]);
                    WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
                    AIEU_TCPClose(iClientSocketList[i]);
                    iClientSocketList[i]=0;
                    continue;
                }
                PacketHead *pHead = (PacketHead *)lpInBuffer;

                sprintf(sLog,"Master Process : Receive packet from socket %d, ID is %d",iClientSocketList[i],pHead->iPacketID);
                WriteLog((char *)RUN_TIME_LOG_FILE,sLog);

                switch(pHead->iPacketID) {

                    case PKT_VERSION_UPDATE:		// 收到版本更新包
                    {
                        struct UpdateInfoMsg_Iot_Data *pData;

                        pData = (struct UpdateInfoMsg_Iot_Data *)(lpInBuffer+sizeof(PacketHead));
                        sprintf(sLog,"Master Process : 当前版本[%d]升级版本[%d]机型[%s]URL[%s]",pData->nowVersion,pData->toVersion,pData->model,pData->updateUrl);
                        WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
                    };
                        break;
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
int main(int argc, char *argv[])
{
    AIcom_ChangeToDaemon();

    // 重定向输出
    //SetTraceFile((char *)"/var",(char *)"MAIN");

    /* 设置应用服务程序访问路径 */
    AIcom_SetSystemRunTop((char *)DVLP_DIR);

    /* 检查环境变量 */
    if(AIcom_GetCurrentPath() == NULL) {
        printf("Please setup the environment variable %s first!\n",(char *)DVLP_DIR);
        return(AI_NG);
    };

    StartUpgradeServer();
    return 0;
}

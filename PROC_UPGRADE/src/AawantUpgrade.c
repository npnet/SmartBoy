/********************************************************
* FILE     : ProcAlarm.c
* CONTENT  : 闹钟服务主程序
*********************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "AI_PKTHEAD.h"
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"
#include "cJSON.h"
#include <upg_control.h>



/*************************************************************************************************************************************************************************************
 *                                                                                                                                                                                   *
 *                                                                                                                                                                                   *
 *                                                                             升级 管 理                                                                                        *
 *                                                                                                                                                                                   *
 *                                                                                                                                                                                   *
 ************************************************************************************************************************************************************************************/

int					 server_sock;		// 服务器SOCKET


int  main(int argc, char *argv[])
{
    char			sService[30],sLog[300],sServerIP[30];
    int				read_sock, numfds;
    fd_set			readmask;
    struct timeval	timeout_select;
    int             nError;

    AIcom_ChangeToDaemon();

    // 重定向输出
    //SetTraceFile((char *)"UPGRADE",(char *)CONFIG_FILE);

    /* 与主进程建立联接 */
    char *sMsg = AIcom_GetConfigString((char *)"Config", (char *)"Socket",(char *)CONFIG_FILE);
    if(sMsg==NULL) {
        printf("Fail to get Socket in %s!\n", CONFIG_FILE);
        return(AI_NG);
    };
    strcpy(sService,sMsg);

    server_sock=AIEU_DomainEstablishConnection(sService);
    if(server_sock<0) {
        sprintf(sLog,"UPGRADE Process : AIEU_DomainEstablishConnection %s error!",sService);
        WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
        return AI_NG;
    };

    // 把本进程的标识送给主进程
    PacketHead stHead;
    memset((char *)&stHead,0,sizeof(PacketHead));
    stHead.iPacketID = PKT_CLIENT_IDENTITY;
    stHead.iRecordNum = UPGRADE_PROCESS_IDENTITY;
    stHead.lPacketSize = sizeof(PacketHead);
    AAWANTSendPacket(server_sock, (char *)&stHead);

    // 初始化本程序中重要的变量

    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;
    for(;;) {
        FD_ZERO(&readmask);
        FD_SET(server_sock,&readmask);
        read_sock = server_sock;

        /* 检查通信端口是否活跃 */
        numfds=select(read_sock+1,(fd_set *)&readmask,0,0,&timeout_select);

        if(numfds<=0) {
            continue;
        };

        /* 主控程序发来包 */
        if( FD_ISSET(server_sock, &readmask))	{
            char *lpInBuffer=AAWANTGetPacket(server_sock, &nError);
            if(lpInBuffer==NULL) {
                if(nError == EINTR ||nError == 0) {  /* 因信号而中断 */
                    continue;
                };
                /* 主进程关闭了联接，本程序也终止！ */
              //  WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"Upgrade Process : Receive disconnect info from Master Process!");
                AIEU_TCPClose(server_sock);


            };
            PacketHead *pHead = (PacketHead *)lpInBuffer;
            switch(pHead->iPacketID) {

                case PKT_UPGRADE_CTRL:
                {
                    TO_UPGRADE_DATA *upData;

                    upData = (TO_UPGRADE_DATA *)(lpInBuffer+sizeof(PacketHead));

                    if(upData->action==DOWNLOAD_START){
                        printf("Upgrade:start download\n");
                        createDownloadPthread();

                    } else if(upData->action==DOWNLOAD_CONTINUE){
                        printf("Upgrade:continue\n");
                    }
                    else if(upData->action==DOWNLOAD_PAUSE){
                        printf("Upgrade:pause\n");

                    }
                    else if(upData->action==DOWNLOAD_CANCEL){
                        printf("Upgrade:cancel\n");

                    } else if(upData->action==UPGRADE_START){
                        printf("Upgrade:start upgrade\n");

                    }


                }
                    break;

                default:
                    //WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"upgraded Process : Receive unknown message from Master Process!");
                    printf("upgraded Process : Receive unknown message from Master Process!\n");
                    break;
            };
            free(lpInBuffer);
        }/* if( FD_ISSET(IOT_sock, &readmask) ) */
    };
}


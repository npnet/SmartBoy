//
// Created by sine on 18-7-19.
//

#include "keydef.h"
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
#include "AI_PKTHEAD.h"
#include "AawantData.hbak"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"
#include "cJSON.h"
#include "pthread.h"

#define LED_DEVICE_PATH "/dev/input"

int					 server_sock;		// 服务器SOCKET

int Led_init(){
//    int fd=open(LED_DEVICE_PATH);
//    if
    return 0;
}

int Led_Ctrl(int mode,int value){
    printf("[%s]===>\n",__FUNCTION__);
    return 0;
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
    SetTraceFile((char *)"LED",(char *)CONFIG_FILE);

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
        sprintf(sLog,"LED Process : AIEU_DomainEstablishConnection %s error!",sService);
        WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
        return AI_NG;
    };


    // 把本进程的标识送给主进程
    PacketHead stHead;
    memset((char *)&stHead,0,sizeof(PacketHead));
    stHead.iPacketID = PKT_CLIENT_IDENTITY;
    stHead.iRecordNum = LED_PROCESS_IDENTITY;
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
                    printf("signal interrupt\n");
                    continue;
                };
                /* 主进程关闭了联接，本程序也终止！ */
                //  WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"Upgrade Process : Receive disconnect info from Master Process!");
                printf("close sock\n");
                AIEU_TCPClose(server_sock);


            };
            PacketHead *pHead = (PacketHead *)lpInBuffer;
            switch(pHead->iPacketID) {

                case PKT_LED_CTRL: {
                    LED_CTL ledCtl;
                    LED_CTRL_FEEDBACK ledBack;
                    int result;
                    ledCtl = (LED_CTL *) (lpInBuffer + sizeof(PacketHead));
                    if(ledCtl.mode==1){
                        result=Led_Ctrl(ledCtl.mode,ledCtl.value);
                    }

                    ledBack.result = result;

                    AAWANTSendPacket(server_sock, PKT_LED_FEEDBACK, (char *) &ledBack,
                                     sizeof(ledBack));
                    break;
                }
                default:
                    //WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"upgraded Process : Receive unknown message from Master Process!");printf("upgraded Process : Receive unknown message from Master Process!\n");
                    break;
            };
            free(lpInBuffer);
        }/* if( FD_ISSET(IOT_sock, &readmask) ) */

    };

}

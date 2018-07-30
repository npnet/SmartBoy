//
// Created by sin on 18-7-29.
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
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"
#include "cJSON.h"
#include "pthread.h"
#include "blns.h"

#define INPUT_DEVICE_PATH "/dev/input"
#define LED_DEVICE_PATH  "/dev/blns"

int					 server_sock;		// 服务器SOCKET
static struct pollfd *ufds;
int blns_fd=0;


int KeyDevInit(char *path){
    int fd;
    fd=open(path,O_RDWR, 0);
    if(fd <0){
        printf("[%s]==>\n",__FUNCTION__);
        return -1;
    }
    return fd;
}

void *keyThread(void *arg){

    while(1){


    }

}

int CreateKeyThread(){
    pthread_t keyId;
    int ret=pthread_create(&keyId,NULL,&keyThread,NULL);
    if(ret<0){
        printf("create thread fail\n");
    }

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
    SetTraceFile((char *)"KEYEVENT",(char *)CONFIG_FILE);

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
        sprintf(sLog,"KEYEVENT Process : AIEU_DomainEstablishConnection %s error!",sService);
        WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
        return AI_NG;
    };


    // 把本进程的标识送给主进程
    PacketHead stHead;
    memset((char *)&stHead,0,sizeof(PacketHead));
    stHead.iPacketID = PKT_CLIENT_IDENTITY;
    stHead.iRecordNum = PERIPHERAL_PROCESS_IDENTITY;
    stHead.lPacketSize = sizeof(PacketHead);
    AAWANTSendPacket(server_sock, (char *)&stHead);

    blns_fd=open(LED_DEVICE_PATH,O_RDWR);
    if(blns_fd<0) {
        printf("open blns failed\n");
    }





    // 初始化本程序中重要的变量

    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;



    CreateKeyThread();

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

            PacketHead *pHead = (PacketHead *) lpInBuffer;
            switch (pHead->iPacketID) {
                    //系统状态灯光
                case PKT_BLNS_SYSTEM_STATUS:
                    //音量控制灯光
                case PKT_BLNS_VALUE_STATUS: {
                    System_Blns_Status *blns;
                    int cmd;
                    int sw = 0;
                    blns = (System_Blns_Status *) (lpInBuffer + sizeof(PacketHead));

                    if (*blns == BLNS_ERROR_STATUS) {
                        cmd = AW9523B_IO_ERROR;
                    } else if (*blns == BLNS_VOLUME_STATUS) {
                        cmd = AW9523B_IO_VOLUME;
                    } else if (*blns == BLNS_OFF_STATUS) {
                        cmd = AW9523B_IO_OFF;
                    } else if (*blns == BLNS_STARTUP_STATUS) {
                        cmd = AW9523B_IO_START_UP;
                    } else if (*blns == BLNS_WAKEUP_STATUS) {
                        cmd = AW9523B_IO_WAKEUP;
                    } else if (*blns == BLNS_GET_SERVER_INFO_STATUS) {
                        cmd = AW9523B_IO_GIFS;
                    } else if (*blns == BLNS_BROADCAST_STATUS) {
                        cmd = AW9523B_IO_BROADCAST;
                    } else if (*blns == BLNS_PLAY_STATUS) {
                        cmd = AW9523B_IO_PLAY;
                    } else if (*blns == BLNS_NET_CONFIG_STATUS) {
                        cmd = AW9523B_IO_NETCONFIG;
                    } else if (*blns == BLNS_UPDATE_STATUS) {
                        cmd = AW9523B_IO_UPDATE;
                    } else if (*blns == BLNS_TEST_STATUS) {
                        cmd = AW9523B_IO_TEST;
                    } else if (*blns == BLNS_SWITCH_PLAY_STATUS) {
                        cmd = AW9523B_IO_SWITCH_PLAY;
                    }

                    if (ioctl(blns_fd, cmd, &sw) < 0) {
                        printf("IOCTL DATA FAIL...\n");
                    }
                }
            case PKT_ROBOT_WIFI_CONNECT: {

                break;
            }
            case PKT_ROBOT_WIFI_DISCONNECT: {
                break;
            }

            default:
                    //WriteLog((char *)RUN_TIME_LOG_FILE,(char *)"upgraded Process : Receive unknown message from Master Process!");
                    printf("upgraded Process : Receive unknown message from Master Process!\n");
                    break;
            };
            free(lpInBuffer);

        }

    }/* if( FD_ISSET(IOT_sock, &readmask) ) */



};

#if 0
while(1){

        poll(ufds, nfds, -1);
        //printf("poll %d, returned %d\n", nfds, pollres);
        if(ufds[0].revents & POLLIN) {
            read_notify(device_path, ufds[0].fd, print_flags);
        }
        for(i = 1; i < nfds; i++) {
            if(ufds[i].revents) {
                if(ufds[i].revents & POLLIN) {
                    res = read(ufds[i].fd, &event, sizeof(event));
                    if(res < (int)sizeof(event)) {
                        fprintf(stderr, "could not get event\n");
                        return 1;
                    }
                    if(get_time) {
                        printf("[%8ld.%06ld] ", event.time.tv_sec, event.time.tv_usec);
                    }
                    if(print_device)
                        printf("%s: ", device_names[i]);
                    print_event(event.type, event.code, event.value, print_flags);
                    if(sync_rate && event.type == 0 && event.code == 0) {
                        int64_t now = event.time.tv_sec * 1000000LL + event.time.tv_usec;
                        if(last_sync_time)
                            printf(" rate %lld", 1000000LL / (now - last_sync_time));
                        last_sync_time = now;
                    }
                    printf("%s", newline);
                    if(event_count && --event_count == 0)
                        return 0;
                }
            }
        }
    }
#endif


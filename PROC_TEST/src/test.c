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
#include "AI_PKTHEAD.h"
#include "AawantData.h"
#include "AIcom_Tool.h"
#include "AILogFile.h"
#include "AIprofile.h"
#include "AIUComm.h"

static  int  server_sock;

void test_bind_ok(char *sBindID)
{
	struct Robot_Binding_Data stRobot_Binding_Data;
	strcpy(stRobot_Binding_Data.sBindID,sBindID);
	
	AAWANTSendPacket(server_sock,PKT_ROBOT_BIND_OK,(char *)&stRobot_Binding_Data,sizeof(struct Robot_Binding_Data));
}

void test_wifi_ok()
{
	AAWANTSendPacketHead(server_sock,PKT_ROBOT_WIFI_CONNECT);
}

void test_alarm_file(char *szFileName)
{
	struct sJSON_Data sData;

	memset((char *)&sData,0,sizeof(struct sJSON_Data));
	AIcom_GetFile(szFileName,sData.sJsonString);

	AAWANTSendPacket(server_sock,PKT_ALARM_SETUP,(char *)&sData,sizeof(struct sJSON_Data));
}

// 媒体点播状态
void test_media_status()
{
	struct MediaStatus_Iot_Data stData;
	stData.action = 1;
	strcpy(stData.mediaName,"11111111111111111111111111111");
	strcpy(stData.artist,"2222222222222222222");
	stData.secondTimes = 280;
	strcpy(stData.imgUrl,"http://127.0.0.1/abcdefg");

	AAWANTSendPacket(server_sock,PKT_MEDIA_STATUS,(char *)&stData,sizeof(struct MediaStatus_Iot_Data));
	
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

void test_update(){
	struct UpdateInfoMsg_Iot_Data updata;

	strcpy(updata.updateUrl,"http://192.168.1.118/update.zip");
	strcpy(updata.id,"1");
	strcpy(updata.model,"mt8516");
	updata.nowVersion=2;
	updata.toVersion=4;
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
void menu(){
    printf("==========Cmd menu==========\n"
           "1:AAWANT_SYSTEM_IDLE_TASK       601\n"
           "2:AAWANT_SYSTEM_AUDIO_TASK      602\n"
           "3:AAWANT_SYSTEM_ALARM_TASK      603\n"
           "4:AAWANT_SYSTEM_NETCONFIG_TASK  604\n"
           "5:AAWANT_SYSTEM_MSC_RECOGNIZE   605\n"
           "6:AAWANT_SYSTEM_REQUEST_SERVLET 606\n"
           "7:AAWANT_SYSTEM_COMMAND_CONTROL 607\n");
}



void test_systemtask(){
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

   while (1){

       //memset(buf1,0, sizeof(buf1));
      // memset(buf1,0, sizeof(buf1));
      // memset(buf1,0, sizeof(buf1));
       printf("sine@:");

       //scanf("%s%s%s",buf1,buf2,buf3);
	   //b=scanf("%s",buf1);
	   gets(buf);

		word=strtok_r(buf," ",&left);
		if(word!=NULL){
			//printf("word=%s\n",word);
			//printf("left=%s\n",left);

			if(strcmp(word,"menu")==0){
				menu();
			} else if(strcmp(word,"task")==0){
				//strcpy(next,left);
				word=strtok_r(left," ",&next);
				if(word!=NULL){
					printf("word=%s\n",word);
					System_Task_Status sysStatus;
					if(strcmp(word,"601")==0){

						sysStatus =AAWANT_SYSTEM_IDLE_TASK;
					} else{
						sysStatus =AAWANT_SYSTEM_AUDIO_TASK;
					}
					AAWANTSendPacket(server_sock, PKT_SYSTEMTASK_STATUS, (char *) &sysStatus,
									 sizeof(System_Task_Status));

				}
			}
		}




	   fflush(stdin);
       //printf("sine@:%s\n",buf1);
   }

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

void test_voice_connect(){

}

void test_bt_connect(){

}




void kill_master()
{
	AAWANTSendPacketHead(server_sock,PKT_SYSTEM_SHUTDOWN);
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
int  main(int argc, char *argv[])
{
	char			sService[30],sLog[300],sServerIP[30],*sMsg;
	int				read_sock, numfds;
	fd_set			readmask;
	struct timeval	timeout_select;
	int             nError;
		
	// 重定向输出
	//nError = SetTraceFile((char *)"TEST",(char *)CONFIG_FILE);

	/* 与主进程建立联接 */
	sMsg = AIcom_GetConfigString((char *)"Config", (char *)"Socket",(char *)CONFIG_FILE);
	if(sMsg==NULL) {
		printf("Fail to get Socket in %s!\n", CONFIG_FILE);
		return(AI_NG);
	};
	strcpy(sService,sMsg);
	
	server_sock=AIEU_DomainEstablishConnection(sService);
	if(server_sock<0) {
		sprintf(sLog,"TEST Process : AIEU_DomainEstablishConnection %s error!",sService);
		WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
		return AI_NG;
	};

	// 把本进程的标识送给主进程
	PacketHead stHead;
	memset((char *)&stHead,0,sizeof(PacketHead));
	stHead.iPacketID = PKT_CLIENT_IDENTITY;
	stHead.iRecordNum = TEST_PROCESS_IDENTITY;
	stHead.lPacketSize = sizeof(PacketHead);
	AAWANTSendPacket(server_sock, (char *)&stHead);

	// WIFI连接成功后发该消息到主控进程
	if(argc<2 || strcasecmp(argv[1],"wifi")==0) {
		test_wifi_ok();
	};

	// 绑定成功后发该消息到主控进程，必须保证绑定成功后得到的推送标识的正确性
	if(argc>=2 && strcasecmp(argv[1],"bind")==0) {
		char sBindID[BUFSIZE];
		if(argc>2) {
			strcpy(sBindID,argv[2]);
		} else {
			strcpy(sBindID,"1357924680");
		};
		test_bind_ok(sBindID);
	};
	
	// 媒体点播状态发送到服务器
	if(argc>=2 && strcasecmp(argv[1],"media")==0) {
		test_media_status();
	};

	// 媒体点播状态发送到服务器
	if(argc>=2 && strcasecmp(argv[1],"alarm")==0) {
		test_alarm_file(argv[2]);
	};

	//发送升级命令给主程序
	if(argc>=2&& strcasecmp(argv[1],"upgrade")==0){
		test_update();
	}

    //发送升级命令给主程序
    if(argc>=2&& strcasecmp(argv[1],"cli")==0){
        test_systemtask();
    }

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
	if(argc>=2 && strcasecmp(argv[1],"kill")==0) {
		kill_master();
	};
}

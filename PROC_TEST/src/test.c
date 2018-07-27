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
#include "AawantData.hbak"
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
	nError = SetTraceFile((char *)"TEST",(char *)CONFIG_FILE);

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
	stHead.iRecordNum = PLAY_PROCESS_IDENTITY;
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

	// 终止系统
	if(argc>=2 && strcasecmp(argv[1],"kill")==0) {
		kill_master();
	};
}

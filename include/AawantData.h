/********************************************************************
 * NAME         : AawantData.h
 * FUNCTION     : AAWANT数据包头结构定义 
 * PROGRAMMED   : aawant
 * DATE(ORG)    : 2018/06/23
 * PROJECT      : aawant
 * OS           : LINUX
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00   2018/06/23  Created By WTQ  
 ********************************************************************/
#ifndef _AAWANTDATA_H
#define _AAWANTDATA_H

#define AAWANT_DEBUG
#ifdef AAWANT_DEBUG
    #define  CONFIG_FILE  (char *)"/data/aawant.conf"
#else
    #define  CONFIG_FILE  (char *)"/home/sine/test/config/aawant.conf"
#endif
#define  BUFSIZE            512
#define  ALARM_FIELD_SIZE    80        // 闹钟ID最大长度

// 各进程身份标识
#define  IOT_PROCESS_IDENTITY        1        //	IOT进程
#define  ALARM_PROCESS_IDENTITY        2        //  闹钟进程
#define  PLAY_PROCESS_IDENTITY        3        //  播放进程
#define  UPGRADE_PROCESS_IDENTITY   4           //升级进程
#define  MAIN_PROCESS_IDENTITY      5           //主进程

// 各进程用的信号灯键值（要保证各进程用的不同且不与系统中其他进程使用的冲突）
#define  ALARM_SEM_KEY        100        // 闹钟管理的信号灯键值
#define  IOT_SEM_KEY        101        // 闹钟管理的信号灯键值

#define URLSIZE             256
/*********************************************************************
*  数据包ID定义                                                      *
*********************************************************************/
// 各子进程 ---> 主控进程
#define PKT_CLIENT_IDENTITY        1        // 各进程发给主控进程，标识身份，包头中iRecordNum存放各进程标识值

// 音箱绑定进程 ---> 主控进程 --->  IOT
#define PKT_ROBOT_BIND_OK        2        // 音箱绑定成功

// 音箱绑定进程 ---> 主控进程  ---> IOT
#define PKT_ROBOT_WIFI_CONNECT  3        // WIFI联网成功

// 音箱绑定进程 ---> 主控进程
#define PKT_SYSTEM_SHUTDOWN        4        // 系统停止

// IOT --->  主控进程 ---> 相关子进程
#define PKT_ROBOT_WIFI_CHANGE   101        // 更换WIFI
#define PKT_FACTORY_RESET        102        // 恢复出厂设置
#define PKT_VERSION_UPDATE        103        // 版本更新信息
#define PKT_LANGUAGE_CHANGE     104        // 语言改变
#define PKT_MEDIA_ON_DEMAND        105        // 媒体点播
#define PKT_MEDIA_ACTION        106        // 媒体点播控制
#define PRK_MEDIA_PLAY_LIST        107        // 媒体点播列表

// IOT --->  主控进程 ---> IOT
#define PRK_GET_DEVICEINFO        108        // 取得设备信息

// 媒体播放进程 ---> 主控进程 ---> IOT 
#define PKT_MEDIA_STATUS        109        // 媒体点播状态

// 主控进程 ---> ALARM 
#define PKT_ALARM_SETUP            201        // 闹钟设置

// ALARM ---> 主控进程
#define PKT_ALARM_REMIND        202        // 闹钟提醒

//主控进程 ---> upgraded进程
#define PKT_UPGRADE_CTRL        203         //系统升级

//upgraded进程--->主控进程
#define PKT_UPGRADE_FEEDBACK    204         //升级信息返回

/*********************************************************************
*  数据包体定义                                                      *
*********************************************************************/
// 音箱绑定成功的推送标识
struct Robot_Binding_Data {
    char sBindID[BUFSIZE];  // 推送标识
};

//  设置同步、更换WIFI
struct Property_Iot_Data {
    int type;                                // 修改类型(1 修改名称 2 修改remained地址 3更换wifi)
    char name[BUFSIZE];                        // 音箱名称
    char wifiSsid[BUFSIZE];                    // 连接的WIFI名称
    char wifiPasswd[BUFSIZE];                // 连接的WIFI密码
    char remained[BUFSIZE];                    // remained地址
    char ackKey[BUFSIZE];                    // ackKey更换WIFI时有用
};

//  版本更新信息下发
struct UpdateInfoMsg_Iot_Data {
    int toVersion;            // 升级后版本号
    int nowVersion;        // 当前版本
    char model[BUFSIZE];    // 当前机器的机型
    char updateUrl[BUFSIZE];// 升级包URL			
    char id[BUFSIZE];        // 升级包ID 用于错误上报
};

//  语言设置
struct LanguageChange_Iot_Data {
    int language;            // 语言编号 1 国语 2 粤语
};

//  媒体点播
struct MediaOnDemand_Iot_Data {
    char ackKey[BUFSIZE];        // 确认的Key：可以确认或不确认均可
    char mediaPlayUrl[BUFSIZE];    // 播放URL
    char mediaName[BUFSIZE];    // 媒体名称
    char artist[BUFSIZE];        // 艺人
    char author[BUFSIZE];        // 作者
    int priority;                // 优先级 1 空闲时播放 2 延迟播放 3 立即播放
    long delay;                    // 延时时间(当且仅当 priority=2时，该值有效 单位 ms)
};

//  媒体点播控制
struct MediaAction_Iot_Data {
    int action;                // 播放资源的状态 , 1播放，2暂停，3上一首，4下一首，5进度条拖动
    long secondTimes;            // 资源的时长，时间单位是秒
};

//  媒体点播列表   1个MediaOnDemandUrl_Iot_Data + N个MediaPlayResult_Iot_Data
struct MediaPlayResult_Iot_Data {
    char playUrl[BUFSIZE];        // 资源播放的地址，有可能为空（百度音乐就是空的）
    char mediaName[BUFSIZE];    // 资源名称
    char artist[BUFSIZE];        // 如果是音乐数据，artist就是歌手名称，如果是儿童有生资源就是空的
};

struct MediaOnDemandUrl_Iot_Data {
    int num;        // MediaPlayResult_Iot_Data个数
    struct MediaPlayResult_Iot_Data *pData;        // 媒体数据
    int index;        // 当前用户点播的歌曲在当前页歌曲列表中的位置
};

//  媒体点播状态
struct MediaStatus_Iot_Data {
    int action;                // 播放资源的状态  1播放（上一首，下一首），2暂停
    char mediaName[BUFSIZE];    // 媒体名称
    char artist[BUFSIZE];        // 艺人
    long secondTimes;            // 资源的时长，时间单位是秒
    char imgUrl[BUFSIZE];        // 歌曲图片URL
};

// 取得设备信息
struct EquipmentRegister_Iot_Data {
    char mac[BUFSIZE];                        // 设备的MAC地址
    char name[BUFSIZE];                        // 设备名称
    char version[BUFSIZE];                    // 固件版本号
    char wifiSSID[BUFSIZE];                    // 连接的WIFI名称
    char wifiPasswd[BUFSIZE];                // 链接的WIFI密码
    char address_province[BUFSIZE];            // 设备地址_省级行政区
    char address_city[BUFSIZE];                // 设备地址_地级市
    char address_district[BUFSIZE];            // 设备地址_区县
    char address_street[BUFSIZE];            // 设备地址_街道
    char address_position[BUFSIZE];            // 设备地址_详细地址：详细地址：**省**市**
    char address_gis[BUFSIZE];                // 设备地址_经纬度
    char address_remained[BUFSIZE];            // 设备地址_区以后那段 xx路xx号xx
    char address_setting_remained[BUFSIZE];    // 设备地址_区以后那段(用户设定的) xx路xx号xx
    char phoneNumber[BUFSIZE];                // 机器内部的SIM卡的号码
    char iccid[BUFSIZE];                    // 机器内部的SIM卡的CCID
    char netType[BUFSIZE];                    // 网络类型0:wifi,1:4g
};


typedef enum {
    DOWNLOAD_START=1,
    DOWNLOAD_PAUSE,
    DOWNLOAD_CONTINUE,
    DOWNLOAD_CANCEL,
    UPGRADE_START
}UP_ACTION;

//
typedef struct TO_UPGRADE_DATA_T{
    UP_ACTION action;     //1:下载  2:暂停   3：继续  4:取消  5：升级
    char url[URLSIZE];
}TO_UPGRADE_DATA;


typedef enum {
    DOWNLOAD_SUCESS=0,
    DOWNLOAD_FAIL,
    REQUEST_UPGRADE,
    REQUEST_REBOOT
};

typedef struct FROM_UPGRADE_DATA_T{
    int status;
    int code;       //1:下载成功 2:
}FROM_UPGRADE_DATA;
// 闹钟设置的JASON数据格式
struct sJSON_Data {
    char sJsonString[BUFSIZE * 2];
};

/* 闹钟设置的JSON格式为：(智核服务器返回给主控进程的数据的featuresModel段)
{
	"status": 1,
	"features": "闹钟",									// 这个字段内容必须为“闹钟”
	"data": {
		"action": "0",									// 动作属性  0:添加  1:修改  2:查询  3:删除
		"actionDatas": [{								// 一个列表，可以包含多条数据
			"id": "7b5ac205dc05d94d1160c9944fb543e8",	// 闹钟唯一ID
			"time": "11:00",							// 24H格式
			"repeatType": "0",							// 循环类型 0:不循环 1:每天循环 2:按星期循环
			"date": "2018-07-08",						// 循环类型为0(不循环)时，内容为具体日期
														// 循环类型为1(每天循环)时，内容为第一次提醒日期
			                                            // 循环类型为2(按星期循环)时，内容为1-7,对应(星期天-星期六), 如"2"对应"星期一"等
			"event": "接人",							// 闹钟对应的事件
			"ask": "请问是否需要为您播报路线",			// 事件闹钟响起提后的二次问答
			"sentence": "我要导航"						// 事件闹钟响起提后的动作语句
		}]
	},
	"tips": "好的，明天上午11点,我会提醒你去看书"		// 提示语，操作时给客户提升
};
*/

//  闹钟提醒数据结构(由闹钟进程返回给主控进程)
struct Alarm_Remind_Data {
    char sTime[ALARM_FIELD_SIZE];    // HH:MM
    char sEvent[ALARM_FIELD_SIZE];    // 事件
    char sAsk[ALARM_FIELD_SIZE];        // ask语句
    char sSentence[ALARM_FIELD_SIZE];// 话术
};

#endif

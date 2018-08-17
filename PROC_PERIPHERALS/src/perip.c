//
// Created by sin on 18-7-29.
//


#include "keydef.h"
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
#include "sys/unistd.h"
#include "malloc.h"
#include "memory.h"

#include <sys/inotify.h>
#include <sys/poll.h>
#include <linux/input.h>
#include <dirent.h>

#include <sys/ioctl.h>

#define INPUT_DEVICE_PATH "/dev/input"
#define LED_DEVICE_PATH  "/dev/blns"


#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define LONG_PRESS_DEFAULT_DURATION   3000000     //3 seconds
#define LONG_PRESS_PERIOD   1000*100   //100 seconds
#define MONITOR_SLEEP_PERIOD   1000*50   //50 seconds
#define SEC_TO_USEC    1000000 //1s = 1000000us
#define KEY_RECORD_MASK   0xFFFF
#define INPUT_DEVICE_PATH "/dev/input"
#define BUFFER_LENGTH_L  512 //buffer length
#define BUFFER_LENGTH_S  80 //buffer length
#define NAME_ELEMENT(element) [element] = #element
#define FACTORY_RESET 249// add by yuyun0707
#define KEYPAD_DEVICE_TYPE 0x01
#define IR_DEVICE_TYPE     0x02

#ifndef EV_SYN
#define EV_SYN 0
#endif /*ifndef EV_SYN*/
#ifndef SYN_MAX
#define SYN_MAX 3
#define SYN_CNT (SYN_MAX + 1)
#endif /*ifndef SYN_MAX*/
#ifndef SYN_REPORT
#define SYN_REPORT 0
#endif /*ifndef SYN_REPORT*/
#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT 2
#endif /*ifndef SYN_MT_REPORT*/
#ifndef SYN_DROPPED
#define SYN_DROPPED 3
#endif /*ifndef SYN_DROPPED*/


int					 server_sock;		// 服务器SOCKET
static struct pollfd *ufds;
int blns_fd=0;

static struct pollfd *gufds;
static char **gdevice_names;
static int gnfds;
static int gwd;
static pthread_mutex_t gmutex;
pthread_cond_t gkeycond;
static int gKeyPressed = False;
static int gKeyLongPressed = True;
static int gKeyValueRecord = 0xFFFF;
static uint32 glongPressDuration = LONG_PRESS_DEFAULT_DURATION;

static uint8 device_type_nfds[16];

#if 1
static const char * keypad_device_name[] =
{
        "mtk-kpd",
        "mtk-pmic-keys",
        "gpio-keys",
        "jg916k"
};


/**
 *
 * @param start
 * @return
 */
int time_exceed(struct timeval start)
{
    struct timeval currentTime;
    uint32 ui8Offset = 0;
    FUNC_START

    gettimeofday(&currentTime, NULL);

    ui8Offset = (currentTime.tv_sec - start.tv_sec)*SEC_TO_USEC + (currentTime.tv_usec - start.tv_usec);
    if (ui8Offset >= LONG_PRESS_DEFAULT_DURATION)
    {
        FUNC_END
        return True;
    }
    else
    {
        FUNC_END
        return False;
    }

}


static int is_keypad_device( char *filename)
{
    int i;
    char name[PATH_MAX];
    char *strpos = NULL;

    FUNC_START
    for (i = 0; i < (int) ARRAY_SIZE(keypad_device_name); i++)
    {
        printf("check device name: %s v.s. %s \n", filename, keypad_device_name[i]);
        strpos = strcasestr(filename, keypad_device_name[i]);
        if (strpos != NULL)
        {
            return True;
        }
    }
    FUNC_END
    return False;
}


static int open_device(const char *device)
{

    int version;
    int fd;
    struct pollfd *new_ufds;
    char **new_device_names;
    char name[BUFFER_LENGTH_S];
    char location[BUFFER_LENGTH_S];
    char idstr[BUFFER_LENGTH_S];
    struct input_id id;
    int print_flags = KEY_RECORD_MASK;
    FUNC_START
            fd = open(device, O_RDWR);
    if (fd < 0)
    {
        printf("could not open %s, %s\n", device, strerror(errno));
        return -1;
    }
    if (ioctl(fd, EVIOCGVERSION, &version))
    {
        printf("could not get driver version for %s, %s\n", device, strerror(errno));
        return -1;
    }
    if (ioctl(fd, EVIOCGID, &id))
    {
        printf("could not get driver id for %s, %s\n", device, strerror(errno));
        return -1;
    }
    name[sizeof(name) - 1] = '\0';
    location[sizeof(location) - 1] = '\0';
    idstr[sizeof(idstr) - 1] = '\0';
    if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1)
    {
        name[0] = '\0';
    }
    printf("device=%s,open_device_name=%s \n", device,name);

    if (ioctl(fd, EVIOCGPHYS(sizeof(location) - 1), &location) < 1)
    {
        location[0] = '\0';
    }
    if (ioctl(fd, EVIOCGUNIQ(sizeof(idstr) - 1), &idstr) < 1)
    {
        idstr[0] = '\0';
    }
    new_ufds = (struct pollfd*)realloc(gufds, sizeof(gufds[0]) * (gnfds + 1));
    if (NULL == new_ufds)
    {
        printf("out of memory\n");
        return -1;
    }
    gufds = new_ufds;
    new_device_names = (char**)realloc(gdevice_names, sizeof(gdevice_names[0]) * (gnfds + 1));
    if (NULL == new_device_names)
    {
        printf("out of memory\n");
        return -1;
    }
    gdevice_names = new_device_names;
    gufds[gnfds].fd = fd;
    gufds[gnfds].events = POLLIN;
    gdevice_names[gnfds] = strdup(device);

    /**
     *
     */
    if (is_keypad_device(name))
    {
        device_type_nfds[gnfds] = KEYPAD_DEVICE_TYPE;
    }

    gnfds++;
    FUNC_END

    return 0;
}

int close_device(const char *device)
{
    int i;
    FUNC_START
    for (i = 1; i < gnfds; i++) {
        if (!strcmp(gdevice_names[i], device))
        {
            int count = gnfds - i - 1;
            printf(("remove device %d: %s\n", i, device));
            free(gdevice_names[i]);
            memmove(gdevice_names + i, gdevice_names + i + 1, sizeof(gdevice_names[0]) * count);
            memmove(gufds + i, gufds + i + 1, sizeof(gufds[0]) * count);
            gnfds--;
            return 0;
        }
    }
    printf("remote device: %s not found\n", device);
    FUNC_END
    return -1;
}

static int read_notify(const char *dirname, int nfd)
{
    int res;
    char devname[PATH_MAX];
    char *filename;
    char event_buf[BUFFER_LENGTH_L];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;
    FUNC_START
    res = read(nfd, event_buf, sizeof(event_buf));
    if (res < (int)sizeof(*event))
    {
        if (errno == EINTR)
        {
            return 0;
        }
        printf("could not get event, %s\n", strerror(errno));
        return 1;
    }

    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';

    while (res >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        if (event->len) {
            strcpy(filename, event->name);
            if (event->mask & IN_CREATE) {
                open_device(devname);
            }
            else {
                close_device(devname);
            }
        }
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }
    FUNC_END
    return 0;
}

/**
 * 读取/dev/input/下的设备，并打开
 */
static int scan_dir(const char *dirname)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;

    FUNC_START
            dir = opendir(dirname);
    if (NULL == dir)
    {
        return -1;
    }
    strcpy(devname, dirname);
    filename = devname + strlen(devname);
    *filename++ = '/';
    while ((de = readdir(dir)))
    {
        if ((de->d_name[0] == '.') &&
            ((de->d_name[1] == '\0') ||
             ((de->d_name[1] == '.') &&
              (de->d_name[2] == '\0'))))
        {
            continue;
        }
        strcpy(filename, de->d_name);
        printf("open_device %s\n",devname);
        open_device(devname);
    }
    closedir(dir);
    FUNC_END
    return 0;
}

/**
 * 按键长按线程处理
 * @param arg
 * @return
 */
static void *key_long_press_thread_routine(void *arg)
{
    struct timeval start;
    //int32 i4_ret = UI_OK;
    int32 i4_ret=0;
    int32 long_key_msg;
    FUNC_START
    while (1)
    {
        pthread_mutex_lock(&gmutex);
        //线程等待
        pthread_cond_wait(&gkeycond, &gmutex);

        gettimeofday(&start, NULL);

        while (gKeyPressed)
        {
            usleep(LONG_PRESS_PERIOD); //100ms

            if (time_exceed(start))
            {
                /* key long press process */
                printf("key long press process,KeyValueRecord = %d\n",gKeyValueRecord);
                gKeyLongPressed = True;
                gKeyPressed = False; //add by lei.xiao

                switch (gKeyValueRecord)
                {

                    case KEY_WAKEUP:

                        printf("KEY p long press\n");
                        break;
                    case KEY_MICMUTE:
                        AAWANTSendPacketHead(server_sock, PKT_SYSTEM_READY_NETCONFIG);
                        break;
                    default:
                        printf("key long press isn't MUTE/BLUTOOTH/POWER/VOLUME+/VOLUME-\n");
                        break;
                }

                printf("long key=%d\n", gKeyValueRecord);

                if (0 != i4_ret)
                {
                    printf("[lei]end long key to sm fail(%d)!!\n", i4_ret);
                }
            }
        }
        pthread_mutex_unlock(&gmutex);
    }
    FUNC_END

    return NULL;
}


void Long_Press_Ctrl()
{
    int ret = 0;
    pthread_attr_t attr;
    pthread_t key_long_press_thread;
    FUNC_START
    pthread_mutex_init(&gmutex, NULL);
    pthread_cond_init(&gkeycond, NULL);
    pthread_attr_init(&attr);

    ret = pthread_create(&key_long_press_thread, &attr, key_long_press_thread_routine, NULL);
    if (ret != 0)
    {
        printf("create key pthread failed.\n");
    }
    FUNC_END
}

void *key_event_monitor_thread(void *arg)
{
    FUNC_START
    int res, i, j;
    int pollres;
    const char *device = NULL;
    struct input_event event;
    int32 irbtnMsg;

    glongPressDuration = 500;
    /**
     * 开启长按键判断线程
     */
    Long_Press_Ctrl();

    gnfds = 1;
    gufds = (struct pollfd *)calloc(1, sizeof(gufds[0]));
    gufds[0].fd = inotify_init();
    gufds[0].events = POLLIN;

    /**
     *使用inotify机制监控文件或目录
     */
    gwd = inotify_add_watch(gufds[0].fd, INPUT_DEVICE_PATH, IN_DELETE | IN_CREATE);
    if (gwd < 0)
    {
        printf("could not add watch for %s, %s\n", INPUT_DEVICE_PATH, strerror(errno));
    }
    res = scan_dir(INPUT_DEVICE_PATH);
    if (res < 0)
    {
        printf("scan dir failed for %s\n", INPUT_DEVICE_PATH);
    }

    while (1)
    {
        usleep(10000);
        //pollres > 0,gufds准备好好读、写或出错状态
        pollres = poll(gufds, gnfds, -1);

        //POLLIN:普通或优先级带数据可读
        if (gufds[0].revents & POLLIN)
        {
            read_notify(INPUT_DEVICE_PATH, gufds[0].fd);
        }

        for(i = 1; i < gnfds; i++) {
            if(gufds[i].revents) {
                if(gufds[i].revents & POLLIN) {
                    res = read(gufds[i].fd, &event, sizeof(event));
                    if (res < (int) sizeof(event)) {
                        printf("could not get event\n");
                    }

                    /**
                     * 按键处理
                     */
                    if (KEYPAD_DEVICE_TYPE == device_type_nfds[i] && EV_KEY == event.type) {
                        //printf("KeyValueRecord=%d \n",KeyValueRecord);
                        /**
                         * 按键按下处理
                         */
                        if (1 == event.value)   /* key press process */
                        {
                            if ((KEY_WAKEUP == event.code || KEY_VOLUMEUP == event.code ||
                                 KEY_POWER == event.code) && KEY_RECORD_MASK == gKeyValueRecord) {
                                mprintf("[%s]==>key code=%d,value%d\n", __FUNCTION__, event.code, event.value);
//                              printf("<user_interface> key code = %d (%s), value = %d \n",
//                                     event.code, codename(event.type, event.code), event.value);
                                gKeyPressed = True;
                                gKeyLongPressed = False;  /* long press flag, handle in the function key_long_press_thread_routine */
                                gKeyValueRecord = event.code;
                                pthread_cond_signal(&gkeycond);/*trigger long key thread to start time record*/
                            } else if (KEY_A == event.code || KEY_B == event.code ||
                                       KEY_C == event.code || KEY_D == event.code ||
                                       KEY_E == event.code || KEY_F == event.code ||
                                       KEY_G == event.code || KEY_H == event.code ||
                                       KEY_I == event.code || KEY_J == event.code ||
                                       KEY_J == event.code || KEY_K == event.code ||
                                       KEY_L == event.code) {

                            }

                        } else    /* 按键释放处理 */
                        {
                            /*so far, only handle the case of one key press, if other key release, don't care it*/
                            if (gKeyValueRecord == event.code) {
                                //                             printf("<user_interface> key code = %d (%s), value = %d \n",
//                              event.code, codename(event.type, event.code), event.value);

                                printf(" KEY released \n");
                                gKeyPressed = False;    /* long press flag clear */
                                gKeyValueRecord = KEY_RECORD_MASK;
                                if (!gKeyLongPressed) {

                                }
                            }
                        }
                    }
                }
            }
        }
    }
    FUNC_END
}


void SendKey(int value){
    PacketHead stPacketHead;

    memset(&stPacketHead, 0, sizeof(PacketHead));
    stPacketHead.iPacketID = PKT_SYSTEM_TB_KEY_VALUE;
    stPacketHead.lPacketSize = sizeof(PacketHead);
    stPacketHead.iRecordNum =(short) value;

    if (AAWANTSendPacket(server_sock, (char *) &stPacketHead) < 0) {
        //AIcom_SetErrorMsg(ERROR_SOCKET_WRITE, NULL, NULL);
        printf("Send packet err,server_sock=%d\n",server_sock);
        // return -1;
    };
}

void *KeyEventMonitorThread2(void *arg)
{
    FUNC_START
    int res, i, j;
    int pollres;
    const char *device = NULL;
    struct input_event event;
    int32 irbtnMsg;
   // int server_sock;


    glongPressDuration = 500;
    /**
     * 开启长按键判断线程
     */
    Long_Press_Ctrl();

    /**
     * 手势判断
     */
    //Gesture_Press_Ctrl();

    gnfds = 1;
    gufds = (struct pollfd *)calloc(1, sizeof(gufds[0]));
    gufds[0].fd = inotify_init();
    gufds[0].events = POLLIN;

    /**
     *使用inotify机制监控文件或目录
     */
    gwd = inotify_add_watch(gufds[0].fd, INPUT_DEVICE_PATH, IN_DELETE | IN_CREATE);
    if (gwd < 0)
    {
        printf("could not add watch for %s, %s\n", INPUT_DEVICE_PATH, strerror(errno));
    }
    res = scan_dir(INPUT_DEVICE_PATH);
    if (res < 0)
    {
        printf("scan dir failed for %s\n", INPUT_DEVICE_PATH);
    }

    while (1)
    {
        //pollres > 0,gufds准备好好读、写或出错状态
        pollres = poll(gufds, gnfds, -1);

        //POLLIN:普通或优先级带数据可读
        if (gufds[0].revents & POLLIN)
        {
            read_notify(INPUT_DEVICE_PATH, gufds[0].fd);
        }

        for (i = 1; i < gnfds; i++)
        {
            if (gufds[i].revents)
            {
                if (gufds[i].revents & POLLIN)
                {
                    res = read(gufds[i].fd, &event, sizeof(event));
                    if (res < (int)sizeof(event))
                    {
                        printf("could not get event\n");
                    }
                    /**
                     * 按键处理
                     */
                    if (KEYPAD_DEVICE_TYPE == device_type_nfds[i] && EV_KEY == event.type)
                    {
                        /**
                         * 按键按下处理
                         * 248:对应唤醒键==>KEY_MICMUTE
                         *
                         */
                        if (1 == event.value)   /* key press process */
                        {
                            //mprintf("[%s]==>key press: code=%d,value=%d\n",__FUNCTION__,event.code,event.value);
                            if ((KEY_MICMUTE == event.code ||
                                 KEY_POWER == event.code) && KEY_RECORD_MASK == gKeyValueRecord)
                            {
                                mprintf("[%s]==>key code=%d,value%d\n",__FUNCTION__,event.code,event.value);

                                gKeyPressed = True;
                                gKeyLongPressed = False;  /* long press flag, handle in the function key_long_press_thread_routine */
                                gKeyValueRecord = event.code;
                                pthread_cond_signal(&gkeycond);/*trigger long key thread to start time record*/
                            } else if(KEY_A==event.code||KEY_B==event.code||
                                      KEY_C==event.code||KEY_D==event.code||
                                      KEY_E==event.code||KEY_F==event.code||
                                      KEY_G==event.code||KEY_H==event.code||
                                      KEY_I==event.code||KEY_J==event.code||
                                      KEY_J==event.code||KEY_K==event.code||
                                      KEY_L==event.code)
                            {
                                gKeyPressed = True;
                                gKeyValueRecord = event.code;
                            }

                        }
                        else    /* event.value=0,按键释放处理 */
                        {
                            mprintf("[%s]==>key press: code=%d,value=%d\n",__FUNCTION__,event.code,event.value);
                            /*so far, only handle the case of one key press, if other key release, don't care it*/
                            if (KEY_MICMUTE == event.code)
                            {
                                printf(" KEY released \n");
                                gKeyPressed = False;    /* long press flag clear */
                                gKeyValueRecord = KEY_RECORD_MASK;
                                /**
                                 * 长按线程只是改变gKeyPressed，gKeyLongPressed的值
                                 * 事件的处理在按键释放的时候触发，在触发的时候，时间达到了
                                 * 长按的设定则判断为长按，否则判断为短按
                                 */
                                if (gKeyLongPressed)
                                {
                                    //长按联网配置
                                    printf("long press\n");
                                   // AAWANTSendPacketHead(server_sock, PKT_SYSTEM_READY_NETCONFIG);
                                } else{
                                    //短按唤醒
                                    printf("short press,server_sock=%d\n",server_sock);
                                    AAWANTSendPacketHead(server_sock, PKT_SYSTEM_WAKEUP);
                                }
                                //触控暂时按发键值来处理，后续不排除按键趋势算法这边做
                            }else if(KEY_A==event.code||KEY_B==event.code||
                                     KEY_C==event.code||KEY_D==event.code||
                                     KEY_E==event.code||KEY_F==event.code||
                                     KEY_G==event.code||KEY_H==event.code||
                                     KEY_I==event.code||KEY_J==event.code||
                                     KEY_K==event.code||KEY_L==event.code)
                            {
                                mprintf("[%s]==>key release: code=%d,value=%d\n",__FUNCTION__,event.code,event.value);
                                SendKey(event.code);
                            }
                        }
                    }
                }
            }
        }
    }
    FUNC_END
}

int create_KeyThread(){
    pthread_t keythread;
    FUNC_START
    int ret=pthread_create(&keythread,NULL,KeyEventMonitorThread2,NULL);
    if(ret){
        printf("creat key thread fail\n");
    }
    FUNC_END
    return 0;
}

#endif

int write_blns(int fd, int data)
{
    int ret = 0;
    if(fd < 0 )
    {
        printf("OPEN DEVICE FAIL...\n");
        return -errno;
    }

#ifdef DEBUG_
    printf("write blns data -> %d", data);
#endif
    ret = write(fd, &data, sizeof(int));
    if(ret < 0)
    {
        printf("WRITE DATA FAIL...");
        return -errno;
    }

    return ret;
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
    //SetTraceFile((char *)"KEYEVENT",(char *)CONFIG_FILE);

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



    // 初始化本程序中重要的变量
    timeout_select.tv_sec = 10;
    timeout_select.tv_usec = 0;

    blns_fd=open(LED_DEVICE_PATH,O_RDWR);
    if(blns_fd<0) {
        printf("open blns failed\n");
    } else{
        printf("blns fd=%d\n",blns_fd);
    }
    create_KeyThread();
    Long_Press_Ctrl();

    for(;;) {
        if(server_sock==NULL){
            printf("server_sock is null\n");
        }
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
                return -1;
            };

            PacketHead *pHead = (PacketHead *) lpInBuffer;
            switch (pHead->iPacketID) {
                    //系统状态灯光
                case PKT_BLNS_SYSTEM_STATUS: {
                    printf("Get PKT_BLNS_SYSTEM_STATUS \n");

                    System_Blns_Status blns;
                    int cmd;
                    int sw = 0;
                    //blns = (System_Blns_Status *) (lpInBuffer + sizeof(PacketHead));
                    blns = (System_Blns_Status ) pHead->iRecordNum;
                    //printf("Get PKT_BLNS_VALUE_STATUS =%d\n", blns);
                    if (blns == BLNS_ERROR_STATUS) {
                        cmd = AW9523B_IO_ERROR;
                    } else if (blns == BLNS_VOLUME_STATUS) {
                        cmd = AW9523B_IO_VOLUME;
                    } else if (blns == BLNS_OFF_STATUS) {
                        cmd = AW9523B_IO_OFF;
                    } else if (blns == BLNS_STARTUP_STATUS) {
                        cmd = AW9523B_IO_START_UP;
                    } else if (blns == BLNS_WAKEUP_STATUS) {
                        cmd = AW9523B_IO_WAKEUP;
                    } else if (blns == BLNS_GET_SERVER_INFO_STATUS) {
                        cmd = AW9523B_IO_GIFS;
                    } else if (blns == BLNS_BROADCAST_STATUS) {
                        cmd = AW9523B_IO_BROADCAST;
                    } else if (blns == BLNS_PLAY_STATUS) {
                        cmd = AW9523B_IO_PLAY;
                    } else if (blns == BLNS_NET_CONFIG_STATUS) {
                        printf("BLNS_NET_CONFIG_STATUS\n");
                        cmd = AW9523B_IO_NETCONFIG;
                    } else if (blns == BLNS_UPDATE_STATUS) {
                        printf("BLNS_UPDATE_STATUS\n");
                        cmd = AW9523B_IO_UPDATE;
                    } else if (blns == BLNS_TEST_STATUS) {
                        printf("BLNS_TEST_STATUS\n");
                        cmd = AW9523B_IO_TEST;
                    } else if (blns == BLNS_SWITCH_PLAY_STATUS) {
                        printf("BLNS_SWITCH_PLAY_STATUS\n");
                        cmd = AW9523B_IO_SWITCH_PLAY;
                    }

                    if (ioctl(blns_fd, cmd, &sw) < 0) {
                        printf("IOCTL DATA FAIL...\n");
                    }

                    break;
                }
                    //音量控制灯光
                case PKT_BLNS_VALUE_STATUS: {
                    printf("Get PKT_BLNS_VALUE_STATUS \n");

                    int led=0;
                    led = (int ) pHead->iRecordNum;
                    write_blns(blns_fd,led);


                }
                    break;

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


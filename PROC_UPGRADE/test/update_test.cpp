//
// Created by sine on 18-7-11.
//
#include <../include/common.h>
#include <../include/upg_download.h>
#include <fcntl.h>
#include <include/AI_PKTHEAD.h>
#include "HttpClient.h"
#include "AIprofile.h"
#include "AawantData.h"
//#include "string"
#include "string.h"
#include "systool.h"
#include "cJSON.h"

#define Dprintf(x) printf("[%s:%d]x",__FUNCTION__,__LINE__,x)
//#define Aprintf(x) Dprintf(__FUNCTION__,__LINE__,x)
//using namespace std;


#define REPORT_DOWNLOAD_PATH  "www.aawant.com/speaker/0.0.1/release/equipment/afterDownload"
#define REPORT_UPGRADE_PATH "www.aawant.com/speaker/0.0.1/release/EquipmentUpdateReportWebServlet"

const char *A_REPORT_DOWNLOAD_PATH=REPORT_DOWNLOAD_PATH;
const char *A_REPORT_UPGRADE_PATH=REPORT_UPGRADE_PATH;

void test() {
    Dprintf("hello\n");
}

long GetCurrentTime(){
    struct timeval currentTime;
    long ms = 0;
    gettimeofday(&currentTime, NULL);
    ms = currentTime.tv_sec*1000000  + (currentTime.tv_usec );
    return ms;
}

typedef struct UpgradeReport_T{

    char mac[20];
    long time_t;
    char info[2];
    int type;
    int toVersion;
    int nowVersion;
    char model[256];
    char updateUrl[256];
    char id[10];
    int ids;

}UpgradeReport;

UpgradeReport upgReport;
UpgradeReport oldUpgReport;
UpgradeReport newUpgReport;



int COnWriteData(void* buffer, size_t size, size_t nmemb, char * useless)
{
    char value[4096] = {0};
    char htvalue[4096] = {0};
    char *v = NULL;

    memcpy(value, (char *)buffer, size*nmemb);
    printf("COnWriteData==>%s\n", value);
    /*
    v = strstr(value, "\"data\"");
    if (NULL != v)
    {
        memcpy(htvalue, "{", 1);
        strcat(htvalue, v);
        //printf("-------%s\n", htvalue);
    }
    else
    {

    }
    */
    return 0;
}

int CGet(const char *url, char * file)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    /*
    if(m_bDebug)
    {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
     */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

    if(NULL!=file
            ){
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)file);
    } else{
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, COnWriteData);
    }

    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 4);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 4);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

int CPost(const char *url, const char *data , char *file)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    /*
    if(m_bDebug)
    {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
    */
    curl_easy_setopt(curl, CURLOPT_URL, url);//url地址
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);//post参数
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

    /**
     * 如果不想用回调函数而保存数据，那么可以使用 CURLOPT_WRITEDATA 选项，使用该选项时，
     * 函数的第 3 个参数必须是个 FILE 指针，函数会将接收到的数据自动的写到这个 FILE 指针所指向的文件流中。
     */
    if(NULL!=file){
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)file);
    } else{
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, COnWriteData);//接收的数据
    }

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}

void ReportDownloadResult(){

    char strUrl[256];
    char *data="?mac=";
    char strResponse[256];
    char mac[17];
    memset(strUrl,0,sizeof(strUrl));
    strcpy(strUrl,A_REPORT_DOWNLOAD_PATH);
    strcat(strUrl,data);
    printf("get==>%s\n",strUrl);
    GetMac(mac,"wlan0");
    strcat(strUrl,mac);
    printf("url==>%s\n",strUrl);
    CGet(strUrl,NULL);
}


//void ReportUpgradeResult(char *mac,int time,int type,int toVersion,
//                         int nowVersion,char *model,char *updateUrl,char *id,int ids){
void ReportUpgradeResult(UpgradeReport *rp){

    char json[4096];
    char data[4096];
   // char Mac[20];

    cJSON *root=cJSON_CreateObject();

    cJSON_AddItemToObject(root, "mac", cJSON_CreateString(rp->mac));
    cJSON_AddItemToObject(root,"time",cJSON_CreateNumber(rp->time_t));
    cJSON_AddItemToObject(root,"info",cJSON_CreateString(rp->info));
    cJSON_AddItemToObject(root,"type",cJSON_CreateNumber(rp->type));
    cJSON_AddItemToObject(root,"toVersion",cJSON_CreateNumber(rp->toVersion));
    cJSON_AddItemToObject(root,"nowVersion",cJSON_CreateNumber(rp->nowVersion));
    cJSON_AddItemToObject(root,"model",cJSON_CreateString(rp->model));
    cJSON_AddItemToObject(root,"updateUrl",cJSON_CreateString(rp->updateUrl));
    cJSON_AddItemToObject(root,"id",cJSON_CreateString(rp->id));
    cJSON_AddItemToObject(root,"ids",cJSON_CreateNumber(rp->ids));



    printf("%s\n", cJSON_PrintUnformatted(root));
    /*
    memset(data,0,sizeof(data));
    strcpy(data,A_REPORT_UPGRADE_PATH);
    strcat(data,"?data=");
    strcat(data,cJSON_PrintUnformatted(root));
    printf("data==>%s\n",data);
    */

    CPost(rp->updateUrl,cJSON_PrintUnformatted(root),NULL);

}

int GetUpgradeInfo(){

    char *oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "mac",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    strcpy(oldUpgReport.mac,oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "time",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    // strcpy(oldUpgReport.mac,oldMsg);
    oldUpgReport.time_t=atoi(oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "info",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    strcpy(oldUpgReport.info,oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "type",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    //strcpy(oldUpgReport.type,oldMsg);
    oldUpgReport.type=atoi(oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "toVersion",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    //strcpy(oldUpgReport.toVersion,oldMsg);
    oldUpgReport.toVersion=atoi(oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "nowVersion",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    //strcpy(oldUpgReport.mac,oldMsg);
    oldUpgReport.nowVersion=atoi(oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "model",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    strcpy(oldUpgReport.model,oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "updateUrl",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    strcpy(oldUpgReport.updateUrl,oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "id",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    strcpy(oldUpgReport.id,oldMsg);

    oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "ids",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };
    //strcpy(oldUpgReport.ids,oldMsg);
    oldUpgReport.ids=atoi(oldMsg);



    printf("mac=%s\n",oldUpgReport.mac);
    printf("time=%ld\n",oldUpgReport.time_t);
    printf("info=%s\n",oldUpgReport.info);
    printf("type=%d\n",oldUpgReport.type);
    printf("ids=%d\n",oldUpgReport.ids);
    printf("model=%s\n",oldUpgReport.model);
    printf("id=%s\n",oldUpgReport.id);
    printf("nowVersion=%d\n",oldUpgReport.nowVersion);
    printf("toVersion=%d\n",oldUpgReport.toVersion);
    printf("updateUrl=%s\n",oldUpgReport.updateUrl);

}


#if 0
int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    if(m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}



#endif





/**
 * 上报升级结果
 * {"mac":"xxxx","time":1300000000000,"info":"","type":1,"toVersion":2,"nowVersion":1,
 * "model":"mtk6735","updateUrl":"http://www.aawant.com/xxxx.zip","id":"xxxxxx","ids":1}
 */

/**
 *
 * @return 1:有升级; 0:没升级
 */
int CheckUpgradeResult() {

    UpgradeReport report;
    char oldver[10];
    char nowver[10];
    int old;
    int now;
    char *oldMsg = AIcom_GetConfigString((char *) "UPDATE", (char *) "Version",(char *) UPDATE_FILE);
    if (oldMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", UPDATE_FILE);
        return (AI_NG);
    };

    snprintf(oldver,10,oldMsg);
    printf("oldver = %s\n",oldver);

    char *nowMsg = AIcom_GetConfigString((char *) "Config", (char *) "Version",(char *) CONFIG_FILE);
    if (nowMsg == NULL) {
        printf("Fail to get Version Info in %s!\n", CONFIG_FILE);
        return (AI_NG);
    };
    //strcpy(sService, sMsg);
    strcpy(nowver,oldMsg);
    printf("nowver = %d\n",atoi(oldver));
    if(atoi(nowver)>atoi(oldver)){
        printf("No Upgrage\n");
        return 1;
    }

    return 0;

}

int WriteUpgradeReportToFile(UpgradeReport *np,char *path){
    int fd;
    char buf[512];
    int len=-1;
    char ver[10];

    char *currentMsg = AIcom_GetConfigString((char *) "Config", (char *) "Version",(char *) CONFIG_FILE);
    if (currentMsg == NULL) {
        printf("Fail to get Version info: %s!\n", CONFIG_FILE);
        return (AI_NG);
    };

    strcpy(ver,currentMsg);
    printf("Current Version %s\n",ver);

    fd=open(path,O_CREAT|O_TRUNC|O_RDWR);
    if(-1==fd){
        printf("Cannot open update.conf\n");
        return -1;
    }

    snprintf(buf,512,"[UPDATE]\nVersion=%s\nmac=%s\ntime=%d\ninfo=%s\ntype=%d\ntoVersion=%d\nnowVersion=%d\n"
                      "model=%s\nupdateUrl=%s\nid=%s\nids=%d\n",ver,np->mac,np->time_t,np->info,np->type,
    np->toVersion,np->nowVersion,np->model,np->updateUrl,np->id,np->ids);
    len=strlen(buf);
    printf("buf len = %d\n",len);
    printf("buf =%s\n",buf);

    if(write(fd,buf,len)!=len){
        printf("Cannot open update.conf\n");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;

}

int main() {
#define myurl "http://192.168.1.118/update.bin"
    int a = 4;
    int dl_lenth;
    int ctime;
    int ret;
    //test();
   // ReportDownloadResult();

    memset(&upgReport,0,sizeof(upgReport));

    GetMac(upgReport.mac,"wlan0");

    upgReport.time_t=GetCurrentTime();
    strcpy(upgReport.info,"");
    upgReport.type=0;
    upgReport.ids=1;
    strcpy(upgReport.model,"mt8516");
    strcpy(upgReport.id,"1");
    strcpy(upgReport.updateUrl,REPORT_UPGRADE_PATH);
    upgReport.nowVersion=0;
    upgReport.toVersion=1;

    /*
    printf("mac=%s\n",upgReport.mac);
    printf("time=%ld\n",upgReport.time_t);
    printf("info=%s\n",upgReport.info);
    printf("type=%d\n",upgReport.type);
    printf("ids=%d\n",upgReport.ids);
    printf("model=%s\n",upgReport.model);
    printf("id=%s\n",upgReport.id);
    printf("nowVersion=%d\n",upgReport.nowVersion);
    printf("toVersion=%d\n",upgReport.toVersion);
    printf("updateUrl=%s\n",upgReport.updateUrl);
    */
   // ReportUpgradeResult(&upgReport);
    WriteUpgradeReportToFile(&upgReport,"/data/etc/update.conf");
    ret=CheckUpgradeResult();
    if(ret){
        printf("Have new ver\n");
    }

    GetUpgradeInfo();

    /*
    switch(a) {
        case 1:
            _upgrade_start_download_firmware(UPDATE_FROM_NET,"http://192.168.1.118/update.bin", "/tmp/");
            break;
        case 2:
            _upgrade_start_download_firmware("http://192.168.1.118/update.bin", "/tmp/");
            break;
        case 3:
            dl_lenth = _upgrade_get_download_file_lenth(NULL, myurl);
            if ((dl_lenth > 0) )
            {
                printf("file size from id:%d.\n",dl_lenth);
            }
            break;
        case 4:
            Aawant_StartDownLoad("http://192.168.1.118/","/home/sine/download",True);
            break;
        case 5:
            Aawant_StartDownLoad("http://192.168.1.118/","/home/sine/download",False);
            break;


    }
*/
    //_upgrade_download_param_basic_init()
    /*
        while (1){
            sleep(10000);
        }
*/

}

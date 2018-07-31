//
// Created by sine on 18-7-11.
//
#include <../include/common.h>
#include <../include/upg_download.h>
#include "HttpClient.h"
#include "string"

#define Dprintf(x) printf("[%s:%d]x",__FUNCTION__,__LINE__,x)
//#define Aprintf(x) Dprintf(__FUNCTION__,__LINE__,x)
using namespace std;
void test() {
    Dprintf("hello\n");
}

int GetCurrentTime(){
    struct timeval currentTime;
    uint32 ms = 0;
    gettimeofday(&currentTime, NULL);
    ms = currentTime.tv_sec*1000000  + (currentTime.tv_usec );
    return ms;
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
int main() {
#define myurl "http://192.168.1.118/update.bin"
    int a = 4;
    int dl_lenth;
    test();
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
}

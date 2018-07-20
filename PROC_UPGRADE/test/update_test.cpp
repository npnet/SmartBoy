//
// Created by sine on 18-7-11.
//
#include <../include/common.h>
#include <../include/upg_download.h>

#define Dprintf(x) printf("[%s:%d]x",__FUNCTION__,__LINE__,x)
//#define Aprintf(x) Dprintf(__FUNCTION__,__LINE__,x)

void test() {
    Dprintf("hello\n");
}

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

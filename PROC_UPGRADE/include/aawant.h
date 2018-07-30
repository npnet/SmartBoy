//
// Created by sin on 18-7-14.
//

#ifndef SMARTBOY_AAWANT_H
#define SMARTBOY_AAWANT_H

#include "common.h"

#define PRINTF_DEBUG
#ifdef PRINTF_DEBUG
#define mprintf(...)  printf(__VA_ARGS__)
#else
#define  mprintf(...)
#endif

/* define value*/
#define UPGRADE_IMG_NUM             (4)                  /* assuming only 4 images to upgrade */
#define UPG_CMD_LEN                 (255)
#define UPG_MAX_FILE_PATH_LEN       (255)
#define CURL_DOWNLOAD_TIMEOUT       (10*60)

#define NETWORK_UPGRADE_BASE_URL    "http://192.168.1.118/"


//#define SINE_DEBUG
#ifdef SINE_DEBUG
/* for id file download */
#define UPGRADE_ID_FILE_NAME        "upg_id_file.bin"
#define UPGRADE_ID_FILE_SAVE_PATH   "/home/sine/download"
#define UPGRADE_ID_FILE_TMP_PATH    "/tmp/"

/* for sectionally package download */
#define UPGRADE_OTA_FILE_NAME       "update.bin"
#define UPGRADE_OTA_FILE_SAVE_PATH  "/tmp/update/"

/* for full update package download */
#define UPGRADE_FULL_PKG_NAME       "update.zip"
#define UPGRADE_FULL_PKG_SAVE_PATH  "/home/sine/download/"

#define UPGRADE_DOWNLOAD_RESUME_TIMES           (60)        /* try 60 times, about 1 min if connection timeout */
#define UPGRADE_TRANSFER_FLASH_SIZE             (1<<21)     /*  transfer around 2MB data to upg_app for flashing every times */

#else
/* for id file download */
#define UPGRADE_ID_FILE_NAME        "upg_id_file.bin"
#define UPGRADE_ID_FILE_SAVE_PATH   "/tmp/"
#define UPGRADE_ID_FILE_TMP_PATH    "/tmp/"

/* for sectionally package download */
#define UPGRADE_OTA_FILE_NAME       "update.bin"
#define UPGRADE_OTA_FILE_SAVE_PATH  "/tmp/update/"

/* for full update package download */
#define UPGRADE_FULL_PKG_NAME       "update.zip"
#define UPGRADE_FULL_PKG_SAVE_PATH  "/tmp/"

#define UPGRADE_DOWNLOAD_RESUME_TIMES           (60)        /* try 60 times, about 1 min if connection timeout */
#define UPGRADE_TRANSFER_FLASH_SIZE             (1<<21)     /*  transfer around 2MB data to upg_app for flashing every times */

#endif


/*enum */
typedef enum {
    E_UPG_CONTROL_UPGRADE_STATUS_INITED = 0,
    E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING,
    E_UPG_CONTROL_UPGRADE_STATUS_DONE,
    E_UPG_CONTROL_UPGRADE_STATUS_FAILED,
    E_UPG_CONTROL_UPGRADE_STATUS_CANCELLED,
    E_UPG_CONTROL_UPGRADE_STATUS_MAX
} E_UPG_CONTROL_UPGRADE_STATUS;

typedef enum {
    E_UPG_CONTROL_CHECK_UPDATE,
    E_UPG_CONTROL_MSG_MAX
} E_UPG_CONTROL_TIMER_MSG;

typedef enum {
    E_UPG_CONTROL_DOWNLOAD_FW = 0,
    E_UPG_CONTROL_FLASH_IMG_DATA,
    E_UPG_CONTROL_FLASH_IMG_DONE,
    E_UPG_CONTROL_STATE_MNGR_MSG_MAX
} E_UPG_CONTROL_MSG_TYPE;


typedef enum {
    UPDATE_FROM_CARD = 0x00,
    UPDATE_FROM_NET = 0x01,
    UPDATE_FROM_TEMP = 0x02,
    UPDATE_GET_OTA_INFO = 0X03
} UPDATE_TYPE;

typedef struct zip_header_raw {
    char sign_head[4];
    char version[2];
    char gp_bit_flag[2];
    char comp_method[2];
    char mod_time[2];
    char mod_date[2];
    char crc32[4];
    char comp_size[4];
    char uncomp_size[4];
    char name_length[2];
    char ext_field_length[2];
    char file_name;
} ZIP_HEADER_RAW;

typedef struct zip_header {
    int32 sign_head;
    int16 version;
    int16 gp_bit_flag;
    int16 comp_method;
    int16 mod_time;
    int16 mod_date;
    int32 crc32;
    int32 comp_size;
    int32 uncomp_size;
    int16 name_length;
    int16 ext_field_length;
    char file_name[64];
} ZIP_HEADER;

typedef struct UPGRADE_FILE_INFO_T {
    boolean is_force_update;
    int32 upgrade_file_version;
    int32 upgrade_file_size;
    boolean is_low_mem_upg;
    char url[256];
    char save_path[256];
    uchar fgfs_ubi;
} UPGRADE_FILE_INFO;

typedef struct UPGRADE_IMAGE_INFO_T {
    ZIP_HEADER img_header;
    int32 img_data_offset;
    int32 img_blk_size;
    int32 img_flash_unit_size;
    boolean is_flash_unit_done;
    int32 img_downloaded_size;
    boolean is_img_download_done;
    int32 img_flashed_size;
    boolean is_img_flashed_done;

} UPGRADE_IMAGE_INFO;




typedef enum AAWANT_UPG_CTL_STATUS_T {
    AAW_CTL_DOWNLOAD_INIT = 0,
    AAW_CTL_DOWNLOAD_SUCESS,
    AAW_CTL_DOWNLOAD_FAIL,
    AAW_CTL_DOWNLOAD_DOING,
    AAW_CTL_DOWNLOAD_CANCEL,
    AAW_CTL_DOWNLOAD_PAUSE,
    AAW_CTL_UPGRADE_DOING,
    AAW_CTL_UPGRADE_SUCESS,
    AAW_CTL_UPGRADE_FAIL,
    AAW_CTL_UPGRADE_CANCEL

} AAWANT_UPG_CTL_STATUS;

typedef struct AAWANT_DOWNLOAD_PARAM_T {
    char url[256];
    char save_path[256];
    boolean is_full_pkg_update;
    int32 upg_file_size;
    FILE *fp;
    CURL *curl;
    boolean is_curl_inited;
    CURLcode errcode;
    char curl_error_buf[CURL_ERROR_SIZE];
    boolean is_head_info;
    int32 requested_size;
    int32 downloaded_size;
    char *conntent_buf;
    boolean is_request_done;
    boolean is_continue;
    pthread_cond_t flash_cond;
    pthread_mutex_t flash_mutex;
    pthread_mutex_t status_mutex;//升级状态锁
    pthread_cond_t status_cond;
    pthread_mutex_t action_mutex;
    pthread_cond_t  action_cond;

    pthread_mutex_t intent_mutex;
    pthread_cond_t  intent_cond;

    char img_num;
    char processing_img_idx;
    UPGRADE_IMAGE_INFO image[UPGRADE_IMG_NUM];
    int dl_sock;
    E_UPG_CONTROL_UPGRADE_STATUS status;
    //AAWANT_UPG_CTL_STATUS status;
} DOWNLOAD_PARAM;



typedef struct UPDATA_VERSION_T{
    int nowVersion;
    int toVersion;
    char url[256];
    char model[256];
}UPDATA_VERSION;



typedef enum {
    UPG_START=1,
    UPG_PAUSE,
    UPG_CONTINUE,
    UPG_CANCEL,

}UPG_ACTION;

typedef enum{
   // INTENT_START=0,
    INTENT_PAUSE=1,
    INTENT_CONTINUE,
    INTENT_CANCEL,
}WantMeToDo;

//
typedef struct TO_UPGRADE_DATA_T{
    UPG_ACTION action;
    char url[256];
}TO_UPGRADE_DATA;


typedef enum {
    DOWNLOAD_SUCESS=0,
    DOWNLOAD_INIT_FAIL,
    DOWNLOAD_FAIL,
    DOWNLOAD_FINISH_AND_REQUEST_UPGRADE,
    REQUEST_UPGRADE,
    REQUEST_FLASH,
    REQUEST_REBOOT,
    UPGRADE_FAIL,
    UPGRADE_FINISH_AND_REQUEST_REBOOT
}UPGRADE_STATUS;

typedef struct FROM_UPGRADE_DATA_T{
    UPGRADE_STATUS status;
    int code;       //-1:下载失败,0:下载成功
}FROM_UPGRADE_DATA;




extern DOWNLOAD_PARAM a_dl_param;
extern E_UPG_CONTROL_UPGRADE_STATUS a_status;
extern AAWANT_UPG_CTL_STATUS aa_status;
extern UPG_ACTION up_action;
extern WantMeToDo mpCmd;
extern void SetMainProcessIntent(DOWNLOAD_PARAM *dl,WantMeToDo intent);
extern WantMeToDo GetMainProcessIntent();


#define FunctionStart printf("----------------[%s][Start]----------------\n", __FUNCTION__);
#define FunctionEnd   printf("-----------------[%s][End]-----------------\n", __FUNCTION__);
#endif //SMARTBOY_AAWANT_H

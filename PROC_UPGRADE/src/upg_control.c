/**
 * 目前升级方式分两种：
 * 1-全包烧写，整个烧写镜像（打包成zip包）下载下来，进行烧写
 * 2-分段烧写，低内存的时候使用，每次下载2M，烧写进系统后，然后继续下载，烧写，如此循环
 *   直到整个系统完成升级完成
 * 考虑到，分段烧写过程复杂，担心断电的各种情况引起不可预知的情况，现在默认为第一种烧写，
 * 第二种暂时做测试用
 *
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <AawantData.h>
#include <AIUComm.h>

/* private */
#include "upg_control.h"
#include "upg_download.h"

static uint8 _g_check_times = 0;

#define BUF_SIZE  256
#define BUF_SIZE_HALF 128
#define UPG_START_TIME 4
#define UPG_END_TIME 6


/**
 * 调用第三方app写入数据
 * @param imginfo
 * @param unflashed_size
 * @param is_last_flash_unit
 * @return
 */

int32 AawantCmd_Call_App_To_Flash_Data(UPGRADE_IMAGE_INFO *imginfo, int32 unflashed_size, boolean is_last_flash_unit) {
    int32 i4_ret = 0;
    UPGRADE_IMAGE_INFO *img_info = NULL;
    char *flash_img_name = NULL;
    int32 flash_offset = 0;
    boolean is_all_img_flashed = False;
    char upg_app_cmd[BUF_SIZE_HALF] = {0};

    img_info = imginfo;
    flash_img_name = img_info->img_header.file_name;
    flash_offset = img_info->img_flashed_size;

    snprintf(upg_app_cmd, sizeof(upg_app_cmd), "upgrade_app %s %d %d %d", flash_img_name, flash_offset, unflashed_size,
             is_last_flash_unit);
    printf("[%s]==>call upg app: %s\n", __FUNCTION__, upg_app_cmd);
    i4_ret = system(upg_app_cmd);
    printf("[%s]==>upgrade_app return: %d\n", __FUNCTION__, i4_ret);

    return i4_ret;
}

/**
 *
 * @param dl_param
 * @return
 */
int32 Aawant_Fully_Flash_ImgData(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    char upg_app_cmd[BUF_SIZE_HALF] = {0};
#define PATH "/tmp/update"

    snprintf(upg_app_cmd, sizeof(upg_app_cmd), "upgrade_app %s", dl_param->save_path);
    printf("call upg app: %s\n", upg_app_cmd);

    if (0 == access(PATH, F_OK)) {

        if (0 == remove(PATH)) {
            printf("remove existing %s \n", PATH);
        } else {
            printf("remove existing %s failed\n", PATH);
            return -1;
        }
    }
    i4_ret = system(upg_app_cmd);
    printf("[%s]==>upgrade_app return: %d\n", __FUNCTION__, i4_ret);

    //  Aawant_Wakeup_Data_Flash_Done(dl_param, &dl_param->is_request_done);
    return i4_ret;
}

/**
 * 分段烧写
 * @param dl_param
 * @return
 */
int32 Aawant_Sectionally_Flash_ImgData(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    UPGRADE_IMAGE_INFO *img_info = NULL;
    int32 flash_unit_size = 0;
    int32 unflashed_size = 0;
    boolean is_img_download_done = False;
    boolean is_all_img_flashed = False;

    img_info = &(dl_param->image[dl_param->processing_img_idx]);
    flash_unit_size = img_info->img_flash_unit_size;
    is_img_download_done = img_info->is_img_download_done;
    unflashed_size = img_info->img_downloaded_size - img_info->img_flashed_size;

    if (unflashed_size < flash_unit_size) {
        /* last data unit to be flashed */
        if (is_img_download_done) {
            is_all_img_flashed = ((dl_param->processing_img_idx + 1) == dl_param->img_num) ? True : False;
            i4_ret = AawantCmd_Call_App_To_Flash_Data(img_info, unflashed_size, is_all_img_flashed);
            if (i4_ret) {
                printf("[%s:%d]==>call_app_to_flash_data failed, cancel download\n", __FUNCTION__, __LINE__);
                Aawant_Set_Upgrade_Status(dl_param, AAW_CTL_DOWNLOAD_CANCEL);
                Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
                return i4_ret;
            }
            img_info->img_flashed_size += unflashed_size;
            img_info->is_img_flashed_done = True;
            Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
        } else {
            printf("[%s]==>unflash data size %d is less than flash_unit_size %d, skip this time\n", __FUNCTION__,
                   unflashed_size, flash_unit_size);
            return i4_ret;
        }
    } else {
        if (is_img_download_done)
            is_all_img_flashed = ((dl_param->processing_img_idx + 1) == dl_param->img_num) ? True : False;

        i4_ret = AawantCmd_Call_App_To_Flash_Data(img_info, flash_unit_size, is_all_img_flashed);
        if (i4_ret) {
            printf("[%s]==>Call_App_To_Flash_Data failed, cancel download\n", __FUNCTION__);
            Aawant_Set_Upgrade_Status(dl_param, AAW_CTL_DOWNLOAD_CANCEL);
            Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
            return i4_ret;
        }
        img_info->img_flashed_size += flash_unit_size;
        Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
    }
    printf("[%s]==>img flashed/downloaded/total size:%d/%d/%d \n", __FUNCTION__,
           img_info->img_flashed_size, img_info->img_downloaded_size, img_info->img_header.comp_size);

    return i4_ret;

}

/**
 * 烧写升级程序完成
 */

/*
void AawantCmd_Flash_Img_Done(void)
{
    int32 i4_ret = 0;
    char old_id_file[BUF_SIZE] = {0};
    char new_id_file[BUF_SIZE] = {0};
    char cmd_buf[BUF_SIZE] = {0};

    strncpy(old_id_file, UPGRADE_ID_FILE_SAVE_PATH, strlen(UPGRADE_ID_FILE_SAVE_PATH));
    strncat(old_id_file, UPGRADE_ID_FILE_NAME, strlen(UPGRADE_ID_FILE_NAME));
    strncpy(new_id_file, UPGRADE_ID_FILE_TMP_PATH, strlen(UPGRADE_ID_FILE_TMP_PATH));
    strncat(new_id_file, UPGRADE_ID_FILE_NAME, strlen(UPGRADE_ID_FILE_NAME));

    if (NULL == access(new_id_file, F_OK))
    {
        snprintf(cmd_buf, 256, "mv -f %s %s", new_id_file, old_id_file);
        i4_ret = system(cmd_buf);
        if (i4_ret)
            printf("[%s]==>storage new id file failed\n",__FUNCTION__);
        else
            printf("[%s]==>storage new id file to %s\n",__FUNCTION__,old_id_file);
    }

    printf("[%s]==>upgrade done! now reboot!\n",__FUNCTION__);

    Aawant_Set_Upgrade_Status(AAW_CTL_UPGRADE_SUCESS);

    system("reboot");

}
*/


/**
 * 把升级程序烧写到flash
 * @param dl_param
 * @return
 */
int32 AawantCmd_Flash_ImgData(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    boolean is_full_pkg = dl_param->is_full_pkg_update;
    Aawant_Set_Upgrade_Status(dl_param, AAW_CTL_UPGRADE_DOING);
//#define ZIP_PATH  "/tmp/update.zip"
    mprintf("------------------[%s][Start]---------------------\n", __FUNCTION__);
    if (is_full_pkg) {
        i4_ret = Aawant_Fully_Flash_ImgData(dl_param);
        mprintf("------------------[%s][Finish]--------------------\n", __FUNCTION__);
        /*
        if(i4_ret!=-1){
            if(0==access(ZIP_PATH,F_OK))
            {
                if (0 == remove(ZIP_PATH))
                {
                    printf("remove existing %s \n", ZIP_PATH);
                }
                else
                {
                    printf("remove existing %s failed\n", ZIP_PATH);
                    return -1;
                }
            }
        }
         */
    } else {
        i4_ret = Aawant_Sectionally_Flash_ImgData(dl_param);
        mprintf("[%s]==>Sectionally_Flash_ImgData Finish\n", __FUNCTION__);
    }
    if (i4_ret == -1) {
        Aawant_Set_Upgrade_Status(dl_param, AAW_CTL_UPGRADE_FAIL);
    } else {
        Aawant_Set_Upgrade_Status(dl_param, AAW_CTL_UPGRADE_SUCESS);
    }
    return i4_ret;
}

void Aawant_Set_Upgrade_Status(DOWNLOAD_PARAM *dl, AAWANT_UPG_CTL_STATUS status) {
    pthread_mutex_lock(&dl->status_mutex);
    aa_status = status;

    mprintf("[%s]==>current status:%d\n", __FUNCTION__, aa_status);
    pthread_mutex_unlock(&dl->status_mutex);
}


//E_UPG_CONTROL_UPGRADE_STATUS Aawant_Get_Upgrade_Status(void){
AAWANT_UPG_CTL_STATUS Aawant_Get_Upgrade_Status(void) {
    mprintf("[%s]==>current status:%d\n", __FUNCTION__, aa_status);
    return aa_status;
}


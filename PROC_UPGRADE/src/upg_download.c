#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/statfs.h>
#include <malloc.h>
#include <AIUComm.h>

//#include "u_appman.h"
//#include "u_app_thread.h"
#include "upg_download.h"
#include "curl/curl.h"
#include "AIEUComm.h"
#include "AawantData.h"

#define BUF_SIZE_ENTRIE 256
#define BUF_SIZE_HALF 128
#define BUF_SIZE_QUT 64
#define URL_LENGTH 255


char g_url[BUF_SIZE_ENTRIE] = {NETWORK_UPGRADE_BASE_URL};
DOWNLOAD_TREAD_PARAM g_dl_param;
DOWNLOAD_PARAM a_dl_param;
//E_UPG_CONTROL_UPGRADE_STATUS a_status;
AAWANT_UPG_CTL_STATUS aa_status;


/***********************
 * by sine             *
 ***********************/





double Aawant_Get_Download_FileLenth(CURL *handle, const char *url) {
    double downloadFileLenth = 0.0;
    CURL *curl = NULL;
    CURLcode code;


    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    code = curl_easy_perform(curl);
    if (code == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
    } else {
        downloadFileLenth = -1;
    }
    mprintf("[%s]==>url=%s,FileLenth:%f\n", __FUNCTION__, url, downloadFileLenth);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return downloadFileLenth;
}


int32 Aawant_DownLoad_init(DOWNLOAD_PARAM *dl_param,
                           char *url,
                           char *save_path,
                           int32 file_size,
                           boolean is_full_pkg) {
    int32 i4_ret = 0;
    double dl_lenth = 0;
    char *file_name = NULL;
    mprintf("-------->%s:(^_^)<---------\n", __FUNCTION__);


    if (is_full_pkg)//update.zip
    {
        file_name = UPGRADE_FULL_PKG_NAME;
    } else//update.bin
    {
        file_name = UPGRADE_OTA_FILE_NAME;
    }
    // memset(dl_param, 0, sizeof(DOWNLOAD_PARAM));

    pthread_mutex_init(&dl_param->flash_mutex, NULL);
    pthread_cond_init(&dl_param->flash_cond, NULL);

    // dl_param->is_full_pkg_update = is_full_pkg;
    //dl_param->upg_file_size = file_size;
    memset(dl_param->save_path, 0, sizeof(dl_param->save_path));
    strncpy(dl_param->url, url, strlen(url));
    strncat(dl_param->url, file_name, strlen(file_name));

    strncpy(dl_param->save_path, save_path, strlen(save_path));
    strncat(dl_param->save_path, file_name, strlen(file_name));

    /*
     * 获得下载文件大小信息
     */
    dl_lenth = Aawant_Get_Download_FileLenth(NULL, dl_param->url);
    if ((dl_lenth > 0) &&
        ((int32) dl_lenth != file_size)) {
        dl_param->upg_file_size = (int32) dl_lenth;
        printf("[%s]==>file size from id:%d, size not match, use server size:%d.\n", __FUNCTION__, file_size,
               dl_param->upg_file_size);
    } else if (-1 == dl_lenth) {
        return -1;
    }

    dl_param->fp = fopen(dl_param->save_path, "wb");
    if (dl_param->fp == NULL) {
        printf("[%s]==>fail to open save file:%s, reopen later\n", __FUNCTION__, dl_param->save_path);
        return -1;
    }

    return i4_ret;
}


int32 Aawant_Get_Dowload_Free_StorageSize(char *path) {
    int32 i4_ret = 0;
    struct statfs fs_info = {0};
    int32 avail_size = 0;
    mprintf("[%s]==>\n", __FUNCTION__);
    i4_ret = statfs(path, &fs_info);
    if (i4_ret) {
        printf("[%s]==>%s:statsfs failed\n", __FUNCTION__, path);
        return -1;
    }
    avail_size = fs_info.f_bsize * fs_info.f_bavail;

    printf("[%s]==> block size:%d, total block:%d, free block:%d, available block:%d \n", __FUNCTION__,
           path, fs_info.f_bsize, fs_info.f_blocks, fs_info.f_bfree, fs_info.f_bavail);

    return avail_size;
}

int32 Aawant_Check_Download_StorageSize(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    int32 avail_size = 0;
    int32 ota_size = 0;
    mprintf("[%s]==>\n", __FUNCTION__);
    ota_size = dl_param->upg_file_size;
    avail_size = Aawant_Get_Dowload_Free_StorageSize(UPGRADE_FULL_PKG_SAVE_PATH);
    if (avail_size < ota_size) {
        printf("[%s]==>check free size failed, available size:%d, ota size:%d\n",
               __FUNCTION__, avail_size, ota_size);
        i4_ret = -1;
    }

    return i4_ret;
}


void Aawant_Map_Partition_Name(const char *img_name, char **part_name) {
    mprintf("[%s]==>\n", __FUNCTION__);
    if (!strncmp(img_name, BOOT_IMAGE_NAME, strlen(BOOT_IMAGE_NAME))) {
        strncpy(*part_name, A_BOOT_PARTITION_NAME, sizeof(A_BOOT_PARTITION_NAME));
    } else if (!strncmp(img_name, TZ_IMAGE_NAME, strlen(TZ_IMAGE_NAME))) {
        strncpy(*part_name, A_TZ_PARTITION_NAME, sizeof(A_TZ_PARTITION_NAME));
    } else if (!strncmp(img_name, ROOTFS_IMAGE_NAME, strlen(ROOTFS_IMAGE_NAME))) {
        strncpy(*part_name, A_ROOTFS_PARTITION_NAME, sizeof(A_ROOTFS_PARTITION_NAME));
    } else if (!strncmp(img_name, LK_IMAGE_NAME, strlen(LK_IMAGE_NAME))) {
        strncpy(*part_name, LK_PARTITION_NAME, sizeof(LK_PARTITION_NAME));
    }
    printf("image name:%s, map to partition name:%s \n", img_name, *part_name);
}

int32 Aawant_Get_ImgBlkSize_By_Name(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    int32 img_idx = 0;
    int32 img_blk_size = 0;
    char partition_name[BUF_SIZE_QUT] = {0};
    char *p_part_name = NULL;;
    UPGRADE_IMAGE_INFO *image_info = NULL;
    mprintf("[%s]==>\n", __FUNCTION__);
    while (img_idx != dl_param->img_num) {
        memset(partition_name, 0, 64);
        //p_part_name = &partition_name;
        p_part_name = partition_name;
        image_info = &(dl_param->image[img_idx]);

        Aawant_Map_Partition_Name(image_info->img_header.file_name, &p_part_name);
        i4_ret = get_partition_blk_size_by_name(partition_name, &image_info->img_blk_size);
        if (i4_ret) {
            printf("get_partition_blk_size_by_name failed\n");
            return i4_ret;
        }

        img_blk_size = image_info->img_blk_size;
        //每次烧写2M
        image_info->img_flash_unit_size =
                (UPGRADE_TRANSFER_FLASH_SIZE + img_blk_size - 1) / img_blk_size * img_blk_size;
        img_idx++;
    }
    return i4_ret;
}


int32 Aawant_Download_Curl_init(DOWNLOAD_PARAM *dl_param, CURL *curl, CURL_CB_FCT curl_callback_fct) {
    int32 i4_ret = 0;
    mprintf("[%s]==>\n", __FUNCTION__);
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback_fct);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, dl_param);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_DOWNLOAD_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, dl_param->curl_error_buf);

    /* To prevent blocking when connection issue occured */
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 50);
    curl_easy_setopt(curl, CURLOPT_URL, dl_param->url);

    printf("[%s]==>init url:%s\n", __FUNCTION__, dl_param->url);

    dl_param->curl = curl;
    dl_param->is_curl_inited = True;
    return i4_ret;
}

int32 Aawant_Download_Param_Reset(DOWNLOAD_PARAM *dl_param) {
    mprintf("[%s]==>\n", __FUNCTION__);
    dl_param->errcode = (CURLcode) 0;
    dl_param->is_head_info = False;
    dl_param->requested_size = 0;
    dl_param->downloaded_size = 0;
    dl_param->conntent_buf = NULL;
    dl_param->is_request_done = False;
}


int32 Aawant_Check_UpgStatus(void) {

    int32 i4_ret = 0;
    AAWANT_UPG_CTL_STATUS status = (AAWANT_UPG_CTL_STATUS) 0;

    status = Aawant_Get_Upgrade_Status();
    //mprintf("[%s]==>status is %d\n",__FUNCTION__,status);
    if (status == AAW_CTL_DOWNLOAD_CANCEL) {
        i4_ret = -1;
    }
    return i4_ret;
}


int32 Aawant_Resume_Download(DOWNLOAD_PARAM *dl_param, int32 offset, int32 file_size) {
    int32 i4_ret = 0;
    int32 downloaded_size = 0;
    int32 requested_size = 0;
    double dl_percent = 0;
    int32 try_times = UPGRADE_DOWNLOAD_RESUME_TIMES;

    printf("[%s]==>Trying to resume download from brocken\n", __FUNCTION__);

    while (try_times--) {
        /* for head info, use offset directly */
        if (dl_param->is_head_info) {
            downloaded_size = 0;
        } else {
            downloaded_size = dl_param->downloaded_size;
        }
        requested_size = dl_param->requested_size;
        dl_percent = (double) downloaded_size / (double) requested_size;
        printf("[%s]==>Download from offset:%d, progress: %0.2f%%\n", __FUNCTION__, downloaded_size + offset,
               dl_percent * 100);

        if (downloaded_size < requested_size) {
            curl_easy_setopt(dl_param->curl, CURLOPT_RESUME_FROM, downloaded_size + offset);
            dl_param->errcode = curl_easy_perform(dl_param->curl);

            if (CURLE_OK == dl_param->errcode) {
                printf("[%s]==>complete download! size:%d\n", __FUNCTION__, dl_param->downloaded_size);
                return 0;
            } else {
                i4_ret = Aawant_Check_UpgStatus();
                if (i4_ret) {
                    printf("[%s]==>check upg status failed, download may be cancelled\n", __FUNCTION__);
                    return -1;
                }

                if (dl_param->is_request_done) {
                    printf("[%s]==>complete download! size:%d\n", __FUNCTION__, dl_param->downloaded_size);
                    return 0;
                } else {
                    printf("[%s]==>Resume download failed, err:%d %s %s, times:%d\n", __FUNCTION__,
                           dl_param->errcode, curl_easy_strerror(dl_param->errcode), dl_param->curl_error_buf,
                           try_times);
                    /* met connection issue, wait 1s and try again */
                    if (CURLE_COULDNT_CONNECT == dl_param->errcode)
                        sleep(1);
                }
            }
        }
    }
    printf("[%s]==>Resume download failed\n", __FUNCTION__);
    return -1;
}


int32 Aawant_Download_From_Offset(DOWNLOAD_PARAM *dl_param, int32 offset, int32 size) {
    mprintf("[%s]==>offset:%d,size:%d\n", __FUNCTION__, offset, size);
    int32 i4_ret = 0;
    if (!dl_param->is_curl_inited) {
        printf("[%s:%d]==>please init curl first\n", __FUNCTION__, __LINE__);
        i4_ret = -1;
    }

    if (offset) {
        curl_easy_setopt(dl_param->curl, CURLOPT_RESUME_FROM, offset);
    }
    if (size) {
        dl_param->requested_size = size;
    }
    printf("[%s:%d]==>start to download!\n", __FUNCTION__, __LINE__);
    printf("[%s]==>from:%s, offset:%d, size:%d\n", __FUNCTION__, dl_param->url, offset, dl_param->requested_size);
    dl_param->errcode = curl_easy_perform(dl_param->curl);

    if (CURLE_OK != dl_param->errcode) {
        i4_ret = Aawant_Check_UpgStatus();
        if (i4_ret) {
            printf("[%s:%d]==>check upg status failed, download may be cancelled\n", __FUNCTION__, __LINE__);
            return -1;
        }

        if (dl_param->is_request_done) {
            printf("[%s:%d]==>complete unit download! downloaded size:%d\n", __FUNCTION__, __LINE__,
                   dl_param->downloaded_size);
            i4_ret = 0;
        } else {
            printf("[%s:%d]==>download fail , err:%d %s %s\n ", __FUNCTION__, __LINE__,
                   dl_param->errcode, curl_easy_strerror(dl_param->errcode), dl_param->curl_error_buf);

            /* use absolute offset for full pkg download resuming */
            if (dl_param->is_full_pkg_update) {
                offset = 0;
            }
            i4_ret = Aawant_Resume_Download(dl_param, offset, dl_param->requested_size);
        }
    } else {
        printf("[%s:%d]==>complete download! downloaded size:%d\n", __FUNCTION__, __LINE__, dl_param->downloaded_size);
        i4_ret = 0;
    }

    return i4_ret;
}


int32 Aawant_Parse_ZipHeadInfo(const struct zip_header_raw *raw_data, struct zip_header *data) {
    int32 i4_ret = 0;

    parse_zip_head_info(raw_data, data);
    if (ZIP_HEAD_SIGN != data->sign_head) {
        printf("[%s]==>parse_zip_head_info failed, header signature not match\n", __FUNCTION__);
        i4_ret = -1;
    }

    return i4_ret;
}

int32 Aawant_Request_To_Flash_Data(DOWNLOAD_PARAM *dl) {
    printf("[%s]==>\n", __FUNCTION__);


    int ret = 0;
    /*
    FROM_UPGRADE_DATA upgradeData;

//    upgradeData.status=(E_UPG_CONTROL_UPGRADE_STATUS)0;
    upgradeData.status=DOWNLOAD_FINISH_AND_REQUEST_UPGRADE;
    upgradeData.code=0;

    printf("[%s]==>sock=%d\n",__FUNCTION__,a_dl_param.dl_sock);
    ret=AAWANTSendPacket(dl->dl_sock,PKT_UPGRADE_FEEDBACK,(char *)&upgradeData, sizeof(upgradeData));
    if(ret==-1)
        printf("[%s]==>request to flash fail\n",__FUNCTION__);
        */
    return ret;

}


int32 Aawant_Wakeup_Data_Flash_Done(DOWNLOAD_PARAM *dl_param, boolean *is_done) {
    int32 i4_ret = 0;

    pthread_mutex_lock(&dl_param->flash_mutex);
    printf("[%s]==>Aawant_Wakeup_Data_Flash_Done\n", __FUNCTION__);
    *is_done = True;
    pthread_cond_signal(&dl_param->flash_cond);
    pthread_mutex_unlock(&dl_param->flash_mutex);

    return i4_ret;
}


int32 Aawant_Wait_Data_Flash_Done(DOWNLOAD_PARAM *dl_param, boolean *is_done) {
    int32 i4_ret = 0;

    pthread_mutex_lock(&dl_param->flash_mutex);
    while (!(*is_done)) {
        printf("[%s]==>wait_data_flash_done\n", __FUNCTION__);
        pthread_cond_wait(&dl_param->flash_cond, &dl_param->flash_mutex);
    }
    pthread_mutex_unlock(&dl_param->flash_mutex);

    return i4_ret;
}


size_t Aawant_Download_FullOtaPackage_CB(void *ptr, size_t size, size_t nmemb, void *stream) {
    int32 i4_ret = 0;
    DOWNLOAD_PARAM *dl_param = (DOWNLOAD_PARAM *) stream;
    uchar *buffer = NULL;
    double percent = 0.0;
    int32 file_size = 0;
    if (NULL == dl_param->fp) {
        printf("[%s]==>please init download parameter first\n", __FUNCTION__);
        return -1;
    }


    i4_ret = Aawant_Check_UpgStatus();
    if (i4_ret) {
        printf("[%s]==>check upg status failed, download may be cancelled\n", __FUNCTION__);
        return -1;
    }

    buffer = (unsigned char *) malloc(size * nmemb);
    if (NULL == buffer) {
        printf("[%s]==>malloc buffer failed\n", __FUNCTION__);
        return -1;
    }

    file_size = ftell(dl_param->fp);
    printf("[%s]==>full cb: current dl size:%d, file size:%d\n", __FUNCTION__, dl_param->downloaded_size, file_size);

    memcpy(buffer, ptr, size * nmemb);
    i4_ret = fwrite(buffer, 1, size * nmemb, dl_param->fp);
    if (i4_ret != (size * nmemb)) {
        printf("[%s]==>ERROR: write size:%d, but return:%d, something may wrong\n", __FUNCTION__, size * nmemb, i4_ret);
        //    _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_CANCELLED);
        free(buffer);
        return -1;
    }

    fflush(dl_param->fp);
    fsync(fileno(dl_param->fp));
    free(buffer);
    dl_param->downloaded_size += (size * nmemb);

    percent = (double) dl_param->downloaded_size / dl_param->requested_size * 100;
    printf("[%s]==>full pkg download: download data/requested size: %d/%d --- %0.2f%%\n", __FUNCTION__,
           dl_param->downloaded_size, dl_param->requested_size, percent);

    if (dl_param->downloaded_size >= dl_param->requested_size) {
        printf("[%s]==>full pkg download done,download_size/requested_size: %d/%d\n", __FUNCTION__,
               dl_param->downloaded_size, dl_param->requested_size);
        dl_param->is_request_done = False;
        // i4_ret =Aawant_Request_To_Flash_Data(dl_param);

        if (i4_ret == -1)
            printf("[%s]==>request_to_flash_data failed, ignore \n", __FUNCTION__);
    }
    return (size * nmemb);

}


size_t Aawant_Download_Ota_Package_CB(void *ptr, size_t size, size_t nmemb, void *stream) {
    int32 i4_ret = 0;
    DOWNLOAD_PARAM *dl_param = (DOWNLOAD_PARAM *) stream;
    uchar *buffer = NULL;
    int32 usful_size = 0;
    UPGRADE_IMAGE_INFO *img_info = NULL;

    // mprintf("[%s]==>\n",__FUNCTION__);
    i4_ret = Aawant_Check_UpgStatus();
    if (i4_ret) {
        printf("[%s:%d]==>check upg status failed, download may be cancelled\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if (dl_param->is_head_info) {
        if (dl_param->conntent_buf) {
            if ((dl_param->downloaded_size + (size * nmemb)) >= (dl_param->requested_size)) {
                memcpy(dl_param->conntent_buf, ptr, (dl_param->requested_size - dl_param->downloaded_size));
                dl_param->is_request_done = True;
                printf("[%s:%d]==>head info downloaded_size:%d, requested_size:%d\n", __FUNCTION__, __LINE__,
                       dl_param->downloaded_size + (size * nmemb), dl_param->requested_size);
                return -1;
            } else {
                memcpy(dl_param->conntent_buf, ptr, size * nmemb);
                dl_param->downloaded_size += (size * nmemb);
                return (size * nmemb);
            }
        }
    } else {
        if (NULL == dl_param->fp) {
            printf("[%s:%d]==>please init download parameter first\n", __FUNCTION__, __LINE__);
            return -1;
        }

        buffer = (unsigned char *) malloc(size * nmemb);
        if (buffer == NULL)
            return 0;
        img_info = &(dl_param->image[dl_param->processing_img_idx]);

        if ((dl_param->downloaded_size + (size * nmemb)) >= (dl_param->requested_size)) {
            //mprintf("[%s]==>download_size >= requested_size\n",__FUNCTION__);
            usful_size = dl_param->requested_size - dl_param->downloaded_size;
            memcpy(buffer, ptr, usful_size);
            fwrite(buffer, 1, usful_size, dl_param->fp);
            fflush(dl_param->fp);
            fsync(fileno(
                    dl_param->fp)); /* is it neccesary to sync this file which has no backing device? it is just memory */
            free(buffer);

            dl_param->is_request_done = True;
            dl_param->downloaded_size += (size * nmemb);
            img_info->img_downloaded_size += usful_size;
            if (img_info->img_downloaded_size >= img_info->img_header.comp_size)
                img_info->is_img_download_done = True;

            printf("[%s:%d]==>downloaded_size:%d, requested_size:%d, img_downloade_size:%d \n",
                   __FUNCTION__, __LINE__, dl_param->downloaded_size, dl_param->requested_size,
                   img_info->img_downloaded_size);

            printf("[%s:%d]==>download data/comp_size: %d/%d --- %0.2f%%\n", __FUNCTION__, __LINE__,
                   img_info->img_downloaded_size,
                   img_info->img_header.comp_size,
                   (double) img_info->img_downloaded_size / img_info->img_header.comp_size * 100);

            img_info->is_flash_unit_done = False;
//==========>


            i4_ret = Aawant_Request_To_Flash_Data(dl_param);
            if (i4_ret == -1)
                printf("[%s:%d]==>request_to_flash_data failed, ignore \n", __FUNCTION__, __LINE__);

            Aawant_Wait_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);

            return -1;
        } else {
            //mprintf("[%s]==>download_size < requested_size\n",__FUNCTION__);
            memcpy(buffer, ptr, size * nmemb);
            fwrite(buffer, 1, size * nmemb, dl_param->fp);
            fflush(dl_param->fp);
            fsync(fileno(dl_param->fp));
            free(buffer);
            dl_param->downloaded_size += (size * nmemb);
            img_info->img_downloaded_size += (size * nmemb);
            if ((img_info->img_downloaded_size - img_info->img_flashed_size) >= img_info->img_flash_unit_size) {
                printf("[%s:%d]==>download data: %d/%d --- %0.2f%%\n", __FUNCTION__, __LINE__,
                       img_info->img_downloaded_size, img_info->img_header.comp_size,
                       (double) img_info->img_downloaded_size / img_info->img_header.comp_size * 100);

                img_info->is_flash_unit_done = False;



                /*
                 //这函数放这里会导致，下载线程还没完全结束，就收到主程序升级的信息，然后就开始升级了
                 //发给下载完成的信息给主程序
                i4_ret = Aawant_Request_To_Flash_Data(dl_param);
                if (i4_ret==-1)
                {
                    printf("[%s:%d]==>request_to_flash_data failed, ignore \n",__FUNCTION__,__LINE__);
                }
                 */

                //  Aawant_Wait_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);

            }
            return (size * nmemb);
        }
    }
}

int32 Aawant_Get_Download_ImgHeadInfo(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    CURL *curl = NULL;
    int32 head_offset = 0;
    int32 img_index = 0;
    char img_header[ZIP_HEAD_PRE_SIZE] = {0};
    UPGRADE_IMAGE_INFO *image = NULL;

    mprintf("\n-------------->%s:(^_^)<--------------\n", __FUNCTION__);
    i4_ret = Aawant_Download_Curl_init(dl_param, curl, Aawant_Download_Ota_Package_CB);
    if (i4_ret) {
        printf("[%s:%d]==>curl init failed\n", __FUNCTION__, __LINE__);
        return i4_ret;
    }

    while (1) {
        Aawant_Download_Param_Reset(dl_param);
        memset(img_header, 0, ZIP_HEAD_PRE_SIZE);

        dl_param->requested_size = ZIP_HEAD_PRE_SIZE;
        dl_param->is_head_info = True;
        // dl_param->conntent_buf = &img_header;
        dl_param->conntent_buf = img_header;

        i4_ret = Aawant_Download_From_Offset(dl_param, head_offset, ZIP_HEAD_PRE_SIZE);
        if (i4_ret) {
            printf("[%s:%d]==>download from offset failed\n", __FUNCTION__, __LINE__);
            break;
        }

        if (ZIP_HEAD_SIGN != *(int32 *) (&img_header)) {
            dl_param->img_num = img_index;
            printf("[%s]==>get img headinfo done! img num:%d\n", __FUNCTION__, img_index);
            break;
        }

        if (UPGRADE_IMG_NUM <= img_index) {
            printf("[%s]==>request imgs num: %d, we now only support %d images to update\n", __FUNCTION__,
                   img_index + 1, UPGRADE_IMG_NUM);
            break;
        }

        image = &(dl_param->image[img_index]);
        i4_ret = Aawant_Parse_ZipHeadInfo((struct zip_header_raw *) (&img_header), &(image->img_header));
        if (i4_ret) {
            printf("[%s:%d]==>parse zip head info failed\n", __FUNCTION__, __LINE__);
            break;
        }

        image->img_data_offset =
                head_offset + ZIP_HEAD_SIZE_ROW + image->img_header.name_length + image->img_header.ext_field_length;
        head_offset = image->img_data_offset + image->img_header.comp_size;
        img_index++;
    }

    if (!dl_param->img_num) {
        printf("[%s:%d]==>No image need to be updated\n", __FUNCTION__, __LINE__);
        i4_ret = -1;
    }

    if (dl_param->curl) {
        curl_easy_cleanup(dl_param->curl);
        curl_global_cleanup();
        dl_param->curl = NULL;
    }

    mprintf("-------------->%s:End<--------------\n\n", __FUNCTION__);

    return i4_ret;
}


int32 Aawant_Make_DestDir(char *DEST_DIR) {
    if ((access(DEST_DIR, F_OK))) {
        if (mkdir(DEST_DIR, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
            printf("Can not mkdir %s\n", DEST_DIR);
            return -1;
        }
    }

    return 0;
}


int32 Aawant_Reopen_SaveFile(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    int32 file_size = 0;
    char *file_path = NULL;
    mprintf("-------->%s:(^_^)<---------\n", __FUNCTION__);


    if (dl_param->is_full_pkg_update) {
        file_path = UPGRADE_FULL_PKG_SAVE_PATH;
    } else {
        file_path = UPGRADE_OTA_FILE_SAVE_PATH;
    }
    if (dl_param->fp) {
        file_size = ftell(dl_param->fp);
        printf("[%s]==>close update file size: %d\n", __FUNCTION__, file_size);
        fclose(dl_param->fp);
        dl_param->fp = NULL;
    }

    i4_ret = Aawant_Make_DestDir(file_path);
    if (i4_ret) {
        printf("[%s]==>make_dest_dir failed\n", __FUNCTION__);
        return i4_ret;
    }

    if (0 == access(dl_param->save_path, F_OK)) {
        if (0 == remove(dl_param->save_path)) {
            printf("[%s]==>remove existing %s \n", __FUNCTION__, dl_param->save_path);
        } else {
            printf("[%s]==>remove existing %s failed\n", __FUNCTION__, dl_param->save_path);
            return -1;
        }
    }

    dl_param->fp = fopen(dl_param->save_path, "wb");
    if (dl_param->fp == NULL) {
        printf("[%s]==>fail to open save file:%s\n", __FUNCTION__, dl_param->save_path);
        return -1;
    }
    return 0;
}

int32 Aawant_Get_Download_ImgData(DOWNLOAD_PARAM *dl_param) {
    int32 i4_ret = 0;
    int32 img_idx = 0;
    CURL *curl = NULL;
    int32 img_dl_offset = 0;
    int32 rest_dl_size = 0;
    UPGRADE_IMAGE_INFO *image_info = NULL;

    mprintf("\n--------------->%s:(^_^)<---------------\n", __FUNCTION__);
    mprintf("[%s]==>img_num=%d\n", __FUNCTION__, dl_param->img_num);

    i4_ret = Aawant_Get_ImgBlkSize_By_Name(dl_param);
    if (i4_ret) {
        printf("[%s:%d]==>get ImgBlkSize by name failed\n", __FUNCTION__, __LINE__);
        return i4_ret;
    }

    i4_ret = Aawant_Reopen_SaveFile(dl_param);
    if (i4_ret) {
        printf("[%s:%d]==>reopen_save_file failed\n", __FUNCTION__, __LINE__);
        return i4_ret;
    }

    Aawant_Download_Curl_init(dl_param, curl, Aawant_Download_Ota_Package_CB);

    while (img_idx != dl_param->img_num) {
        mprintf("[%s]==>img_idx:%d\n", __FUNCTION__, img_idx);
        image_info = &(dl_param->image[img_idx]);
        img_dl_offset = image_info->img_data_offset;
        dl_param->processing_img_idx = img_idx;

        while (!image_info->is_img_download_done) {

            Aawant_Download_Param_Reset(dl_param);

            mprintf("[%s]==>\nblk size=%d,\ndata_offset=%d,\ndownload_size=%d,\nflash_unit_size=%d,\n"
                    "flash_size=%d\n", __FUNCTION__, image_info->img_blk_size, image_info->img_data_offset,
                    image_info->img_downloaded_size, image_info->img_flash_unit_size,
                    image_info->img_flashed_size);

            /* adjust to align flash unit size since update.bin has been removed */
            img_dl_offset = image_info->img_data_offset + image_info->img_flashed_size;
            image_info->img_downloaded_size = image_info->img_flashed_size;

            rest_dl_size = image_info->img_header.comp_size - image_info->img_downloaded_size;
            dl_param->requested_size = (rest_dl_size > image_info->img_flash_unit_size) ?
                                       image_info->img_flash_unit_size : rest_dl_size;
            dl_param->is_head_info = False;

            mprintf("[s%]==>img_dl_offset=%d,dl_param->requested_size=%d\n", img_dl_offset, dl_param->requested_size);
            i4_ret = Aawant_Download_From_Offset(dl_param, img_dl_offset, dl_param->requested_size);
            if (i4_ret) {
                printf("[%s]==>Download_From_Offset failed\n", __FUNCTION__);
                goto out;
            }

            i4_ret = Aawant_Reopen_SaveFile(dl_param);
            if (i4_ret) {
                printf("[%s]==>Reopen_SaveFile failed\n", __FUNCTION__);
                goto out;
            }
        }

        img_idx++;
    }

    mprintf("--------------->%s:End<---------------\n\n", __FUNCTION__);


    out:
    if (dl_param->curl) {
        curl_easy_cleanup(dl_param->curl);
        curl_global_cleanup();
        dl_param->curl = NULL;
    }

    return i4_ret;
}

void Aawant_Clean_Download_BasicInfo(DOWNLOAD_PARAM *dl_param) {
    pthread_cond_destroy(&dl_param->flash_cond);
    pthread_mutex_destroy(&dl_param->flash_mutex);

    mprintf("-------->%s:(^_^)<---------\n", __FUNCTION__);

    if (dl_param->curl) {
        curl_easy_cleanup(dl_param->curl);
        curl_global_cleanup();
        dl_param->curl = NULL;
    }

    if (dl_param->fp) {
        fclose(dl_param->fp);
        dl_param->fp = NULL;
    }

    /*
    //删除下载的文件
    if (0 == access(dl_param->save_path, F_OK))
    {
        if (0 == remove(dl_param->save_path))
        {
            printf("[%s]==>remove existing %s \n",__FUNCTION__, dl_param->save_path);
        }
        else
        {
            printf("[%s]==>remove existing %s failed\n",__FUNCTION__, dl_param->save_path);
        }
    }
    */
}


//int32 Aawwant_Download_OtaPackage(char *url, char *save_path, int32 upg_file_size)
int32 Aawwant_Download_OtaPackage(DOWNLOAD_PARAM dl, char *url, char *save_path, int32 upg_file_size) {
    int32 i4_ret = 0;
    CURL *curl = NULL;
    //DOWNLOAD_PARAM *dl_param = &a_dl_param;
    DOWNLOAD_PARAM *dl_param = &dl;

    printf("|<-------------%s:Start(-_-)-------------->|\n", __FUNCTION__);
    i4_ret = Aawant_DownLoad_init(dl_param, url, save_path, upg_file_size, False);
    if (i4_ret) {
        printf("%s:%d==>init failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    i4_ret = Aawant_Get_Download_ImgHeadInfo(dl_param);
    if (i4_ret) {
        printf("%s:%d==>Download ImgHeadInfo failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    i4_ret = Aawant_Get_Download_ImgData(dl_param);
    if (i4_ret) {
        printf("%s:%d==>Dowload ImgData failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    out:
    Aawant_Clean_Download_BasicInfo(dl_param);

    printf("|<-------------%s:End-------------->|\n", __FUNCTION__);
    return i4_ret;
}


int32 Awant_Get_Download_FullPkgData(DOWNLOAD_PARAM *dl) {
    int32 i4_ret = 0;
    CURL *curl = NULL;

    // DOWNLOAD_PARAM* dl_param = &a_dl_param;
    DOWNLOAD_PARAM *dl_param = dl;

    mprintf("-------->%s:Start(^_^)<---------\n", __FUNCTION__);
    Aawant_Download_Curl_init(dl_param, curl, Aawant_Download_FullOtaPackage_CB);

    Aawant_Download_Param_Reset(dl_param);

    while (dl_param->downloaded_size < dl_param->upg_file_size) {
        i4_ret = Aawant_Download_From_Offset(dl_param, dl_param->downloaded_size, dl_param->upg_file_size);
        if (i4_ret) {
            /* sometimes the connection will be closed by server, we will try to request data untill all data downloaded,
                         but if resume download failed, we'll return failure. */
            printf("%s==>Aawant_Download_From_Offset failed\n", __FUNCTION__);
            goto out;
        }
    }
    // Aawant_Wait_Data_Flash_Done(dl_param, &dl_param->is_request_done);

    out:
    if (dl_param->curl) {
        curl_easy_cleanup(dl_param->curl);
        curl_global_cleanup();
        dl_param->curl = NULL;
    }

    printf("|<-------------%s:End-------------->|\n", __FUNCTION__);
    return i4_ret;

}





/**
 * 全包
 * @param url
 * @param save_path
 * @param upg_file_size
 * @return
 */
//int32 Aawant_Download_FullOtaPackage(char *url, char *save_path, int32 upg_file_size)
int32 Aawant_Download_FullOtaPackage(DOWNLOAD_PARAM dl, char *url, char *save_path, int32 upg_file_size) {
    int32 i4_ret = 0;
    CURL *curl = NULL;
    // DOWNLOAD_PARAM* dl_param = &a_dl_param;
    DOWNLOAD_PARAM *dl_param = &dl;

    mprintf("|<------------%s(+_+)----------->|\n", __FUNCTION__);
    i4_ret = Aawant_DownLoad_init(dl_param, url, save_path, upg_file_size, True);
    if (i4_ret) {
        printf("[%s:%d]==>download init failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    /*
     * 检查存储空间是否足够
     */

    i4_ret = Aawant_Check_Download_StorageSize(dl_param);
    if (i4_ret) {
        printf("[%s:%d]==>check storage size failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    i4_ret = Aawant_Reopen_SaveFile(dl_param);
    if (i4_ret) {
        printf("[%s:%d]==>reopen_save_file failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    i4_ret = Awant_Get_Download_FullPkgData(dl_param);
    if (i4_ret) {
        printf("[%s:%d]==>get FullPkgData failed\n", __FUNCTION__, __LINE__);
        goto out;
    }

    out:
    Aawant_Clean_Download_BasicInfo(dl_param);

    return i4_ret;
}


int32 Aawant_Notify_Flash_Done(DOWNLOAD_PARAM dl) {
#ifdef ISNEED
    int32 i4_ret = 0;
    UPGRADE_DL_MSG dl_msg;
    dl_msg.pt_msg.ui4_sender_id = MSG_FROM_UPG_CONTROL;
    dl_msg.pt_msg.ui4_msg_type = E_UPG_CONTROL_FLASH_IMG_DONE;
    dl_msg.dl_param = NULL;
    i4_ret = _upgrade_download_send_msg(&dl_msg, UPG_CONTROL_THREAD_NAME);
    if (i4_ret)
    {
        printf("_upgrade_download_notify_flash_done failed:%d\n", i4_ret);
    }

    return i4_ret;
#endif

    FROM_UPGRADE_DATA upgradeData;

    upgradeData.status = UPGRADE_FINISH_AND_REQUEST_REBOOT;
    //upgradeData.code=0;
    // printf("[%s]==>dl_sock=%d\n",__FUNCTION__,a_dl_param.dl_sock);
    printf("[%s]==>dl_sock=%d\n", __FUNCTION__, dl.dl_sock);
    AAWANTSendPacket(dl.dl_sock, PKT_UPGRADE_FEEDBACK, (char *) &upgradeData, sizeof(upgradeData));
}


/**********
 *
 * @param url
 * @param save_path
 * @return
 */
int32 Aawant_StartDownLoad(DOWNLOAD_PARAM dl, char *g_url, char *save_path, boolean is_full_pkg) {
    int ret;

#define ZIP_PATH  "/tmp/update.zip"
    if (is_full_pkg) {

        if (0 == access(ZIP_PATH, F_OK)) {

            if (0 == remove(ZIP_PATH)) {
                printf("remove existing %s \n", ZIP_PATH);
            } else {
                printf("remove existing %s failed\n", ZIP_PATH);
                return -1;
            }
        }

        ret = Aawant_Download_FullOtaPackage(dl, g_url, UPGRADE_FULL_PKG_SAVE_PATH, 0);
        if (ret) {
            printf("[%s]==>Download FullOtaPackage failed,%d\n", __FUNCTION__, ret);
            return ret;
        }

    } else {


        ret = Aawwant_Download_OtaPackage(dl, g_url, UPGRADE_OTA_FILE_SAVE_PATH, 0);
        if (ret) {
            printf("[%s]==>Download OtaPackage failed,%d\n", __FUNCTION__, ret);
            return ret;
        }
    }

    /*
    ret = Aawant_Notify_Flash_Done(dl);
    if (ret)
    {
        printf("[%s]=Notify Flash Done failed\n",__FUNCTION__);
    }

    printf("[%s]==>ret=%d\n",__FUNCTION__,ret);
    */
    return ret;
}


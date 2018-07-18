/* Copyright Statement:                                                        
 *                                                                             
 * This software/firmware and related documentation ("MediaTek Software") are  
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without 
 * the prior written permission of MediaTek inc. and/or its licensors, any     
 * reproduction, modification, use or disclosure of MediaTek Software, and     
 * information contained herein, in whole or in part, shall be strictly        
 * prohibited.                                                                 
 *                                                                             
 * MediaTek Inc. (C) 2014. All rights reserved.                                
 *                                                                             
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")     
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER  
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL          
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED    
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR          
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH 
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,            
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.   
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK       
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE  
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR     
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S 
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE       
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE  
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE  
 * charGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <AawantData.h>
#include <AIUComm.h>

/* application level */
//#include "u_amb.h"
//#include "u_common.h"
//#include "u_app_thread.h"
//#include "u_cli.h"
//#include "u_os.h"
//#include "u_timerd.h"
//#include "u_assert.h"
//#include "u_sm.h"
//#include "u_dm.h"
//#include "u_fw_version.h"

/* private */
#include "upg_control.h"
#include "upg_control_cli.h"
#include "upg_download.h"

#ifdef ISNEED
static UPG_CONTROL_OBJ_T t_g_upg_control               = {0};

static HANDLE_T h_check_update_timer = NULL_HANDLE;
#endif

static uint8 _g_check_times = 0;

#define BUF_SIZE  256
#define BUF_SIZE_HALF 128
#define UPG_START_TIME 4
#define UPG_END_TIME 6


#ifdef ISNEED
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static int32 _upg_control_start (
                    const char*                 ps_name,
                    HANDLE_T                    h_app
                    );
static int32 _upg_control_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static int32 _upg_control_process_msg (
                    HANDLE_T                    h_app,
                    uint32                      ui4_type,
                    const void*                 pv_msg,
                    size_t                      z_msg_len,
                    boolean                        b_paused
                    );


/*---------------------------------------------------------------------------
 * Name
 *      a_app_set_registration
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
void a_upg_control_register(AMB_REGISTER_INFO_T *pt_reg)
{
    if (t_g_upg_control.b_app_init)
    {
        return;
    }

    DBG_UPG_CONTROL_DEFAULT(("%s is called\n", __FUNCTION__));

    memset(pt_reg->s_name, 0, sizeof(char) *(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, UPG_CONTROL_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _upg_control_start;
    pt_reg->t_fct_tbl.pf_exit                   = _upg_control_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _upg_control_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = UPG_CONTROL_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = UPG_CONTROL_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = UPG_CONTROL_NUM_MSGS;

    pt_reg->t_desc.ui2_msg_count                = UPG_CONTROL_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = UPG_CONTROL_MAX_MSGS_SIZE;
}

static int32 _upg_control_start (const char *ps_name,HANDLE_T h_app)
{
    int32 i4_ret;
    memset(&t_g_upg_control, 0, sizeof(UPG_CONTROL_OBJ_T));
    t_g_upg_control.h_app = h_app;

    if (t_g_upg_control.b_app_init)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = upg_control_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        return AEER_FAIL;
    }
    upg_control_set_dbg_level(DBG_LEVEL_ERROR);
#endif /* CLI_SUPPORT */

    t_g_upg_control.b_app_init = TRUE;

    i4_ret = u_timer_create(&h_check_update_timer);
    if(i4_ret)
        DBG_UPG_CONTROL_DEFAULT(("create timer fail.\n"));

    _upg_control_start_check_update_timer();

    _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_INITED);

    return AEER_OK;
}

static int32 _upg_control_exit (HANDLE_T h_app, APP_EXIT_MODE_T e_exit_mode)
{
    int32 i4_ret;

    t_g_upg_control.b_app_init = FALSE;
    if (NULL_HANDLE != h_check_update_timer)
    {
        u_timer_stop(h_check_update_timer);
        u_timer_delete(h_check_update_timer);
    }
    return AEER_OK;
}

int32 _upg_control_send_msg(APPMSG_T *pt_msg,const char *ps_name)
{
    int32 i4_ret = UPG_CONTROL_OK;
    HANDLE_T h_app = NULL_HANDLE;

    i4_ret = u_am_get_app_handle_from_name(&h_app, ps_name);
    UPG_CONTROL_ASSERT(i4_ret);

    i4_ret = u_app_send_msg(h_app,
                            E_APP_MSG_TYPE_UPG_CTRL,
                            pt_msg,
                            sizeof(APPMSG_T),
                            NULL,
                            NULL);
    UPG_CONTROL_ASSERT(i4_ret);
    return i4_ret;
}

#if CONFIG_SUPPORT_UPG_FROM_USB
int32  _upgrade_handle_usb_mount_msg(void)
{
    char buf[BUF_SIZE] = {0};
    int len = 0;
    char *pValue = buf;
    int32 i4_ret = 0;
    FM_MNT_INFO_T t_mnt_info;
    char ps_cmd[UPG_CMD_LEN] = "";
    char ac_id_file_path[UPG_MAX_FILE_PATH_LEN] = {0};

    i4_ret = u_dm_get_mnt_info(&t_mnt_info);
    if (DMR_OK != i4_ret)
    {
        DBG_UPG_CONTROL_DEFAULT(("get mnt info failed(ret:%d)!\n", i4_ret));
        return UPG_CONTROL_FAIL;
    }

    memcpy(ac_id_file_path, t_mnt_info.s_mnt_path,  sizeof(t_mnt_info.s_mnt_path) - 1);
    strncat(ac_id_file_path, "/update.zip", UPG_MAX_FILE_PATH_LEN - strlen(ac_id_file_path) - 1);


    DBG_UPG_CONTROL_DEFAULT(("usb_upg_file is:%s\n", ac_id_file_path));
    i4_ret= access(ac_id_file_path, F_OK);

    DBG_UPG_CONTROL_DEFAULT(("i4_ret is:%d\n", i4_ret));
    if (NULL == (access(ac_id_file_path, F_OK)))
    {
    #if 1
        //step1
        snprintf(ps_cmd, sizeof(ps_cmd), "cat %s/update.zip > /tmp/update.zip", t_mnt_info.s_mnt_path);
        DBG_UPG_CONTROL_DEFAULT(("usb upgrade step1 command is:%s\n", ps_cmd));
        i4_ret = system(ps_cmd);
        if (i4_ret != UPG_CONTROL_OK)
        {
            DBG_UPG_CONTROL_DEFAULT(("Err: step1:USB upgrade copy fail @Line %d\n", __LINE__));
            return UPG_CONTROL_FAIL;
        }
        else
        {
            DBG_UPG_CONTROL_DEFAULT(("OK: step1:USB upgrade copy sucess @Line  %d\n", __LINE__));;//reboot;
        }

        //step2
        memset(ps_cmd, 0, UPG_CMD_LEN);
        snprintf(ps_cmd, sizeof(ps_cmd), "/bin/upgrade_app /tmp/update.zip");
        DBG_UPG_CONTROL_DEFAULT(("usb upgrade step2:command is:%s\n", ps_cmd));
        i4_ret = system(ps_cmd);
        if (i4_ret != UPG_CONTROL_OK)
        {
            DBG_UPG_CONTROL_DEFAULT(("Err:step2:USB upgrade exec command @Line fail %d\n", __LINE__));
            return UPG_CONTROL_FAIL;
        }
        else
        {
            DBG_UPG_CONTROL_DEFAULT(("OK: step2:USB upgrade exec command @Line sucess %d\n", __LINE__));
            i4_ret = system("reboot");//reboot
            if(i4_ret != UPG_CONTROL_OK)
            {
              DBG_UPG_CONTROL_DEFAULT(("usb upgrade reboot fail @Line fail %d\n", __LINE__));
              return UPG_CONTROL_FAIL;
            }
        }
    #endif
    }

    return UPG_CONTROL_OK;
}


static void _upgrade_usb_state_process(boolean b_mnt)
{
    if (b_mnt)
    {
        DBG_UPG_CONTROL_INFO(("[lei_usb]:2 mount sucess.\n"));
        _upgrade_handle_usb_mount_msg();
    }
    else
    {
        DBG_UPG_CONTROL_INFO(("[lei_usb]:-2 umount sucess.\n"));
        DBG_UPG_CONTROL_DEFAULT(("usb didn't mount"));
    }

}

static void _upgrade_usb_dm_msg_process(const void *pv_msg, const size_t z_msg_len)
{
    DM_MNT_MSG_T *pt_mnt_msg = (DM_MNT_MSG_T *)pv_msg;

    if (z_msg_len != sizeof(DM_MNT_MSG_T))
    {
        DBG_UPG_CONTROL_DEFAULT(("dm's msg size[%d] is wrong, should be [%d]!\n", z_msg_len, sizeof(DM_MNT_MSG_T)));
        return;
    }
    DBG_UPG_CONTROL_DEFAULT(("[dm's broadcast]event type:%d\n", pt_mnt_msg->e_dm_event));

    switch (pt_mnt_msg->e_dm_event)
    {
    case DM_DEV_EVT_MOUNTED:
            _upgrade_usb_state_process(TRUE);
            break;
    case DM_DEV_EVT_UNMOUNTED:
            _upgrade_usb_state_process(FALSE);
            break;
    default:
            break;
    }
}
#else
static void _upgrade_usb_dm_msg_process(const void *pv_msg, const size_t z_msg_len)
{
    DBG_UPG_CONTROL_DEFAULT(("Auto upgrade from usb not enable\n"));

    return;
}
#endif /* CONFIG_SUPPORT_UPG_FROM_USB */




int32 _upg_control_start_check_update_timer(void)
{
    int32 i4_ret = 0;
    TIMER_TYPE_T t_timer;

    DBG_UPG_CONTROL_DEFAULT(("_upgrade_start_check_update_timer.\n"));

    memset(&t_timer, 0, sizeof(t_timer));

    u_timer_stop(h_check_update_timer);

    t_timer.h_timer = h_check_update_timer;
    t_timer.e_flags = X_TIMER_FLAG_REPEAT;
    t_timer.ui4_delay = UPG_CHECK_UPDATE_TIMEOUT;
    E_UPG_CONTROL_TIMER_MSG  e_timer_type = E_UPG_CONTROL_CHECK_UPDATE;

    i4_ret = u_timer_start(t_g_upg_control.h_app, &t_timer, (void *)&e_timer_type, sizeof(TIMER_TYPE_T));
    if(i4_ret != UPG_CONTROL_OK)
        DBG_UPG_CONTROL_DEFAULT(("start check version timer fail.\n"));

    return UPG_CONTROL_OK;
}

#endif


#ifdef ISNEED
void _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS status)
{
    DBG_UPG_CONTROL_INFO(("set upgrade status:%d, last status:%d\n", status, t_g_upg_control.i4_upgrade_status));
    t_g_upg_control.i4_upgrade_status = status;

    return;
}

E_UPG_CONTROL_UPGRADE_STATUS _upg_control_get_upgrade_status(void)
{
    E_UPG_CONTROL_UPGRADE_STATUS status = t_g_upg_control.i4_upgrade_status;

    return status;
}

int32 _upg_control_check_id_info(void)
{
    int32 i4_ret = 0;

    UPGRADE_FILE_INFO local_info, upg_info;

    memset(&local_info, 0, sizeof(UPGRADE_FILE_INFO));
    _upgrade_get_id_file_info(&local_info, UPDATE_FROM_CARD);

    memset(&upg_info, 0, sizeof(UPGRADE_FILE_INFO));
    _upgrade_get_id_file_info(&upg_info, UPDATE_FROM_NET);

    if (upg_info.is_force_update)
    {
        DBG_UPG_CONTROL_INFO(("force to upgrade\n"));
        /* TODO: do something? currently we just ignore version */
        return 0;
    }

    if (local_info.upgrade_file_version >= upg_info.upgrade_file_version)
    {
        DBG_UPG_CONTROL_INFO(("current system is up-to-date\n"));
        return -1;
    }

    return i4_ret;

}

static int32 _upg_control_check_update_timer(void)
{
    int32 i4_ret = 0;
    time_t current_time = time(NULL);
    struct tm *local_time = NULL;
    E_UPG_CONTROL_UPGRADE_STATUS upg_status = 0;

    local_time = localtime(&current_time);
    DBG_UPG_CONTROL_INFO(("current time, hour: %d\n",local_time->tm_hour));
    if ((local_time->tm_hour >= UPG_START_TIME) && 
        (local_time->tm_hour <= UPG_END_TIME))
    {
        upg_status = _upg_control_get_upgrade_status();
        if (E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING == upg_status)
        {
            DBG_UPG_CONTROL_INFO(("system is under upgrading\n"));
            return i4_ret;
        }
        else if (E_UPG_CONTROL_UPGRADE_STATUS_DONE == upg_status)
        {
            DBG_UPG_CONTROL_INFO(("upgrade done, need to reboot system\n"));
            return i4_ret;
        }

        _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING);

        i4_ret = _upg_control_check_id_info();
        if (i4_ret)
        {
            _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_FAILED);
            DBG_UPG_CONTROL_DEFAULT(("_upg_control_check_id_info failed\n"));
            return i4_ret;
        }

        i4_ret = _upg_control_upgrade_download_fw();
        if (i4_ret)
        {
            _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_FAILED);
            DBG_UPG_CONTROL_DEFAULT(("_upg_control_upgrade_download_fw failed\n"));
        }
    }

    return i4_ret;

}

static void _upg_control_flash_img_done(void)
{
    int32 i4_ret = 0;
    char old_id_file[BUF_SIZE] = {0};
    char new_id_file[BUF_SIZE] = {0};
    char cmd_buf[BUF_SIZE] = {0};

    strncpy(&old_id_file, UPGRADE_ID_FILE_SAVE_PATH, strlen(UPGRADE_ID_FILE_SAVE_PATH));
    strncat(&old_id_file, UPGRADE_ID_FILE_NAME, strlen(UPGRADE_ID_FILE_NAME));
    strncpy(&new_id_file, UPGRADE_ID_FILE_TMP_PATH, strlen(UPGRADE_ID_FILE_TMP_PATH));
    strncat(&new_id_file, UPGRADE_ID_FILE_NAME, strlen(UPGRADE_ID_FILE_NAME));

    if (NULL == access(new_id_file, F_OK))
    {
        snprintf(cmd_buf, 256, "mv -f %s %s", new_id_file, old_id_file);
        i4_ret = system(cmd_buf);
        if (i4_ret)
            DBG_UPG_CONTROL_DEFAULT(("storage new id file failed\n"));
        else
            DBG_UPG_CONTROL_INFO(("storage new id file to %s\n", old_id_file));
    }

    DBG_UPG_TRIGGER_DEFAULT(("upgrade done! now reboot!\n"));

    _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_DONE);

    system("reboot");

}

static int32 _upg_control_handle_timer_msg(APPMSG_T *app_msg)
{
    int32 i4_ret = 0;
    E_UPG_CONTROL_TIMER_MSG e_timer_type = *(E_UPG_CONTROL_TIMER_MSG*)app_msg;
    switch (e_timer_type)
    {
    case E_UPG_CONTROL_CHECK_UPDATE:
        {
            DBG_UPG_CONTROL_DEFAULT(("check update timer up!\n"));
            _upg_control_check_update_timer();
            break;
        }
    default:
            break;
    }

    return i4_ret;
}

static int32 _upg_control_handle_upg_ctrl_msg(APPMSG_T *app_msg)
{
    int32 i4_ret = 0;
    E_UPG_CONTROL_MSG_TYPE e_upg_ctrl_type = 0;

    e_upg_ctrl_type = (E_UPG_CONTROL_MSG_TYPE)app_msg->ui4_msg_type;
    DBG_UPG_CONTROL_DEFAULT(("recieve state mngr msg,type:%d\n",e_upg_ctrl_type));

    switch (e_upg_ctrl_type)
    {
    case E_UPG_CONTROL_DOWNLOAD_FW:
        {
            DBG_UPG_CONTROL_DEFAULT(("request to download firmware\n"));
            _upg_control_create_download_thread();
            break;
        }
    case E_UPG_CONTROL_FLASH_IMG_DATA:
        {
            DBG_UPG_CONTROL_DEFAULT(("request to flash image data\n"));
            UPGRADE_DL_MSG *upg_dl_msg = (UPGRADE_DL_MSG *)app_msg;
            _upg_control_flash_img_data(upg_dl_msg->dl_param);
            break;
        }
    case E_UPG_CONTROL_FLASH_IMG_DONE:
        {
            DBG_UPG_CONTROL_DEFAULT(("flash image done!\n"));
            _upg_control_flash_img_done();
            break;
        }
    default:
            break;
    }

    return i4_ret;
}

static int32 _upg_control_handle_state_mngr_msg(APPMSG_T *app_msg)
{
    int32 i4_ret = 0;
    E_UPG_CONTROL_UPGRADE_STATUS status = 0;
    SM_MSG_BDY_E sm_type = app_msg->ui4_msg_type;

    switch (sm_type)
    {
    case SM_BODY_UPG_START:
    case SM_BODY_UPG_STOP:
         {
            DBG_UPG_CONTROL_DEFAULT(("receive sm power key msg!\n"));
            status = _upg_control_get_upgrade_status();
            if(status == E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING)
                _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_CANCELLED);
            break;
         }
    default:
            break;
    }

    return i4_ret;
}

static int32 _upg_control_process_msg (
                    HANDLE_T                    h_app,
                    Uint32                      ui4_type,
                    const void*                 pv_msg,
                    size_t                      z_msg_len,
                    boolean                        b_paused
                    )
{
    Uchar *pc_name;
    const char *pc_keysta, *pc_keyval;
    APPMSG_T *app_msg = (APPMSG_T *)pv_msg;

    if (!(t_g_upg_control.b_app_init))
    {
        return AEER_FAIL;
    }
    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        switch (ui4_type)
        {
        case E_APP_MSG_TYPE_TIMER:
            {
                DBG_UPG_CONTROL_DEFAULT(("receive timer callback!\n"));
                _upg_control_handle_timer_msg(app_msg);
                break;
            }
        case E_APP_MSG_TYPE_UPG_CTRL:
            {
                DBG_UPG_CONTROL_DEFAULT(("receive upg control msg!\n"));
                _upg_control_handle_upg_ctrl_msg(app_msg);
                break;
            }
        case E_APP_MSG_TYPE_STATE_MNGR:
            {
                DBG_UPG_CONTROL_DEFAULT(("receive state manager msg!\n"));
                _upg_control_handle_state_mngr_msg(app_msg);
                break;
            }
        default:
                break;
        }
    }
    else
    {
        switch (ui4_type)
        {
        case E_APP_MSG_TYPE_DUT_STATE:
             {
                    // should get power on/off msg here
                    break;
             }

        case E_APP_MSG_TYPE_USB_DEV:
             {
                    DBG_UPG_CONTROL_DEFAULT(("get usb mount/umount msg\n"));
                    // should get usb plug in /out, file system mount/unmount msg.
                    _upgrade_usb_dm_msg_process(pv_msg, z_msg_len);
                    break;
             }

        default:
                break;
        }
    }
    return AEER_OK;
}

int32 _upg_control_set_server_url(const char *ps_url)
{
    _upgrade_set_server_url(ps_url);

    return UPG_CONTROL_OK;

}

int32 _upg_control_call_app_to_flash_data(UPGRADE_IMAGE_INFO *imginfo, int32 unflashed_size, boolean is_last_flash_unit)
{
    int32 i4_ret = 0;
    UPGRADE_IMAGE_INFO *img_info = NULL;
    char *flash_img_name = NULL;
    int32 flash_offset = 0;
    boolean  is_all_img_flashed = FALSE;
    char  upg_app_cmd[BUF_SIZE_HALF]={0};

    img_info = imginfo;
    flash_img_name = img_info->img_header.file_name;
    flash_offset = img_info->img_flashed_size;

    snprintf(upg_app_cmd, sizeof(upg_app_cmd), "upgrade_app %s %d %d %d", flash_img_name, flash_offset, unflashed_size, is_last_flash_unit);
    DBG_UPG_CONTROL_INFO(("call upg app: %s\n", upg_app_cmd));
    i4_ret = system(upg_app_cmd);
    DBG_UPG_CONTROL_INFO(("upgrade_app return: %d\n", i4_ret));

    return i4_ret;
}

int32 _upg_control_fully_flash_img_data(DOWNLOAD_TREAD_PARAM *dl_param)
{
    int32 i4_ret = 0;
    char upg_app_cmd[BUF_SIZE_HALF]={0};

    snprintf(upg_app_cmd, sizeof(upg_app_cmd), "upgrade_app %s", dl_param->save_path);
    DBG_UPG_CONTROL_INFO(("call upg app: %s\n", upg_app_cmd));
    i4_ret = system(upg_app_cmd);
    DBG_UPG_CONTROL_INFO(("upgrade_app return: %d\n", i4_ret));

    _upgrade_download_wakeup_data_flash_done(dl_param, &dl_param->is_request_done);
    return i4_ret;
}

int32 _upg_control_sectionally_flash_img_data(DOWNLOAD_TREAD_PARAM *dl_param)
{
    int32 i4_ret = 0;
    UPGRADE_IMAGE_INFO *img_info = NULL;
    int32 flash_unit_size = 0;
    int32 unflashed_size = 0;
    boolean is_img_download_done = FALSE;
    boolean is_all_img_flashed = FALSE;

    img_info = &(dl_param->image[dl_param->processing_img_idx]);
    flash_unit_size = img_info->img_flash_unit_size;
    is_img_download_done = img_info->is_img_download_done;
    unflashed_size = img_info->img_downloaded_size - img_info->img_flashed_size;

    if (unflashed_size < flash_unit_size)
    {
        /* last data unit to be flashed */
        if (is_img_download_done)
        {
            is_all_img_flashed = ((dl_param->processing_img_idx + 1) == dl_param->img_num)?TRUE:FALSE;
            i4_ret = _upg_control_call_app_to_flash_data(img_info, unflashed_size, is_all_img_flashed);
            if (i4_ret)
            {
                DBG_UPG_CONTROL_DEFAULT(("_upg_control_call_app_to_flash_data failed, cancel download\n"));
                _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_CANCELLED);
                _upgrade_download_wakeup_data_flash_done(dl_param, &img_info->is_flash_unit_done);
                return i4_ret;
            }
            img_info->img_flashed_size += unflashed_size;
            img_info->is_img_flashed_done = TRUE;
            _upgrade_download_wakeup_data_flash_done(dl_param, &img_info->is_flash_unit_done);
        }
        else
        {
            DBG_UPG_CONTROL_INFO(("unflash data size %d is less than flash_unit_size %d, skip this time\n",unflashed_size,flash_unit_size));
            return i4_ret;
        }
    }
    else
    {
        if (is_img_download_done)
            is_all_img_flashed = ((dl_param->processing_img_idx + 1) == dl_param->img_num)?TRUE:FALSE;

        i4_ret = _upg_control_call_app_to_flash_data(img_info, flash_unit_size, is_all_img_flashed);
        if (i4_ret)
        {
            DBG_UPG_CONTROL_DEFAULT(("_upg_control_call_app_to_flash_data failed, cancel download\n"));
            _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_CANCELLED);
            _upgrade_download_wakeup_data_flash_done(dl_param, &img_info->is_flash_unit_done);
            return i4_ret;
        }
        img_info->img_flashed_size += flash_unit_size;
        _upgrade_download_wakeup_data_flash_done(dl_param, &img_info->is_flash_unit_done);
    }
    DBG_UPG_CONTROL_INFO(("img flashed/downloaded/total size:%d/%d/%d \n",
        img_info->img_flashed_size,img_info->img_downloaded_size,img_info->img_header.comp_size));

    return i4_ret;

}

int32 _upg_control_flash_img_data(DOWNLOAD_TREAD_PARAM *dl_param)
{
    int32 i4_ret = 0;
    boolean is_full_pkg = dl_param->is_full_pkg_update;

    if (is_full_pkg)
    {
        i4_ret = _upg_control_fully_flash_img_data(dl_param);
    }
    else
    {
        i4_ret = _upg_control_sectionally_flash_img_data(dl_param);
    }
    return i4_ret;
}

int32 _upg_control_upgrade_download_fw_thread(void)
{
    int32 i4_ret = 0;

    i4_ret = _upgrade_start_download_firmware(UPDATE_FROM_NET, NULL, NULL);
    if (i4_ret)
    {
        DBG_UPG_CONTROL_DEFAULT(("_upgrade_start_download_firmware failed\n"));
        _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS_FAILED);
    }
    return UPG_CONTROL_OK;
}

int32 _upg_control_create_download_thread(void)
{
    int32 i4_ret = 0;
    HANDLE_T h_thread = NULL_HANDLE;

    i4_ret = u_thread_create(&h_thread,
                             UPG_CONTROL_DOWNLOAD_THREAD_NAME,
                             UPG_CONTROL_DOWNLOAD_THREAD_STACK_SIZE,
                             UPG_CONTROL_DOWNLOAD_THREAD_THREAD_PRIORATY,
                             _upg_control_upgrade_download_fw_thread,
                             0,
                             NULL);
    if (i4_ret)
    {
        DBG_UPG_CONTROL_DEFAULT(("_upg_control_create_download_thread failed\n"));
    }
    return i4_ret;
}

int32 _upg_control_upgrade_download_fw(void)
{
    int32 i4_ret = 0;
    APPMSG_T pt_msg;

    pt_msg.ui4_sender_id = MSG_FROM_UPG_CONTROL;
    pt_msg.ui4_msg_type = E_UPG_CONTROL_DOWNLOAD_FW;

    i4_ret = _upg_control_send_msg(&pt_msg, UPG_CONTROL_THREAD_NAME);
    if (i4_ret)
    {
        DBG_UPG_TRIGGER_DEFAULT(("_upg_control_send_msg failed:%d\n", i4_ret));
    }
    return i4_ret;
}




#endif

static int32 Aawant_Check_Update_Timer(void)
{
#ifdef ISNEED
    int32 i4_ret = 0;
    time_t current_time = time(NULL);
    struct tm *local_time = NULL;
    E_UPG_CONTROL_UPGRADE_STATUS upg_status = 0;

    local_time = localtime(&current_time);
    printf("current time, hour: %d\n",local_time->tm_hour);
    if ((local_time->tm_hour >= UPG_START_TIME) &&
        (local_time->tm_hour <= UPG_END_TIME))
    {
        upg_status = Aawant_Get_Upgrade_Status();
        if (E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING == upg_status)
        {
            printf("system is under upgrading\n");
            return i4_ret;
        }
        else if (E_UPG_CONTROL_UPGRADE_STATUS_DONE == upg_status)
        {
            printf("upgrade done, need to reboot system\n");
            return i4_ret;
        }

        Aawant_Set_Upgrade_Status(E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING);



        i4_ret = _upg_control_upgrade_download_fw();
        if (i4_ret)
        {
            Aawant_Set_Upgrade_Status(E_UPG_CONTROL_UPGRADE_STATUS_FAILED);
            printf("_upg_control_upgrade_download_fw failed\n");
        }
    }

    return i4_ret;
#endif
}

/**
 * 调用第三方app写入数据
 * @param imginfo
 * @param unflashed_size
 * @param is_last_flash_unit
 * @return
 */

int32 AawantCmd_Call_App_To_Flash_Data(UPGRADE_IMAGE_INFO *imginfo, int32 unflashed_size, boolean is_last_flash_unit)
{
    int32 i4_ret = 0;
    UPGRADE_IMAGE_INFO *img_info = NULL;
    char *flash_img_name = NULL;
    int32 flash_offset = 0;
    boolean  is_all_img_flashed = False;
    char  upg_app_cmd[BUF_SIZE_HALF]={0};

    img_info = imginfo;
    flash_img_name = img_info->img_header.file_name;
    flash_offset = img_info->img_flashed_size;

    snprintf(upg_app_cmd, sizeof(upg_app_cmd), "upgrade_app %s %d %d %d", flash_img_name, flash_offset, unflashed_size, is_last_flash_unit);
    printf("[%s]==>call upg app: %s\n",__FUNCTION__, upg_app_cmd);
    i4_ret = system(upg_app_cmd);
    printf("[%s]==>upgrade_app return: %d\n", i4_ret);

    return i4_ret;
}

/**
 *
 * @param dl_param
 * @return
 */
int32 Aawant_Fully_Flash_ImgData(DOWNLOAD_PARAM *dl_param)
{
    int32 i4_ret = 0;
    char upg_app_cmd[BUF_SIZE_HALF]={0};
#define PATH "/tmp/update"

    snprintf(upg_app_cmd, sizeof(upg_app_cmd), "upgrade_app %s", dl_param->save_path);
    printf("call upg app: %s\n", upg_app_cmd);

    if(0==access(PATH,F_OK))
    {

        if (0 == remove(PATH))
        {
            printf("remove existing %s \n", PATH);
        }
        else
        {
            printf("remove existing %s failed\n", PATH);
            return -1;
        }
    }
    i4_ret = system(upg_app_cmd);
    printf("[%s]==>upgrade_app return: %d\n",__FUNCTION__,i4_ret);

  //  Aawant_Wakeup_Data_Flash_Done(dl_param, &dl_param->is_request_done);
    return i4_ret;
}

int32 Aawant_Sectionally_Flash_ImgData(DOWNLOAD_PARAM *dl_param)
{
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

    if (unflashed_size < flash_unit_size)
    {
        /* last data unit to be flashed */
        if (is_img_download_done)
        {
            is_all_img_flashed = ((dl_param->processing_img_idx + 1) == dl_param->img_num)?True:False;
            i4_ret = AawantCmd_Call_App_To_Flash_Data(img_info, unflashed_size, is_all_img_flashed);
            if (i4_ret)
            {
                printf("[%s:%d]==>call_app_to_flash_data failed, cancel download\n",__FUNCTION__,__LINE__);
                Aawant_Set_Upgrade_Status(dl_param,AAW_CTL_DOWNLOAD_CANCEL);
                Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
                return i4_ret;
            }
            img_info->img_flashed_size += unflashed_size;
            img_info->is_img_flashed_done = True;
            Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
        }
        else
        {
            printf("[%s]==>unflash data size %d is less than flash_unit_size %d, skip this time\n",__FUNCTION__,unflashed_size,flash_unit_size);
            return i4_ret;
        }
    }
    else
    {
        if (is_img_download_done)
            is_all_img_flashed = ((dl_param->processing_img_idx + 1) == dl_param->img_num)?True:False;

        i4_ret = AawantCmd_Call_App_To_Flash_Data(img_info, flash_unit_size, is_all_img_flashed);
        if (i4_ret)
        {
            printf("[%s]==>Call_App_To_Flash_Data failed, cancel download\n",__FUNCTION__);
            Aawant_Set_Upgrade_Status(dl_param,AAW_CTL_DOWNLOAD_CANCEL);
            Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
            return i4_ret;
        }
        img_info->img_flashed_size += flash_unit_size;
        Aawant_Wakeup_Data_Flash_Done(dl_param, &img_info->is_flash_unit_done);
    }
    printf("[%s]==>img flashed/downloaded/total size:%d/%d/%d \n",__FUNCTION__,
            img_info->img_flashed_size,img_info->img_downloaded_size,img_info->img_header.comp_size);

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

int32 AawantCmd_Flash_ImgData(DOWNLOAD_PARAM *dl_param)
{
    int32 i4_ret = 0;
    boolean is_full_pkg = dl_param->is_full_pkg_update;
    Aawant_Set_Upgrade_Status(dl_param,AAW_CTL_UPGRADE_DOING);
//#define ZIP_PATH  "/tmp/update.zip"
    mprintf("------------------[%s][Start]---------------------\n",__FUNCTION__);
    if (is_full_pkg)
    {


        i4_ret = Aawant_Fully_Flash_ImgData(dl_param);
        mprintf("------------------[%s][Finish]--------------------\n",__FUNCTION__);
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
    }
    else
    {

        i4_ret = Aawant_Sectionally_Flash_ImgData(dl_param);
        mprintf("[%s]==>Sectionally_Flash_ImgData Finish\n",__FUNCTION__);
    }
    if(i4_ret==-1){
        Aawant_Set_Upgrade_Status(dl_param,AAW_CTL_UPGRADE_FAIL);
    } else{
        Aawant_Set_Upgrade_Status(dl_param,AAW_CTL_UPGRADE_SUCESS);
    }
    return i4_ret;
}

void Aawant_Set_Upgrade_Status(DOWNLOAD_PARAM *dl,AAWANT_UPG_CTL_STATUS status){
    pthread_mutex_lock(&dl->status_mutex);
    aa_status=status;

    mprintf("[%s]==>current status:%d\n",__FUNCTION__,aa_status);
    pthread_mutex_unlock(&dl->status_mutex);
}


//E_UPG_CONTROL_UPGRADE_STATUS Aawant_Get_Upgrade_Status(void){
AAWANT_UPG_CTL_STATUS Aawant_Get_Upgrade_Status(void){
    mprintf("[%s]==>current status:%d\n",__FUNCTION__,aa_status);

    return aa_status;
}


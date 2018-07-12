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
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.    
 *                                                                             
 * The following software/firmware and/or related documentation ("MediaTek     
 * Software") have been modified by MediaTek Inc. All revisions are subject to 
 * any receiver's applicable license agreements with MediaTek Inc.             
 */


#include <string.h>

//#include "u_dbg.h"
//#include "u_aee.h"
//#include "u_cli.h"
//#include "u_app_thread.h"
#include "upg_control_cli.h"
#include "upg_control.h"

#define URL_LENGTH 256

#ifdef ISNEED
static uint16 ui2_g_upg_control_dbg_level = DBG_INIT_LEVEL;

uint16 upg_control_get_dbg_level(void)
{
    return (ui2_g_upg_control_dbg_level | DBG_LAYER_APP);
}

#endif

#ifdef CLI_SUPPORT
void upg_control_set_dbg_level(uint16 ui2_db_level)
{
    ui2_g_upg_control_dbg_level = ui2_db_level;
}

static INT32 _upg_control_cli_get_dbg_level (INT32 i4_argc, const CHAR **pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(upg_control_get_dbg_level());

    return i4_ret;
}

static INT32 _upg_control_cli_set_dbg_level (INT32 i4_argc, const CHAR **pps_argv)
{
    uint16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        upg_control_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}

static INT32 _upg_control_update_test (INT32 i4_argc, const CHAR **pps_argv)
{
    INT32  i4_ret = 0;
    E_UPG_CONTROL_UPGRADE_STATUS status = 0;

    status = _upg_control_get_upgrade_status();
    if (E_UPG_CONTROL_UPGRADE_STATUS_UPGRADING == status)
    {
        DBG_UPG_CONTROL_DEFAULT(("system is now upgrading.\n"));
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
        return i4_ret;
    }

    return i4_ret;
}

static INT32 upg_control_cli_set_server_url (INT32 i4_argc, const CHAR **pps_argv)
{
    INT32 i4_ret = UPG_CONTROL_OK;
    CHAR  c_url[URL_LENGTH] = {0};

    strcpy(c_url, pps_argv[1]);
    i4_ret = _upg_control_set_server_url(c_url);
    if (i4_ret != UPG_CONTROL_OK)
    {
        DBG_UPG_CONTROL_DEFAULT((" _upg_control_set_server_url fail\n"));
        return UPG_CONTROL_FAIL;
    }

}

/* main command table */
static CLI_EXEC_T at_upg_control_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _upg_control_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _upg_control_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "ota_dl",
        NULL,
        _upg_control_update_test,
        NULL,
        "ota_dl",
        CLI_GUEST
    },
    {
        "set_url",
        NULL,
        upg_control_cli_set_server_url,
        NULL,
        "set_url",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

static CLI_EXEC_T at_upg_control_root_cmd_tbl[] =
{
    {
        "upg_ctrl",
        "upg_ctrl",
        NULL,
        at_upg_control_cmd_tbl,
        "upg_control commands",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

INT32 upg_control_cli_attach_cmd_tbl(void)
{
    return (u_cli_attach_cmd_tbl(at_upg_control_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}

#endif /* CLI_SUPPORT */

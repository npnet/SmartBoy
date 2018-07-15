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


#ifndef _UPG_CONTROL_H_
#define _UPG_CONTROL_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
//#include "u_common.h"
//#include "u_dbg.h"
//#include "u_appman.h"

#include "common.h"
#include "aawant.h"
#include "upg_utility.h"
//#include "upg_download.h"


#undef  DBG_INIT_LEVEL
#define DBG_INIT_LEVEL          (DBG_LEVEL_ERROR|DBG_LEVEL_API|DBG_LEVEL_INFO | DBG_LAYER_APP)

#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       upg_control_get_dbg_level()

#define UPG_CONTROL_TAG                "<UPG_CTRL>"
#define DBG_UPG_CONTROL_INFO(_stmt)    do{DBG_INFO((UPG_CONTROL_TAG));DBG_INFO(_stmt);}while(0)
#define DBG_UPG_CONTROL_API(_stmt)     do{DBG_API((UPG_CONTROL_TAG));DBG_API(_stmt);}while(0)
#define DBG_UPG_CONTROL_DEFAULT(_stmt)    do{DBG_ERROR((UPG_CONTROL_TAG));DBG_ERROR(_stmt);}while(0)
#define DBG_UPG_CONTROL_ERR(_stmt)                                          \
do{                                                                         \
    DBG_ERROR((UPG_CONTROL_TAG));                                           \
    DBG_ERROR(("(%s:%s:%d)-->",__FILE__,__func__,__LINE__));                \
    DBG_ERROR(_stmt);                                                       \
}while(0)

#define UPG_CONTROL_ASSERT(err) \
    do{                 \
        if(err<0){      \
            DBG_ERROR(("<upg><ASSERT> %s L%d err:%d\n",__FUNCTION__,__LINE__,err)); \
            while(1);   \
        }               \
    }while(0)

#define UPG_CONTROL_OK                   ((int32)    0)        /* OK */
#define UPG_CONTROL_FAIL                 ((int32)   -1)        /* Something error? */
#define UPG_CONTROL_INV_ARG              ((int32)   -2)        /* Invalid arguemnt. */

#define CONFIG_SUPPORT_UPG_FROM_USB 0


/* define value*/
#define UPG_CHECK_UPDATE_TIMEOUT                        (1000*60*10) /* 10min */
#define UPG_CONTROL_DOWNLOAD_THREAD_NAME                "upg_control_download"
#define UPG_CONTROL_DOWNLOAD_THREAD_STACK_SIZE          (1024 * 40)
#define UPG_CONTROL_DOWNLOAD_THREAD_THREAD_PRIORATY     PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)

/* application structure */
#ifdef ISNEED
typedef struct _UPG_CONTROL_OBJ_T
{
    HANDLE_T        h_app;
    boolean            b_app_init;
    int32           i4_power_mode;
    int32           i4_power_status;
    int32           i4_playback_source;
    int32           i4_playback_status;
    int32           i4_network_status;
    int32           i4_upgrade_status;
}UPG_CONTROL_OBJ_T;
#endif
/*app private  msg*/




/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
int32 _upg_control_upgrade_download_fw(void);
int32 _upg_control_check_id_info(void);
void _upg_control_set_upgrade_status(E_UPG_CONTROL_UPGRADE_STATUS status);
E_UPG_CONTROL_UPGRADE_STATUS _upg_control_get_upgrade_status(void);

/*
 *
 */
void Aawant_Set_Upgrade_Status(E_UPG_CONTROL_UPGRADE_STATUS status);
E_UPG_CONTROL_UPGRADE_STATUS Aawant_Get_Upgrade_Status(void);


//int32 createDownloadPthread(DOWNLOAD_PARAM dl);
#endif /* _UPG_CONTROL_H_ */

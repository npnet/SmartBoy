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


#ifndef _UPG_TRIGGER_H_
#define _UPG_TRIGGER_H_
/*-----------------------------------------------------------------------------
                                            include files
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <pthread.h>


#include "common.h"
//#include "u_common.h"
//#include "u_dbg.h"
#include "curl/curl.h"
#include "upg_utility.h"
#include "upg_control.h"

#ifdef ISNEED

#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       DBG_LEVEL_ALL

#define UPG_TRIGGER_TAG                "<UPG_CTRL>"
#define DBG_UPG_TRIGGER_INFO(_stmt)    do{DBG_INFO((UPG_TRIGGER_TAG));DBG_INFO(_stmt);}while(0)
#define DBG_UPG_TRIGGER_API(_stmt)     do{DBG_API((UPG_TRIGGER_TAG));DBG_API(_stmt);}while(0)
#define DBG_UPG_TRIGGER_DEFAULT(_stmt) do{DBG_ERROR((UPG_TRIGGER_TAG));DBG_ERROR(_stmt);}while(0)
#define DBG_UPG_TRIGGER_ERR(_stmt)                                          \
do{                                                                         \
    DBG_ERROR((UPG_TRIGGER_TAG));                                           \
    DBG_ERROR(("(%s:%s:%d)-->",__FILE__,__func__,__LINE__));                \
    DBG_ERROR(_stmt);                                                       \
}while(0)

#define UPG_TRIGGER_ASSERT(err) \
    do{                 \
        if(err<0){      \
            DBG_ERROR(("<upg><ASSERT> %s L%d err:%d\n",__FUNCTION__,__LINE__,err)); \
            while(1);   \
        }               \
    }while(0)

#endif

//#define AAWANT_DEBUG
#define UPG_TRIGGER_OK                   ((int)    0)        /* OK */
#define UPG_TRIGGER_FAIL                 ((int)   -1)        /* Something error? */
#define UPG_TRIGGER_INV_ARG              ((int)   -2)        /* Invalid arguemnt. */



/*app private  msg*/




#ifdef ISNEED

typedef struct UPGRADE_DL_MSG_T
{
    APPMSG_T pt_msg;
    DOWNLOAD_TREAD_PARAM* dl_param;
}UPGRADE_DL_MSG;

#endif

typedef size_t (*CURL_CB_FCT)(void*,size_t,size_t,void*);

/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
int32 _upgrade_download_wakeup_data_flash_done(DOWNLOAD_TREAD_PARAM* dl_param, boolean* is_done);
//#ifdef aawant
int32 _upgrade_start_download_firmware(uint32 t_delivery_type, const char* ps_url, const char* ps_fwPath);
//#else
int32 _upgrade_start_download_firmware(const char* ps_url, const char* ps_fwPath);
//#endif

char* _upgrade_get_save_path(const char *ps_save_path);
char* _upgrade_get_server_url(const char *url);


//-------------
double _upgrade_get_download_file_lenth(CURL *handle, const char *url);
int32 Aawant_Wakeup_Data_Flash_Done(DOWNLOAD_PARAM *dl_param, boolean *is_done);
int32 Aawant_StartDownLoad(DOWNLOAD_PARAM dl,char *g_url,char *save_path,boolean is_full_pkg);

#endif /* _UPG_TRIGGER_H_ */

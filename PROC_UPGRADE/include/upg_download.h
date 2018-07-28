#ifndef _UPG_TRIGGER_H_
#define _UPG_TRIGGER_H_

#include <stdio.h>
#include <pthread.h>
#include "common.h"
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


typedef size_t (*CURL_CB_FCT)(void *, size_t, size_t, void *);

//-------------
double _upgrade_get_download_file_lenth(CURL *handle, const char *url);

int32 Aawant_Wakeup_Data_Flash_Done(DOWNLOAD_PARAM *dl_param, boolean *is_done);

int32 Aawant_StartDownLoad(DOWNLOAD_PARAM dl, char *g_url, char *save_path, boolean is_full_pkg);
int32 StartDownLoad2(DOWNLOAD_PARAM dl, char *g_url, char *save_path);

#endif /* _UPG_TRIGGER_H_ */

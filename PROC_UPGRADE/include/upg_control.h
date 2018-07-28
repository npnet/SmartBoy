#ifndef _UPG_CONTROL_H_
#define _UPG_CONTROL_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "common.h"
#include "aawant.h"
#include "upg_utility.h"



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


void Aawant_Set_Upgrade_Status(DOWNLOAD_PARAM *dl, AAWANT_UPG_CTL_STATUS status);

AAWANT_UPG_CTL_STATUS Aawant_Get_Upgrade_Status(void);

int32 AawantCmd_Flash_ImgData(DOWNLOAD_PARAM *dl_param);
int32 FlashImgData(DOWNLOAD_PARAM *dl_param);

void SetUpgradeAction(DOWNLOAD_PARAM *dl,UPG_ACTION action);
UPG_ACTION GetUpgradeAction(DOWNLOAD_PARAM *dl);

void SetMainProcessIntent(DOWNLOAD_PARAM *dl,WantMeToDo intent);
WantMeToDo GetMainProcessIntent();
//int32 createDownloadPthread(DOWNLOAD_PARAM dl);
#endif /* _UPG_CONTROL_H_ */

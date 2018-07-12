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


#ifndef _UPG_UTILITY_H_
#define _UPG_UTILITY_H_

#include <sys/ioctl.h>
//#include "u_common.h"

#include "common.h"

/*-------------------------------------------------------------------------------------
					for zip header info -begin
-------------------------------------------------------------------------------------*/

#define ZIP_HEAD_PRE_SIZE 	128         /* update.bin head size, not much accurate, but we got what we need */
#define ZIP_HEAD_SIZE_ROW 	30          /* total_header_size = row_size + file_name_len + extra_field_len */
#define ZIP_HEAD_SIGN 		0x04034b50

/*  zip -0 update.bin xxx.img xxx.img xxx.img
  *  zip struct :
  *     |------local file1 header------|-- file1 data--|--file1 data descriptor--|--local file2 header--|-- file2 data--|--file2 data descriptor--|--
  *       (row_size+name_len+ext_len)     (comp_size)      	(option)(12B/16B)
  */
typedef struct zip_header_raw
{
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
}ZIP_HEADER_RAW;

typedef struct zip_header
{
	int32   sign_head;
	int16	version;
	int16	gp_bit_flag;
	int16	comp_method;
	int16	mod_time;
	int16	mod_date;
	int32   crc32;
	int32   comp_size;
	int32   uncomp_size;
	int16	name_length;
	int16	ext_field_length;
	char	file_name[64];
}ZIP_HEADER;

/* seems we do not need this desc */
struct file_descriptor
{
	int32 crc32;
	int32 comp_size;
	int32 uncomp_size;
};

void parse_zip_head_info(const ZIP_HEADER_RAW* raw_data, ZIP_HEADER* data);


/*-------------------------------------------------------------------------------------
					for zip header info -end
-------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------
					for image partition info -begin
-------------------------------------------------------------------------------------*/

#define CONFIG_UPGRADE_USE_NAND_FLASH 1

#if CONFIG_UPGRADE_USE_NAND_FLASH

#define LK_PARTITION_NAME 		"UBOOT"
#define DTBO_PARTITION_NAME 	"DTBO"
#define MISC_PARTITION_NAME 	"MISC"
#define USRDATA_PARTITION_NAME 	"USRDATA"
#define A_TZ_PARTITION_NAME 	"TEE1"
#define B_TZ_PARTITION_NAME 	"TEE2"
#define A_BOOT_PARTITION_NAME 	"BOOTIMG1"
#define B_BOOT_PARTITION_NAME 	"BOOTIMG2"
#define A_ROOTFS_PARTITION_NAME "ROOTFS1"
#define B_ROOTFS_PARTITION_NAME "ROOTFS2"


#define BOOT_IMAGE_NAME		"boot.img"
#define TZ_IMAGE_NAME		"tz.img"
#define ROOTFS_IMAGE_NAME	"rootfs.ubi"
#define LK_IMAGE_NAME		"lk.img"

#define PROC_MTD_PATH	"/proc/mtd"
#endif /* CONFIG_UPGRADE_USE_NAND_FLASH */

int32 get_partition_blk_size_by_name(const char* img_name, int32* blk_size);

/*-------------------------------------------------------------------------------------
					for image partition info -end
-------------------------------------------------------------------------------------*/

#endif /* _UPG_UTILITY_H_  */

#ifndef _UPG_UTILITY_H_
#define _UPG_UTILITY_H_

#include <sys/ioctl.h>
//#include "u_common.h"

#include "common.h"
#include "aawant.h"

/*-------------------------------------------------------------------------------------
					for zip header info -begin
-------------------------------------------------------------------------------------*/

#define ZIP_HEAD_PRE_SIZE    128         /* update.bin head size, not much accurate, but we got what we need */
#define ZIP_HEAD_SIZE_ROW    30          /* total_header_size = row_size + file_name_len + extra_field_len */
#define ZIP_HEAD_SIGN        0x04034b50

/*  zip -0 update.bin xxx.img xxx.img xxx.img
  *  zip struct :
  *     |------local file1 header------|-- file1 data--|--file1 data descriptor--|--local file2 header--|-- file2 data--|--file2 data descriptor--|--
  *       (row_size+name_len+ext_len)     (comp_size)      	(option)(12B/16B)
  */


/* seems we do not need this desc */
struct file_descriptor {
    int32 crc32;
    int32 comp_size;
    int32 uncomp_size;
};

void parse_zip_head_info(const ZIP_HEADER_RAW *raw_data, ZIP_HEADER *data);


/*-------------------------------------------------------------------------------------
					for zip header info -end
-------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------
					for image partition info -begin
-------------------------------------------------------------------------------------*/

#define CONFIG_UPGRADE_USE_NAND_FLASH 1

#if CONFIG_UPGRADE_USE_NAND_FLASH

#define LK_PARTITION_NAME        "UBOOT"
#define DTBO_PARTITION_NAME    "DTBO"
#define MISC_PARTITION_NAME    "MISC"
#define USRDATA_PARTITION_NAME    "USRDATA"
#define A_TZ_PARTITION_NAME    "TEE1"
#define B_TZ_PARTITION_NAME    "TEE2"
#define A_BOOT_PARTITION_NAME    "BOOTIMG1"
#define B_BOOT_PARTITION_NAME    "BOOTIMG2"
#define A_ROOTFS_PARTITION_NAME "ROOTFS1"
#define B_ROOTFS_PARTITION_NAME "ROOTFS2"


#define BOOT_IMAGE_NAME        "boot.img"
#define TZ_IMAGE_NAME        "tz.img"
#define ROOTFS_IMAGE_NAME    "rootfs.ubi"
#define LK_IMAGE_NAME        "lk.img"

#define PROC_MTD_PATH    "/proc/mtd"
#endif /* CONFIG_UPGRADE_USE_NAND_FLASH */

int32 get_partition_blk_size_by_name(const char *img_name, int32 *blk_size);

/*-------------------------------------------------------------------------------------
					for image partition info -end
-------------------------------------------------------------------------------------*/

#endif /* _UPG_UTILITY_H_  */

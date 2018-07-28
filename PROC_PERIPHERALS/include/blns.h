/*
 * blns.h
 *
 * blns hal driver header file
 *
 * Copyright 2017 cary @ aawant (www.aawant.com)
 *
 */
 
#ifndef CPLDLEDS_H
#define CPLDLEDS_H

#include <hardware/hardware.h>
#include <linux/ioctl.h>

#define      BLNS_HARDWARE_MODULE_ID      "blns"
#define      LOG_TAG                      "BLNS_DEBUG"

#define 	 AW9523B_IO_MAGIC          'k'
#define      AW9523B_IO_ERROR       _IOW(AW9523B_IO_MAGIC, 0, int)
#define      AW9523B_IO_VOLUME      _IOW(AW9523B_IO_MAGIC, 1, int)
#define      AW9523B_IO_OFF         _IOW(AW9523B_IO_MAGIC, 2, int)
#define      AW9523B_IO_START_UP    _IOW(AW9523B_IO_MAGIC, 3, int)
#define      AW9523B_IO_WAKEUP      _IOW(AW9523B_IO_MAGIC, 4, int)
#define      AW9523B_IO_GIFS        _IOW(AW9523B_IO_MAGIC, 5, int)
#define      AW9523B_IO_BROADCAST   _IOW(AW9523B_IO_MAGIC, 6, int)
#define      AW9523B_IO_PLAY        _IOW(AW9523B_IO_MAGIC, 7, int)
#define      AW9523B_IO_NETCONFIG   _IOW(AW9523B_IO_MAGIC, 8, int)
#define      AW9523B_IO_UPDATE      _IOW(AW9523B_IO_MAGIC, 9, int)
#define      AW9523B_IO_TEST        _IOW(AW9523B_IO_MAGIC, 10, int)
#define      AW9523B_IO_SWITCH_PLAY      _IOW(AW9523B_IO_MAGIC, 11, int)


struct blns_device_t {
    struct hw_device_t common;
			
    int (*set_bln_status)(struct blns_device_t* dev, int data);
			
    int (*command_blns)(struct blns_device_t* dev, int data);
			
};

#endif

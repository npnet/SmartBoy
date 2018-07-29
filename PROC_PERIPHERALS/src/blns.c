/*
 * blns.c
 *
 * blns hal driver source file
 *
 * Copyright 2017 cary @ aawant (www.aawant.com)
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <blns.h>

#define  DEBUG_
#define  LED         "/dev/blns"
#if 0
int fd;

static int close_blns(struct blns_device_t *dev)
{
	if(dev)
		free(dev);
	close(fd);
	return 0;
}
			
			
int write_blns(struct blns_device_t* dev, int data)
{
	int ret = 0;
	if(fd < 0 )
	{
		printf("OPEN DEVICE FAIL...\n");
		return -errno;
	}
	
#ifdef DEBUG_
	printf("write blns data -> %d", data);
#endif
	ret = write(fd, &data, sizeof(int));
	if(ret < 0)
	{
		printf("WRITE DATA FAIL...");
		return -errno;
	}
	
    return ret;	
}


int ioctl_blns(struct blns_device_t* dev, int data)
{
	int cmd;
	int sw = 0;
	
	if(fd < 0)
	{
		printf("OPEN DEVICE FAIL...\n");
		return -errno;
	}
#ifdef DEBUG_
	printf("write blns data -> %d\n", data);
#endif
	switch(data)
	{
		case 0:
			cmd = AW9523B_IO_ERROR;
			break;
		case 1:
			cmd = AW9523B_IO_VOLUME;
			break;
		case 2:
			cmd = AW9523B_IO_OFF;
			break;
		case 3:
			cmd = AW9523B_IO_START_UP;
			break;
		case 4:
			cmd = AW9523B_IO_WAKEUP;
			break;
		case 5:
			cmd = AW9523B_IO_GIFS;
			break;
		case 6:
			cmd = AW9523B_IO_BROADCAST;
			break;
		case 7:
			cmd = AW9523B_IO_PLAY;
			break;
		case 8:
			cmd = AW9523B_IO_NETCONFIG;
			break;
		case 9:
			cmd = AW9523B_IO_UPDATE;
			break;
		case 10:
			cmd = AW9523B_IO_TEST;
			break;
		case 11:
			cmd = AW9523B_IO_SWITCH_PLAY;
			break;
		default:
			cmd = AW9523B_IO_OFF;
			break;
	}
	
	if(ioctl(fd, cmd, &sw) < 0)
	{
		printf("IOCTL DATA FAIL...\n");
		return -errno;
	}
	return 0;
}


int open_blns()
{
	struct blns_device_t *dev = malloc(sizeof(struct blns_device_t));
	memset(dev, 0, sizeof(*dev));


	dev->set_bln_status = write_blns;
	dev->command_blns = ioctl_blns;
	*device = (struct hw_device_t *)dev;
	
	
	fd = open(DRIVER, O_RDWR);
	if(fd <0)
	{
		printf("OPEN DEVICE FAIL...");
		return -5;
	}
	return 0;
}

int ioctl_blns2(int fd, int data)
{
	int cmd;
	int sw = 0;

	if(fd < 0)
	{
		printf("OPEN DEVICE FAIL...\n");
		return -errno;
	}
#ifdef DEBUG_
	printf("write blns data -> %d\n", data);
#endif
	switch(data)
	{
		case 0:
			cmd = AW9523B_IO_ERROR;
			break;
		case 1:
			cmd = AW9523B_IO_VOLUME;
			break;
		case 2:
			cmd = AW9523B_IO_OFF;
			break;
		case 3:
			cmd = AW9523B_IO_START_UP;
			break;
		case 4:
			cmd = AW9523B_IO_WAKEUP;
			break;
		case 5:
			cmd = AW9523B_IO_GIFS;
			break;
		case 6:
			cmd = AW9523B_IO_BROADCAST;
			break;
		case 7:
			cmd = AW9523B_IO_PLAY;
			break;
		case 8:
			cmd = AW9523B_IO_NETCONFIG;
			break;
		case 9:
			cmd = AW9523B_IO_UPDATE;
			break;
		case 10:
			cmd = AW9523B_IO_TEST;
			break;
		case 11:
			cmd = AW9523B_IO_SWITCH_PLAY;
			break;
		default:
			cmd = AW9523B_IO_OFF;
			break;
	}

	if(ioctl(fd, cmd, &sw) < 0)
	{
		printf("IOCTL DATA FAIL...\n");
		return -errno;
	}
	return 0;
}

#endif
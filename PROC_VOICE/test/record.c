/* =========================================================================

DESCRIPTION
   uart

Copyright (c) 2015 by  USTC iFLYTEK, Co,LTD.  All Rights Reserved.
============================================================================ */

/* =========================================================================

                             REVISION

when            who              why
--------        ---------        -------------------------------------------
2015/10/15     kunzhang        Created.
2016/10/14     leqi            Modify for Record.
============================================================================ */
/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */

#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <alsa/asoundlib.h>  
#include "record.h"

#include "AudioQueue.h"
#include "error.h"
#include <sched.h>
#include <pthread.h>


/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */
#define ALSA_PCM_NEW_HW_PARAMS_API  
#define AUDIO_QUEUE_BUFF_LEN  (1024*10)

#define QUEUE_BUFF_MULTIPLE 1000

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */
typedef struct _RecordData{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	audio_queue_t *queue;
	char *queue_buff;
	char *buffer; 
	int buff_size;
	snd_pcm_uframes_t frames; 
	record_audio_fn cb;
	void *user_data;
	pthread_t tid_pcm_read;
	pthread_t tid_queue_read;
	int runing;
}RecordData;

/* ------------------------------------------------------------------------
** Global Variable Definitions
** ------------------------------------------------------------------------ */
static char *out_pcm_name;

/* ------------------------------------------------------------------------
** Function Definitions
** ------------------------------------------------------------------------ */ 
static void* QueueReadThread(void* param);
static void* RecordThread(void* param);
static void record_audio_cb(const void *audio, unsigned int audio_len, int err_code);
int start_record(struct pcm_config pcm_cfg);
void record_stop(void *record_hd);


static void* QueueReadThread(void* param)
{
	RecordData *record = (RecordData *)param;
	int readLen = 0;
	printf("QueueReadThread record :%x\n", record);
	while(record->runing){
		char *data_buff = NULL;
		readLen = queue_read(record->queue, &data_buff);
		
		if (0 == readLen){
			//printf("queue_read readLen = 0\n");
			//usleep(16000);
			//continue;
		}
		if (record->buff_size != readLen){
			//printf("\nqueue_read readLen %d\n", readLen);
		}
		record->cb(data_buff, readLen, SUCCESS);
		free(data_buff);
	}
	printf("QueueReadThread end \n");
	return NULL;
}


static void* RecordThread(void* param)
{
	RecordData *record = (RecordData *)param;
	int ret = 0;
	cpu_set_t mask;
	printf("RecordThread record:%x\n", record);
	//CPU_ZERO(&mask);
	//CPU_SET(0,&mask);
	ret = sched_setaffinity(0, sizeof(mask), &mask);
	printf("sched_setaffinity return = %d\n", ret);
	while (record->runing){
		ret = snd_pcm_readi(record->handle, record->buffer, record->frames); 
		//printf("snd_pcm_readi return = %d\n", ret);
		if (ret == -EPIPE) {  
			/* EPIPE means overrun */  
			fprintf(stderr, "overrun occurred/n");  
			snd_pcm_prepare(record->handle);  
			continue;
		} else if (ret < 0) {  
			fprintf(stderr,  
				"error from read: %s/n",  
				snd_strerror(ret)); 
			record->cb(NULL, 0, ret);
			return;
		} else if (ret != (int)record->frames) {  
			fprintf(stderr, "short read, read %d frames/n", ret);  
		}  
		queue_write(record->queue, record->buffer, record->buff_size);
	}
	printf("RecordThread end\n");
	return NULL;
}

static void record_audio_cb(const void *audio, unsigned int audio_len, int err_code)
{

	if (SUCCESS == err_code){
		FILE *fp = fopen(out_pcm_name, "ab+");
		if (NULL == fp){
			printf("fopen error\n");
			return;
		}
		fwrite(audio, audio_len, 1, fp);
		fclose(fp);
	}

}

int start_record(struct pcm_config pcm_cfg)
{
	int rc;  
	int size;  
	int ret = SUCCESS;
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	snd_pcm_hw_params_t *params;  
	unsigned int val,val2;  
	int dir;  
	snd_pcm_uframes_t frames;  
	char *buffer;  
    RecordData *record = NULL;
	void *audio_queue_buff = NULL;
	record = (RecordData *)malloc(sizeof(RecordData));	
	audio_queue_buff = malloc(sizeof(audio_queue_t) + AUDIO_QUEUE_BUFF_LEN + 1);
	if (NULL == record){
		return ERROR_OUT_OF_MEMORY;
	}
	memset(record, 0, sizeof(RecordData));
	record->cb = record_audio_cb;
	
	//设置录音参数参数
	rc = snd_pcm_open(&record->handle, pcm_cfg.device_name,SND_PCM_STREAM_CAPTURE, 0);  
	if (rc < 0) {  
		printf("unable to open pcm device: %s/n",  snd_strerror(rc));  
		ret = ERROR_OPEN_FILE;  
		goto error;
	}  
	/* Allocate a hardware parameters object. */  
	snd_pcm_hw_params_alloca(&params);  

	/* Fill it in with default values. */  
	snd_pcm_hw_params_any(record->handle, params);  

	/* Set the desired hardware parameters. */  
	/* Interleaved mode */  
	snd_pcm_hw_params_set_access(record->handle, params,  
		SND_PCM_ACCESS_RW_INTERLEAVED);  

	/* Signed 16-bit little-endian format */  
	snd_pcm_hw_params_set_format(record->handle, params,  
		SND_PCM_FORMAT_S24_LE);  

	/* Two channels (stereo) */  
	snd_pcm_hw_params_set_channels(record->handle, params, pcm_cfg.channels);  

	/* 44100 bits/second sampling rate (CD quality) */  
	val = pcm_cfg.rate;  
	snd_pcm_hw_params_set_rate_near(record->handle, params,  &val, &dir);  

	/* Set period size to  frames. */  
	frames = pcm_cfg.period_size;  
	snd_pcm_hw_params_set_period_size_near(record->handle,  params, &frames, &dir);  
	record->frames = frames;

	/* Write the parameters to the driver */  
	rc = snd_pcm_hw_params(record->handle, params);  
	if (rc < 0) {  
		printf("unable to set hw parameters: %s/n",  
			snd_strerror(rc));  
		return ERROR_FAIL;    
	}  
	/* Use a buffer large enough to hold one period */  
	snd_pcm_hw_params_get_period_size(params,  &frames, &dir);  
	size = frames * 8; /* 2 bytes/sample, 2 channels */  
	record->buff_size = size;
	printf("frames=%d,record->buff_size=%d\n", frames, record->buff_size);
	record->buffer = (char *) malloc(size);  
	if (NULL == record->buffer){
		ret = ERROR_OUT_OF_MEMORY;
		goto error;
	}

	record->queue_buff = (char *) malloc(sizeof(audio_queue_t) + size * QUEUE_BUFF_MULTIPLE + 1);
	if (NULL == record->queue_buff){
		ret = ERROR_OUT_OF_MEMORY;
		goto error;
	}
	record->queue = queue_init(record->queue_buff, size * QUEUE_BUFF_MULTIPLE + 1);
	pthread_attr_init(&thread_attr);
	pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
	thread_param.sched_priority = sched_get_priority_max(SCHED_RR);
	pthread_attr_setschedparam(&thread_attr, &thread_param);
	record->runing = 1;
	
	printf("record_start record :%x\n", record);
	pthread_create(&record->tid_pcm_read, &thread_attr, RecordThread, (void*)record);
	pthread_create(&record->tid_queue_read, NULL, QueueReadThread, (void*)record);
	
	while(1)
	{}
	
	
goto exit;

error:
	printf("record start error %d\n", ret);
	record_stop(record);

exit:
	printf("record start out %d\n", ret);
	free(audio_queue_buff);
	record_stop(record);
	return ret;
	
}
int main(int argc, char **argv)
{
	if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav [-D card] [-d device] [-c channels] "
                "[-r rate] [-b bits] [-p period_size] [-n n_periods]\n", argv[0]);
        return 1;
    }
	unsigned int card = 0;
    unsigned int device = 1;
    unsigned int channels = 8;
    unsigned int rate = 16000;

    unsigned int frames;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
   
	char sound_device_name[256];
	out_pcm_name = argv[1];
	struct pcm_config param_cfg;
	
	/* parse command line arguments */
    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-d") == 0) {
            argv++;
            if (*argv)
                device = atoi(*argv);
        } else if (strcmp(*argv, "-c") == 0) {
            argv++;
            if (*argv)
                channels = atoi(*argv);
        } else if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (*argv)
                rate = atoi(*argv);
        } else if (strcmp(*argv, "-D") == 0) {
            argv++;
            if (*argv)
                card = atoi(*argv);
        } else if (strcmp(*argv, "-p") == 0) {
            argv++;
            if (*argv)
                period_size = atoi(*argv);
        } else if (strcmp(*argv, "-n") == 0) {
            argv++;
            if (*argv)
                period_count = atoi(*argv);
        }
        if (*argv)
            argv++;
    }
	
	snprintf(sound_device_name, sizeof(sound_device_name), "hw:%u,%u",card, device);
	
	
	param_cfg.device_name = sound_device_name;
	param_cfg.channels = channels;
	param_cfg.rate = rate;
	param_cfg.period_size = period_size;
	param_cfg.period_count = period_count;
	
	printf("++++++++++++++++++++++++++\n");
	printf("device name is %s\n",param_cfg.device_name);
	printf("Channle is %d\n",param_cfg.channels);
	printf("rate name is %d\n",param_cfg.rate);
	printf("period_size name is %d\n",param_cfg.period_size);
	printf("++++++++++++++++++++++++++\n");
	
	start_record(param_cfg);
	
	return 0;
	//==========================================
	
	
}

void record_stop(void *record_hd)
{
	RecordData *record = (RecordData*)record_hd;
	printf("\nrecord_stop in record:%x\n\n", record);
	if (NULL != record){
		record->runing = 0; 
		pthread_join(record->tid_pcm_read, NULL);
		pthread_join(record->tid_queue_read, NULL);
		snd_pcm_drain(record->handle);  
		snd_pcm_close(record->handle); 

		if (NULL != record->queue){
			queue_destroy(record->queue);
		}
		if (NULL != record->queue_buff){
			free(record->queue_buff);
		}
		if (NULL != record->buffer){
			free(record->buffer);
		}
		free(record);
	}
	printf("\nrecord_stop out\n");
}
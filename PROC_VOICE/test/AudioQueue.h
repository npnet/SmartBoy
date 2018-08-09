/*
 * AudioQueue.h
 *
 *  Created on: 2015年8月17日
 *      Author: ican
 */

#ifndef AUDIOQUEUE_H_
#define AUDIOQUEUE_H_

#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "cae_thread.h"
/**
 * 队列数据结构：|***控制块***|***数据区***|
 */
typedef struct audio_queue_t {
	pthread_mutex_t mutex;		// 互斥锁
	NATIVE_EVENT_HANDLE sync_event;		// 条件锁
	int capacity;				// 队列容量
	int front;					// 队头索引
	int rear;					// 队尾索引
	int more;					// 写完标记
} audio_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 将一块内存初始化为队列数据结构
 *
 * base 分配给队列的内存块首地址，内存长度为sizeof(audio_queue_t) + capacity
 * capacity 队列数据区大小，队列实际容量为capacity - 1
 */
audio_queue_t* queue_init(void* base, int capacity);

void queue_destroy(audio_queue_t* queue);

int queue_real_capacity(audio_queue_t* queue);

int queue_front(audio_queue_t* queue);

int queue_rear(audio_queue_t* queue);

int queue_len(audio_queue_t* queue);

int queue_len_asyn(audio_queue_t* queue);

int queue_left(audio_queue_t* queue);

int queue_left_asyn(audio_queue_t* queue);

int queue_empty(audio_queue_t* queue);

int queue_full(audio_queue_t* queue);

int queue_write(audio_queue_t* queue, char data[], int dataLen);

int queue_read(audio_queue_t* queue, char **data);

void queue_set_more(audio_queue_t* queue, int more);

int queue_get_more(audio_queue_t* queue);

#ifdef __cplusplus
}
#endif

#endif /* AUDIOQUEUE_H_ */

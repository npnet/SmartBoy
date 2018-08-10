/**
 * @file    cae_app.h
 * 
 * @version 1.0
 * @date    2015/10/15
 * 
 * @see        
 * 
 * History:
 * index    version        date            author        notes
 * 0        1.0            2015/10/15      kunzhang      Create this file
 */

#ifndef __CAE_THREAD_H__
#define __CAE_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>


#define NATIVE_MUTEX_HANDLE pthread_mutex_t *
typedef struct _NATIVE_EVENT{
	pthread_mutex_t	mutex;
	pthread_cond_t	signal;
	int set;
}NATIVE_EVENT, *NATIVE_EVENT_HANDLE;

typedef struct _NATIVE_THREAD{
	pthread_t id;
}NATIVE_THREAD, *NATIVE_THREAD_HANDLE;


/* ------------------------------------------------------------------------
** Function
** ------------------------------------------------------------------------ */
/** 
 * @fn		 
 * @brief	 
 * 
 *   
 * @return	 
 * @param	 
 * @param	 
 * @param	 
 * @see		
 */
extern NATIVE_MUTEX_HANDLE native_mutex_create(const char *name, void *param);
extern int native_mutex_destroy(NATIVE_MUTEX_HANDLE mutex);
extern int native_mutex_take(NATIVE_MUTEX_HANDLE mutex, int timeout);
extern int native_mutex_given(NATIVE_MUTEX_HANDLE mutex);

extern NATIVE_EVENT_HANDLE native_event_create(const char *name, void *param);
extern int native_event_destroy(NATIVE_EVENT_HANDLE event);
extern int native_event_wait(NATIVE_EVENT_HANDLE event, int timeout);
extern int native_event_set(NATIVE_EVENT_HANDLE event);


#ifdef __cplusplus
} /* extern "C" */	
#endif /* C++ */

#endif /* __CAE_THREAD_H__ */








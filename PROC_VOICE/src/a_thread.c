

/* ------------------------------------------------------------------------
** Includes
** ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include "a_thread.h"
#include "error.h"


/* ------------------------------------------------------------------------
** Macros
** ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------
** Defines
** ------------------------------------------------------------------------ */
 

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------
** Global Variable Definitions
** ------------------------------------------------------------------------ */
                             

/* ------------------------------------------------------------------------
** Function Definitions
** ------------------------------------------------------------------------ */

NATIVE_MUTEX_HANDLE native_mutex_create(const char *name, void *param)
{
   pthread_mutex_t *hd = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));  /* we cant use  MSPMemory here! */
   
   if (NULL == hd)
      return NULL;

   pthread_mutex_init(hd, NULL);
   return hd;
}

int native_mutex_destroy(NATIVE_MUTEX_HANDLE hd)
{
   if (NULL == hd)
      return -1;

   pthread_mutex_destroy(hd);
   free((void *)hd); /* we cant use  MSPMemory here! */
   return 0;
}

int native_mutex_take(NATIVE_MUTEX_HANDLE hd, int timeout)
{
   if (NULL == hd)
      return -1;

   return pthread_mutex_lock(hd);
}

int native_mutex_given(NATIVE_MUTEX_HANDLE hd)
{
   if (NULL == hd)
      return -1;

   return pthread_mutex_unlock(hd);
}

NATIVE_EVENT_HANDLE native_event_create(const char *name, void *param)
{
   NATIVE_EVENT_HANDLE event = (NATIVE_EVENT_HANDLE)malloc(sizeof(NATIVE_EVENT));
	
   if (NULL == event)
      return NULL;

   pthread_mutex_init(&event->mutex, NULL);
   pthread_cond_init(&event->signal, NULL);
   event->set = 0;

   return event;
}

int native_event_destroy(NATIVE_EVENT_HANDLE event)
{
   if (NULL != event) {
      pthread_mutex_destroy(&event->mutex);
      pthread_cond_destroy(&event->signal);
      free(event);
   }
}

int native_event_wait(NATIVE_EVENT_HANDLE event, int timeout)
{
   struct timespec to;
   struct timeval now;
   int nsec;
   int ret = 0;
	
   /*if(0x7fffffff == timeout)
      timeout = 1000 * 60 * 60 * 24;*/
	  

   gettimeofday(&now, NULL);
   nsec = now.tv_usec * 1000 + (timeout % 1000) * 1000000;
   to.tv_nsec = nsec % 1000000000;
   to.tv_sec = now.tv_sec + nsec / 1000000000 + timeout / 1000;

   if ( NULL == event )
      return -1;

   pthread_mutex_lock(&event->mutex);
   if (!event->set)
      ret = pthread_cond_timedwait(&event->signal,&event->mutex, &to);
   event->set = 0;
   pthread_mutex_unlock(&event->mutex);
   if (ETIMEDOUT == ret)
      ret = 1;

   return ret;
}

int native_event_set(NATIVE_EVENT_HANDLE event)
{
   if ( NULL == event )
      return -1;
   pthread_mutex_lock(&event->mutex);
   event->set = 1;
   pthread_cond_signal(&event->signal);
   pthread_mutex_unlock(&event->mutex);

   return 0;
}



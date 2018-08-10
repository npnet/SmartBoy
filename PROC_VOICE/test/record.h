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

#ifndef __RAECORD_H__
#define __RAECORD_H__

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

/* ------------------------------------------------------------------------
** Types
** ------------------------------------------------------------------------ */
typedef void* REACORD_HANDLE;
typedef void (*record_audio_fn)(const void *audio, unsigned int audio_len, int err_code);


/* Configuration for a stream */
struct pcm_config {
	char *device_name;
    unsigned int channels;
    unsigned int rate;
    unsigned int period_size;
    unsigned int period_count;
};


#if 0
#define FUNC_START
#define FUNC_END
#define LOG(format,...);

#else
#define FUNC_START printf("========[%s]:START========\n",__FUNCTION__);
#define FUNC_END   printf("========[%s]:END========\n",__FUNCTION__);
#define LOG(format,...) printf("%s,%d:"format,__FUNCTION__,__LINE__,##__VA_ARGS__);
#endif


#ifdef __cplusplus
} /* extern "C" */	
#endif /* C++ */

#endif /* __RAECORD_H__ */




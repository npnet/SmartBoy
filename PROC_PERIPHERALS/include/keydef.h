//
// Created by sine on 18-7-19.
//

#ifndef SMARTBOY_KEYDEF_H
#define SMARTBOY_KEYDEF_H

typedef unsigned char uchar;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long  uint64;

typedef signed int     int32;
typedef signed short    int16;

typedef signed long long  int64;

typedef uint8  boolean;
#define True  (1)
#define False (0)


#define PRINTF_DEBUG
#ifdef PRINTF_DEBUG
#define mprintf(...)  printf(__VA_ARGS__)
#define FUNC_START printf("======[%s:START]======\n",__FUNCTION__);
#define FUNC_END   printf("======[%s:END]======\n",__FUNCTION__);
#else
#define  mprintf(...)
#define FUNC_START
#define FUNC_END
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#endif //SMARTBOY_KEYDEF_H

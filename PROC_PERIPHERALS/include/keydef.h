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


#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#endif //SMARTBOY_KEYDEF_H

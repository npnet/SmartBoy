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


#if 0
#define Myprintf(...)
#define FUNC_START
#define FUNC_END
#define LOG(format,...)
#define FUNC_START
#define FUNC_END
#define FUNCTION


#else
#define Myprintf(format,...)    printf("%s,%d==>"format,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define qWiFiDebug(format, ...) qDebug("[WiFi] "format" File:%s, Line:%d, Function:%s", ##__VA_ARGS__, __FILE__, __LINE__ , __FUNCTION__);
//#define LOG(format,...) printf("%s,%d:"format,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define LOG(format,...) { char	sNow[20]; ReturnNowTime(sNow);printf("[%s][%s][%d]:"format,sNow,__FUNCTION__,__LINE__,##__VA_ARGS__);}
#define FUNC_START      printf("============[%s]:Start=============\n",__FUNCTION__);
#define FUNC_END        printf("============[%s]:End  =============\n",__FUNCTION__);
#define FUNCTION        printf("============[%s]=============\n",__FUNCTION__);

#endif




#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#endif //SMARTBOY_KEYDEF_H

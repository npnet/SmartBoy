//
// Created by sine on 18-8-10.
//

#ifndef SMARTBOY_TOOLS_H
#define SMARTBOY_TOOLS_H


typedef unsigned char uchar;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed int int32;
typedef signed short int16;

typedef signed long long int64;

#if 1
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
#define LOG(format,...) printf("%s,%d:"format,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define FUNC_START      printf("============[%s]:Start=============\n",__FUNCTION__);
#define FUNC_END        printf("============[%s]:End  =============\n",__FUNCTION__);
#define FUNCTION         printf("============[%s]=============\n",__FUNCTION__);

#endif



#endif //SMARTBOY_TOOLS_H

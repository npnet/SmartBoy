//
// Created by sine on 18-8-10.
//

#ifndef SMARTBOY_TOOLS_H
#define SMARTBOY_TOOLS_H


#if 0
#define Myprintf(...)
#define FUNC_START
#define FUNC_END
#define LOG(format,...);

#else
#define Myprintf(format,...)    printf("%s,%d==>"format,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define qWiFiDebug(format, ...) qDebug("[WiFi] "format" File:%s, Line:%d, Function:%s", ##__VA_ARGS__, __FILE__, __LINE__ , __FUNCTION__);
#define LOG(format,...) printf("%s,%d:"format,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define FUNC_START      printf("============[%s]:Start=============\n",__FUNCTION__);
#define FUNC_END        printf("============[%s]:End  =============\n",__FUNCTION__);

#endif



#endif //SMARTBOY_TOOLS_H

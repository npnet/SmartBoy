/********************************************************************
 * NAME         : AI_PKTHEAD.h
 * FUNCTION     : 数据包头结构定义 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00   97/07/03  Created By WTQ  
 ********************************************************************/
#ifndef _AI_PKTHEAD_H
#define _AI_PKTHEAD_H

#include <stdlib.h>

/* select的间隔时间 */
#define MAX_TIME_VAL 1   /* 1 second */

/* 函数返回值定义 */
#define AI_OK               0   /* 成功 */
#define AI_NG              -1   /* 失败 */
#define AI_SENDERROR       -2   /* 失败且回送出错信息(内部用) */
#define AI_CLOSESOCKET      2   /* 成功且删除SOCKET(内部用) */

/********************************************************************
 *                     数据包头结构的定义 
 ********************************************************************/
typedef struct _PacketHead {
    unsigned short iPacketID;  /* 数据包ID	     */
    int      lPacketSize;      /* 数据包大小     */
    short    iRecordNum;       /* 包体记录的个数 */
    short    iRecordSize;      /* 包体记录的长度 */
    char     cRetCode;         /* 包处理的返回值 */
    int      iReserved;	       /* 保留未用       */
} PacketHead;

#endif

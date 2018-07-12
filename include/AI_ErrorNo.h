/********************************************************************
 * NAME         : AI_ErrorNo.h
 * FUNCTION     : 通信函数错误宏定义
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * PROJECT      : aisoft
 * OS           : WINDOWS.X
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/07/03    Created By Wang Taiquan
 ********************************************************************/
#ifndef _AI_ERRORNO_H
#define _AI_ERRORNO_H

#define ERROR_MESSAGE_LEN         80

/*************************************************************
*		公共错误信息表 			                             *
*************************************************************/
/* 编号范围为：-101 ~ -120 */
#define ERROR_NOERROR              0     /* 无错             */
#define ERROR_SOCKET_CREATE     -101     /* 通信端口创建失败 */
#define ERROR_SOCKET_CONNECT    -102     /* 通信端口连接失败 */
#define ERROR_SOCKET_READ       -103     /* 通信端口读出错   */
#define ERROR_SOCKET_WRITE      -104     /* 通信端口写出错   */
#define ERROR_SOCKET_DISCONNECT -105     /* 通信端口异常中断 */
#define ERROR_MEMORYALLOC       -106     /* 存储器分配失败   */

/*************************************************************
*		错误信息定义	    	    	                     *
*************************************************************/
/* 编号范围为：-121 ~ -150 */
#define ERROR_INITIALIZE      -121    /* 初始化错误 */
#define ERROR_ENV_VARIABLE    -122    /* 环境变量未定义 */
#define ERROR_PROCESS_CREAT   -123    /* 进程创建错误   */
#define ERROR_FILEACCESS      -124    /* 文件访问失败 */
#define ERROR_FILEREAD        -125    /* 文件读失败   */
#define ERROR_FILEWRITE       -126    /* 文件写失败   */

#define ERROR_USER_DEFINED    -136    /* 用户自定义的错误 */

#endif

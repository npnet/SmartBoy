/********************************************************************
 * NAME         : AIEUComm.h
 * FUNCTION     : Header file of AIEUComm.c
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/05/20
 * PROJECT      : aisoft
 * OS           : DEC ALPHA UNIX
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/05/20
 ********************************************************************/
#ifndef	___AIEUCOMM_H___
#define	___AIEUCOMM_H___

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define  LISTENQUEUE             100

// 网络间通信用 --- 客户端
extern int  AIEU_TCPEstablishConnection(char *hostName, char *serviceName);
// 网络间通信用 --- 服务器端
extern int  AIEU_TCPListenForConnection(char *serviceName);
extern int  AIEU_TCPDoAccept(int sd,char *szIPaddr);

// 同一机器内通信用 --- 客户端
extern int  AIEU_DomainEstablishConnection(char *szServerSocketName);
// 同一机器内通信用 --- 服务器端
extern int  AIEU_DomainListenForConnection(char *szServerSocketName);
extern int  AIEU_DomainDoAccept(int sd,char *szClientPid);

// 客户端、服务器端通用
extern int  AIEU_TCPSend(int sd,char *buffer,int size) ;
extern int  AIEU_TCPRecv(int sd,char *buffer,int size,int timeout) ;
extern int  AIEU_TCPClose(int sd) ;
extern int  AIEU_CheckRecvBuffer(int sd) ;

#endif	/* ___AIEUCOMM_H___ */

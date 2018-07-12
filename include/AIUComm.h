/********************************************************************
 * NAME         : AIUComm.h
 * FUNCTION     : Header file of AIUComm.c
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/05/20
 * PROJECT      : aisoft
 * OS           : DEC ALPHA UNIX
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/05/20 
 ********************************************************************/
#ifndef        _AIUCOMM_H_
#define        _AIUCOMM_H_

#include "AIEUComm.h"

int  AAWANTSendPacket(int sd, char *pPacket);
int  AAWANTSendPacket(int sd, short iPacketID,char *pPacketBody,int nBodyLen);
char *AAWANTRecvPacket(int sd, int nTimeOut, int *errflag);
char *AAWANTGetPacket(int sd, int *errflag);
int  AIcom_SetErrorMsg(int code,char *error1,char *error2);
int  AAWANTSendPacketHead(int sd, short iPacketID);
int  AAWANTGetHostAddr(char *szHostName);
	
#endif         /* _AIUCOMM_H_ */



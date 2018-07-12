/********************************************************************
 * NAME         : AIUComm.c
 * FUNCTION     : send and recieve packet
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/05/15
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/05/15   Created By Wang Taiquan
 ********************************************************************/

/********************************************************************
 *    INCLUDE FILES
 ********************************************************************/
#include <stdio.h>
#include "AIUComm.h"
#include "AI_PKTHEAD.h"
#include "AI_ErrorNo.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define  PKT_PACKET_SIZE_LEN    8

/********************************************************************
 *    内部函数定义
 ********************************************************************/
static int Send_Packet(int sd, char *pPacket,int lLong);
static char *Recv_Packet(int sd, int nTimeOut, int *errflag );
static int AIcom_InitErrorMsg();
static int AIcom_CombineErrorMsg(char *sError);

/*********************************************************************
 *      Error data structure
 ********************************************************************/
typedef struct _AIcom_ErrorMsg {
    int     iErrorCode;
    char    sErrorStr1[40];
    char    sErrorStr2[20];
} AIcom_ErrorMsg;

static   AIcom_ErrorMsg    stErrorMsg;

struct _ErrorMsgList {
    int     ErrorCode;
    char    *sFormat;
} ErrorMsgList[] = {
        ERROR_INITIALIZE,           (char *)"%s:系统初始化出错。",

        ERROR_ENV_VARIABLE,         (char *)"%s:环境变量%s未定义。",
        ERROR_PROCESS_CREAT,        (char *)"%s:环境变量%s未定义。",

        ERROR_SOCKET_CREATE,        (char *)"%s:数据通道%s读出错。",
        ERROR_SOCKET_CONNECT,       (char *)"%s:数据通道%s连接出错。",
        ERROR_SOCKET_READ,          (char *)"%s:数据通道%s读出错。",
        ERROR_SOCKET_WRITE,         (char *)"%s:数据通道%s写出错。",
        ERROR_SOCKET_DISCONNECT,    (char *)"%s:数据通道%s读出错。",

        ERROR_MEMORYALLOC,          (char *)"%s:存储分配出错。",

        ERROR_FILEACCESS,           (char *)"%s:文件%s不能访问。",
        ERROR_FILEREAD,             (char *)"%s:文件%s不能读。",
        ERROR_FILEWRITE,            (char *)"%s:文件%s不能写。",
};

/********************************************************************
 * NAME         : Send_Packet() 
 * FUNCTION     : send packet 
 * PROCESS      : 1.send packet 
 * INPUT        : sd       :socket descriptor
 *              : pPacket  :pointer to packet 
 * OUTPUT       : 
 * UPDATE       :
 * RETURN       : -1      : error
 *              : >0      : size of data send out
 * PROGRAMMED   : aisoft/aisoft WTQ
 * DATE(ORG)    : 97/05/15
 * CALL         : AIEU_TCPSend()
 * SYSTEM       : 
 ********************************************************************/
static int Send_Packet(int sd, char *pPacket ,int lLong)
{
    char       *pSendPacket,*pFillPacket;
    char       szLengthBuf[PKT_PACKET_SIZE_LEN+1];
    PacketHead *pHead;
    int        lSendSize,lRealSize,has_error;
    
    /* -----<alloc memory for real send packet>----- */
    pHead = (PacketHead *)pPacket;

    has_error = 0;
    if(pHead->cRetCode==AI_NG && stErrorMsg.iErrorCode!=ERROR_NOERROR) {
        lLong += ERROR_MESSAGE_LEN;
        pHead->cRetCode = AI_SENDERROR;
        pHead->lPacketSize += ERROR_MESSAGE_LEN;
	    has_error = 1;
    };

    lSendSize = lLong;
    pSendPacket = (char *)malloc(lSendSize);
    if(pSendPacket==NULL) {
        printf("PID %d::Send Packet Failed To Alloc %ld Bytes Memory!\n",getpid(), lSendSize);
        if(pHead->cRetCode == AI_SENDERROR && has_error==1) {
            pHead->cRetCode = AI_NG;
            pHead->lPacketSize -= ERROR_MESSAGE_LEN;
        };

        return -1L;
    }

    memset(pSendPacket,0,lSendSize);

    /* -----<fill packet head>----- */
    pFillPacket = pSendPacket;

    if(pHead->cRetCode==AI_SENDERROR && has_error==1) {
        memcpy(pFillPacket,pPacket,lLong-ERROR_MESSAGE_LEN);
        pFillPacket += lLong-ERROR_MESSAGE_LEN;
        memset(pFillPacket,0,ERROR_MESSAGE_LEN);
        AIcom_CombineErrorMsg(pFillPacket);
        pHead->cRetCode = AI_NG;
        pHead->lPacketSize -= ERROR_MESSAGE_LEN;
    } else {
        memcpy(pFillPacket,pPacket,lLong);
    }

	pFillPacket = pSendPacket;
	lRealSize = lSendSize;

    /* -----<send packet>----- */
    sprintf(szLengthBuf,"%08d",lRealSize);

    lLong = AIEU_TCPSend( sd, szLengthBuf, PKT_PACKET_SIZE_LEN); 
    if(lLong != PKT_PACKET_SIZE_LEN) {
        printf("PID %d::Send size is error! real %d must %d\n",getpid(),lLong,PKT_PACKET_SIZE_LEN);
        free(pFillPacket);
        return -1L;
    };

    lLong = AIEU_TCPSend( sd, pFillPacket, lRealSize ); 
    free(pFillPacket);

    AIcom_InitErrorMsg();

    return (lLong);
}

/********************************************************************
 * NAME         : Recv_Packet() 
 * FUNCTION     : recieve packet and return all information
 * PROCESS      : 1.check recieve buffer of socket to find error
 *              : 2.malloc memory for packet
 *              : 3.receive packet
 * INPUT        : sd      : socket descriptor
 *              : nTimeOut: time out value ( second ) 
 * OUTPUT       : erflag  : error flag set when socket is disconnect
 * UPDATE       :
 * RETURN       : NULL    : error
 *              : other   : pointer to packet head
 * PROGRAMMED   : aisoft/aisoft Wang Taiquan
 * DATE(ORG)    : 97/07/15 
 * CALL         : AIEU_CheckRecvBuffer()
 *              : AIEU_TCPRecv()
 * SYSTEM       : 
 ********************************************************************/
static char *Recv_Packet(int sd,int nTimeOut,int *errflag )
{
    char	*pPacket,*pPacket2;
    int		lLong,lRealSize;
    char	szPacketSize[PKT_PACKET_SIZE_LEN+1];    

    AIcom_InitErrorMsg();

    if(errflag!=NULL) {
        *errflag = 0;
    }

    lLong = AIEU_CheckRecvBuffer( sd );

    /* -----<when buffer is empty , socket is disconnect>----- */
    if( lLong <= 0 ) { /* socket's buffer is empty ! */
        if(errflag!=NULL) {
            *errflag = 1;  /* socket is disconnect */
        }
        return( NULL );    
    }

    /* -----<read first PKT_PACKET_SIZE_LEN bytes>----- */
    memset(szPacketSize,0,PKT_PACKET_SIZE_LEN+1);
    lLong=AIEU_TCPRecv(sd,szPacketSize,PKT_PACKET_SIZE_LEN,nTimeOut );

    if( lLong != PKT_PACKET_SIZE_LEN){
		printf("PID %d::Want received Head:%d, Real received:%d!\n", getpid(), PKT_PACKET_SIZE_LEN, lLong);
        pPacket = NULL;
        *errflag = 1;  /* socket is disconnect */
        return( pPacket );
    }

    lLong = atoi(szPacketSize);

    pPacket = (char *)malloc( lLong );
    if( pPacket == NULL ) {
        printf("PID %d::Recv Packet Failed To Alloc %ld Bytes Memory!\n",getpid(), lLong);
        *errflag = 1;
        return( pPacket );
    }

    lRealSize=AIEU_TCPRecv(sd,pPacket,lLong, nTimeOut);
    
    if( lRealSize<0 || lLong != lRealSize ) {
		printf("PID %d::Want received:%d, Real received:%d!\n", getpid(), lLong, lRealSize);
        free( pPacket );
        *errflag = 1;  /* socket is disconnect */
        return( NULL );
    }

	pPacket2 = pPacket;
	lRealSize = lLong;

    pPacket = (char *)malloc(lRealSize);
    if(pPacket==NULL) {
        free(pPacket2);
        *errflag = 1;
        return NULL;
    }
    memcpy(pPacket,pPacket2,lRealSize);
    free(pPacket2);

    return(pPacket);
}

/********************************************************************
 * NAME         : AAWANTSendPacket() 
 * FUNCTION     : send packet 
 * PROCESS      : 1.send packet 
 * INPUT        : sd       :socket descriptor
 *              : pPacket  :pointer to packet 
 * OUTPUT       : 
 * UPDATE       :
 * RETURN       : -1      : error
 *              : >0      : size of data send out
 * PROGRAMMED   : aisoft/aisoft WTQ
 * DATE(ORG)    : 97/05/15
 * CALL         : Send_Packet()
 * SYSTEM       : 
 ********************************************************************/
int AAWANTSendPacket(int sd, char *pPacket )
{
    PacketHead *pPackHead;
    int        lLong,lSendSize;
    
    pPackHead = (PacketHead *)pPacket;

    lLong = pPackHead -> lPacketSize;

    if(lLong<=0) {
        printf("PID %d::Packet Size Is Error!\n",getpid());
        return -1L;
    }
        
    lSendSize = Send_Packet( sd, pPacket, lLong ); 
    if(lSendSize < 0) {
        return -1L;
    };

    return (lLong);
}

/********************************************************************
 * NAME         : AAWANTSendPacket() 
 * FUNCTION     : send packet 
 * PROCESS      : 1.send packet 
 * INPUT        : sd         :socket descriptor
 *              : iPacketID  :packet ID
 *              : pPacketBody:pointer to packet Body
 *              : nBodyLen   :Body Length
 * OUTPUT       : 
 * UPDATE       :
 * RETURN       : -1      : error
 *              : >0      : size of data send out
 * PROGRAMMED   : aisoft/aisoft WTQ
 * DATE(ORG)    : 97/05/15
 * CALL         : Send_Packet()
 * SYSTEM       : 
 ********************************************************************/
int AAWANTSendPacket(int sd, short iPacketID,char *pPacketBody,int nBodyLen)
{
	char *pPacket = (char *)malloc(sizeof(PacketHead)+nBodyLen);
	memset(pPacket,0,sizeof(PacketHead)+nBodyLen);
    PacketHead *pPackHead = (PacketHead *)pPacket;
	pPackHead->iPacketID = iPacketID;
	pPackHead->lPacketSize = sizeof(PacketHead)+nBodyLen;
	memcpy(pPacket+sizeof(PacketHead),pPacketBody,nBodyLen);
	
	int nRet = AAWANTSendPacket(sd, pPacket);
	free(pPacket);
	
    return nRet;
}

/********************************************************************
 * NAME         : AAWANTRecvPacket() 
 * FUNCTION     : recieve packet and return all information
 * PROCESS      : 1.check recieve buffer of socket to find error
 *              : 2.malloc memory for packet
 *              : 3.receive packet
 * INPUT        : sd      : socket descriptor
 *              : nTimeOut: time out value ( second ) 
 * OUTPUT       : erflag  : error flag set when socket is disconnect
 * UPDATE       :
 * RETURN       : NULL    : error
 *              : other   : pointer to packet head
 * PROGRAMMED   : aisoft/aisoft Wang Taiquan
 * DATE(ORG)    : 97/07/15 
 * CALL         : Recv_Packet()
 * SYSTEM       : 
 ********************************************************************/
char *AAWANTRecvPacket(int sd,int nTimeOut,int *errflag )
{
    char       *pPacket;

    if(errflag!=NULL) {
        *errflag = 0;
    };

    pPacket = Recv_Packet(sd,nTimeOut,errflag);
    if((errflag!=NULL && *errflag) || !pPacket) {
        return pPacket;
    }

    return(pPacket);
}

/********************************************************************
 * NAME         : AAWANTGetPacket() 
 * FUNCTION     : recieve packet from socket sd using waiting mode
 * PROCESS      : 1. select for receive
 *              : 2. recieve packet
 * INPUT        : sd      : socket descriptor
 * OUTPUT       : erflag  : error flag set when socket is disconnect
 * UPDATE       :
 * RETURN       : NULL    : error
 *              : other   : pointer to packet head
 * PROGRAMMED   : aisoft/aisoft Wang Taiquan
 * DATE(ORG)    : 97/07/15 
 * CALL         : select()
 *              : AAWANTRecvPacket()
 ********************************************************************/
char *AAWANTGetPacket(int sd, int *errflag)
 {
    char       *pPacket;
    fd_set     rd_set;
    struct timeval time_val;
    int        rc;

    time_val.tv_sec  = 10;
    time_val.tv_usec = 0;

    while(1) {
        FD_ZERO(&rd_set);
        FD_SET(sd, &rd_set);

        rc  = select(sd+1,&rd_set,NULL,NULL,&time_val);
        if (rc==0) {
            continue;
        }

        if(rc<0) {
	    	if(errno == EINTR) {
         		continue;
	    	}
            if(errflag!=NULL) {
                *errflag = errno;
            }
            return NULL;
        }

        if(FD_ISSET(sd,&rd_set)) {
            return AAWANTRecvPacket(sd,10,errflag);
        }
    } 
    *errflag = 0;

    return(NULL);
}

/********************************************************************
 * NAME         : AIcom_InitErrorMsg
 * FUNCTION     : initialize error message record data structure
 * PROCESS      : initialize error message record data structure
 * INPUT        :
 * OUTPUT       :
 * UPDATE       :
 * RETURN       :
 * PROGRAMMED   : aisoft
 * DATE(ORG)    : 95/10/30
 * CALL         :
 ********************************************************************/
static int AIcom_InitErrorMsg()
{
    memset((char *)&stErrorMsg,0,sizeof(AIcom_ErrorMsg));
    stErrorMsg.iErrorCode = ERROR_NOERROR;

    return 0;
}

/********************************************************************
 * NAME         : AIcom_SetErrorMsg
 * FUNCTION     : set error message
 * PROCESS      : set error message
 * INPUT        : code  : error code
 *              : error1: error type parameter 1
 *              : error2: error type parameter 2
 * OUTPUT       :
 * UPDATE       :
 * RETURN       :
 * PROGRAMMED   : aisoft
 * DATE(ORG)    : 95/10/30
 * CALL         :
 ********************************************************************/
int AIcom_SetErrorMsg(int code, char *error1,char *error2)
{
    if(stErrorMsg.iErrorCode==ERROR_NOERROR) {
        stErrorMsg.iErrorCode = code;
        if(error1) {
            if(strlen(error1)<40) {
                strcpy(stErrorMsg.sErrorStr1,error1);
            } else {
                strncpy(stErrorMsg.sErrorStr1,error1,40);
                stErrorMsg.sErrorStr1[39] = '\0';
            };
        }
        if(error2) {
            if(strlen(error2)<20) {
                strcpy(stErrorMsg.sErrorStr2,error2);
            } else {
                strncpy(stErrorMsg.sErrorStr2,error2,20);
                stErrorMsg.sErrorStr2[19] = '\0';
            };
        }
    }
    return 0;
}

/********************************************************************
 * NAME         : AIcom_CombineErrorMsg
 * FUNCTION     : construct outputed error message
 * PROCESS      : construct outputed error message
 * INPUT        : sStr  : string used in store message
 * OUTPUT       :
 * UPDATE       :
 * RETURN       :
 * PROGRAMMED   : aisoft
 * DATE(ORG)    : 95/10/30
 * CALL         :
 ********************************************************************/
static int AIcom_CombineErrorMsg(char *sStr)
{
    int i;
    char  sBuf[512];

    if(stErrorMsg.iErrorCode==ERROR_NOERROR) {
        *sStr = 0;
        return 0;
    }

    for(i=0;i<sizeof(ErrorMsgList)/sizeof(struct _ErrorMsgList);i++) {
        if(stErrorMsg.iErrorCode==ErrorMsgList[i].ErrorCode) {
            sprintf(sBuf,ErrorMsgList[i].sFormat,
                         "AAWANT",
                         stErrorMsg.sErrorStr1,
                         stErrorMsg.sErrorStr2);
            if(strlen(sBuf)<ERROR_MESSAGE_LEN) {
                strcpy(sStr,sBuf);
            } else {
                strncpy(sStr,sBuf,ERROR_MESSAGE_LEN);
                sStr[ERROR_MESSAGE_LEN-1]='\0';
            };
            return 0;
        }
    }

    *sStr = 0;
    return 0;
}

/********************************************************************
 * NAME     : AAWANTSendPacketHead
 * FUNCTION : 向指定SOCKET传送一个基本信息的包头
 * PROCESS  : 
 * INPUT    : sd        : SOCKETID
 *          : iPacketID : 包ID
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : AI_OK  : ok
 *          : AI_NG  : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
int AAWANTSendPacketHead(int sd,short iPacketID)
{
    PacketHead stPacketHead;
    
    memset(&stPacketHead,0,sizeof(PacketHead));

    stPacketHead.iPacketID = iPacketID;
    stPacketHead.lPacketSize = sizeof(PacketHead);
    stPacketHead.iRecordNum  = 1;
    
    if(AAWANTSendPacket(sd,(char *)&stPacketHead)<0) {
        AIcom_SetErrorMsg(ERROR_SOCKET_WRITE,NULL,NULL);
        return AI_NG;
    };

    return AI_OK;
}

/********************************************************************
 * NAME     : AAWANTGetHostAddr
 * FUNCTION : 返回本主机的网络地址,形式为:XXX.XXX.XXX.XXX
 * PROCESS  : 
 * UPDATE   : sAddr  : 存放地址的空间
 * RETURN   : AI_OK  : ok
 *          : AI_NG  : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
int AAWANTGetHostAddr(char *szHostName)
{
    struct hostent *pHostent;
    struct in_addr inaddr;
    char   *HostIP;

    pHostent = gethostbyname(szHostName);
    if(pHostent==NULL) {
        return AI_NG;
    }
    
    memcpy(&(inaddr.s_addr),pHostent->h_addr,pHostent->h_length);

    HostIP = (char *)inet_ntoa(inaddr);
    if(HostIP!=NULL) {
	   strcpy(szHostName,HostIP);
	   return  AI_OK;
    };
	return AI_NG;
}


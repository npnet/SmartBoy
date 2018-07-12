/********************************************************************
 * NAME         : AIEUComm.c
 * FUNCTION     : aisoft communication (unix) library
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/05/15
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/05/16 
 ********************************************************************/

/********************************************************************
 *    INCLUDE FILES
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include "AIUComm.h"

/********************************************************************
 *    external variables
 ********************************************************************/
extern int errno;

/********************************************************************
 * NAME     : AIEU_TCPGetPortByName
 * FUNCTION : get service port value by name 
 * PROCESS  : 1. get service by name
 * INPUT    : szServiceName : service name
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : -1    : fail
 *          : port value
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : getservbyname()
 ********************************************************************/
int AIEU_TCPGetPortByName(char *szServiceName)
{
    struct servent * pstServEnt;

    /* -----<get service by name>----- */
    if( (pstServEnt = getservbyname( szServiceName, "tcp" ))==NULL) {
		char  szPortNum[20];
		int   nPort=atoi(szServiceName);
		sprintf(szPortNum,"%d",nPort);
		if(strcmp(szServiceName,szPortNum)==0) {
			return nPort;
		};
        printf("PID %d::Unknown service no %d!\n",getpid(),nPort);
		
        printf("PID %d::Unknown service name %s!\n",getpid(),szServiceName);
        return( -1 );
    }

    return  pstServEnt -> s_port;
}

/********************************************************************
 * NAME     : AIEU_TCPEstablishConnection
 * FUNCTION : client connect to server
 * PROCESS  : 1. get host by name
 *          : 2. get service by name
 *          : 3. creat socket
 *          : 4. set blocking mode
 *          : 5. bind to local address
 *          : 6. connect to remote hosts
 * INPUT    : szHostName    : host name
 *          : szServiceName : service name
 * OUTPUT   : NONE 
 * UPDATE   : NONE
 * RETURN   : -1    : connect fail
 *          : other : connected socket ID
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : gethostbyname()
 *          : getservbyname()
 *          : socket()
 *          : ioctl()
 *          : bind()
 *          : connect()
 *          : memset()
 ********************************************************************/
int AIEU_TCPEstablishConnection(char *szHostName, char *szServiceName)
{
    int    nPort;
    char   szHostIP[50];
    struct sockaddr_in sin;
    int    sd;        /* socket descript */
    
    /* -----<get host ipaddr by name>----- */
    strcpy(szHostIP,szHostName);
    AAWANTGetHostAddr(szHostIP);

    /* -----<get service by name>----- */
    nPort = AIEU_TCPGetPortByName(szServiceName);
    if(nPort == -1) {
        return( -1 );
    }

    /* -----<creat socket>----- */
    sd = socket(AF_INET, SOCK_STREAM, 0 );
    if(sd<0) {
        printf("PID %d::Socket failed!\n",getpid());
        return ( -1 );
    }

    /* -----<connect to remote host>----- */
    memset( &sin, 0, sizeof( struct sockaddr_in ) );
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(szHostIP);
    sin.sin_port = htons(nPort);

    if(connect(sd, (const struct sockaddr *)&sin, sizeof(struct sockaddr_in))<0) {
        close(sd);
        printf("PID %d::Connect failed on port=%d(errno=%d)!\n",getpid(),nPort,errno);
        return( -1 );
    }

    /* -----<we have connection>----- */
    return ( sd );
}

/********************************************************************
 * NAME     : AIEU_TCPListenForConnection
 * FUNCTION : create server and listen for connection
 * PROCESS  : 1. get service by name
 *          : 2. create socket
 *          : 3. bind to local address
 *          : 4. listen for connect
 * INPUT    : szServiceName : service name
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : -1    : fail
 *          : other : socket ID
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : getservbyname()
 *          : socket()
 *          : ioctl()
 *          : bind()
 *          : listen()
 ********************************************************************/
int AIEU_TCPListenForConnection(char *szServiceName )
{
    int    port;
    struct sockaddr_in sin;
    int    sd;        /* socket descript */
    long   lEnable;   /* 0: blocking ; 1: non-blocking */
	int    nEnable;
    
    /* -----<get service by name>----- */
    if((port=AIEU_TCPGetPortByName(szServiceName)) < 0) {
	return -1;
    };

    /* -----<creat socket>----- */
    sd = socket(AF_INET, SOCK_STREAM, 0 );
    if ( sd < 0) {
        printf("PID %d::socket create error, err:%s!\n",getpid(),strerror(errno));
        return ( -1 );
    }

    /* -----<set non-blocking mode>----- */
    lEnable = 1;
    ioctl(sd, FIONBIO, &lEnable);

    /* -----<bind to local address>----- */
    bzero((char *) &sin, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port =htons(port);

    nEnable = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &nEnable, sizeof(int)) !=0){
       printf("PID %d::Fail to setsockopt, err:%s!\n", getpid(), strerror(errno));  
    }

    if ( bind( sd, (const struct sockaddr *)&sin, sizeof(sin) ) < 0 ) {
        printf("PID %d::socket bind error, err:%s!\n",getpid(), strerror(errno));
        close( sd );
        return ( -1 );
    }

    /* -----<listen for connection>----- */
    if(listen( sd, LISTENQUEUE )<0) {
        printf("PID %d::socket listen error, err:%s!\n",getpid(), strerror(errno));
		close(sd);
		return -1;
	};

    return ( sd );
}

/********************************************************************
 * NAME     : AIEU_TCPDoAccept
 * FUNCTION : server accept connection from client
 * PROCESS  : 1. set blocking mode
 *          : 2. waiting for connect
 * INPUT    : sd : socket ID 
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : new socket ID 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : ioctl()
 *          : accept()
 * SYSTEM   : 
 ********************************************************************/
int AIEU_TCPDoAccept(int sd, char *szIPaddr )
{
    int    newsd;
    char   *pIP;
    struct sockaddr_in from;
    struct in_addr inaddr;
    int    len;
    long   lEnable;     /* 0: blocking ; 1: non-blocking */

    /* -----<set blocking mode>----- */
    lEnable = 0;
    ioctl(sd, FIONBIO, &lEnable);

    len = sizeof( from );
    memset( &from, 0, len );
    newsd = accept( sd, (struct sockaddr *)&from, (socklen_t *)&len );

    if( newsd < 0 ) {
        if( errno != EINTR ) {
            printf("PID %d::Accept failed, err:%s!\n", getpid(),strerror(errno));
        } else {
            printf("PID %d::User interrupted!\n", getpid());
        }

        return -1;
    }

    /* remote host information in from */
    inaddr.s_addr = from.sin_addr.s_addr;
    pIP = (char *)inet_ntoa(inaddr);
    strcpy(szIPaddr,pIP);

    return( newsd );
}

/********************************************************************
 * NAME     : AIEU_TCPSend
 * FUNCTION : send data using socket
 * PROCESS  : 1. split data into many packet
 *          : 2. send packets by loop 
 * INPUT    : sd      : socket ID
 *          : pBuffer : pointer of data buffer
 *          : lSize   : buffer size
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : -1    : send fail
 *          : other : bytes send
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : send()
 * SYSTEM   : 
 ********************************************************************/
int AIEU_TCPSend(int sd, char *pBuffer,int lSize )
{
    int				rc;
    int				lLeft;
    fd_set			wr_set;
    struct timeval  tv;

    FD_ZERO(&wr_set);
    FD_SET(sd, &wr_set);

    tv.tv_sec = 60;
    tv.tv_usec = 0;

    lLeft = lSize;

    while (lLeft > 0)     {
		FD_ZERO(&wr_set);
		FD_SET(sd, &wr_set);
		rc = (int) select(sd+1, NULL, &wr_set, NULL, &tv);
		if(rc == 0) {
			break;
		}

		if(rc < 0) {
			if(errno == EINTR) {
				continue;
			}
			printf("PID %d::Select error, err:%s\n",getpid(), strerror(errno));
			return( -1 );
		}

		rc = send ( sd, pBuffer, lLeft, 0);
		if ( rc == -1 )  {
			printf("PID %d::Send error, err:%s\n",getpid(), strerror(errno));
			return (-1);
		};
		pBuffer += rc;
		lLeft -= rc;
    }

    /* -----<data all sent, now prepare for the next send>----- */
    return (lSize-lLeft);
}

/********************************************************************
 * NAME     : AIEU_TCPRecv
 * FUNCTION : recieve data by socket
 * PROCESS  : 1. receive all data in socket
 *          : 2. pack these data into a buffer
 * INPUT    : sd       : socket ID
 *          : pBuffer  : buffer used in store received data
 *          : lSize    : received size expected 
 *          : nTimeOut : time out ( seconds )
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : -1    : fail
 *          : other : received data real size
 * PROGRAMMED    : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : select()
 *          : recv()
 *          : ioctl()
 *          : FD_SET()
 * SYSTEM   : 
 ********************************************************************/
int AIEU_TCPRecv(int sd, char *pBuffer, int lSize, int nTimeOut  )
{
    int		rc;
    int     lLeft;   
    long    lEnable;        /* 0: blocking ; 1: non-blocking */
    fd_set  rd_set;
    struct	timeval    tv;
    
    FD_ZERO(&rd_set);
    FD_SET(sd, &rd_set);

    tv.tv_sec = nTimeOut;
    tv.tv_usec = 0;

    lEnable = 1;
    ioctl(sd, FIONBIO, &lEnable);

    lLeft = lSize;

    while (lLeft > 0)  {
        FD_ZERO(&rd_set);
        FD_SET(sd, &rd_set);
        rc = (int) select(sd+1, &rd_set, NULL, NULL, &tv);

        if(rc == 0) {	/* 如果没有数据，则返回0 */
			break;
        };

	    if(rc<0) {
	        if(errno == EINTR) {
	           	continue;
	        }

        	printf("PID %d::Select error, err:%s!\n",getpid(), strerror(errno));
            return( -1 );         
        }

        rc = recv ( sd, pBuffer, lLeft, 0);
        if ( rc <= 0 )  {
            printf("PID %d::Recv error, err:%s!\n",getpid(),strerror(errno));
            return (-1);
        } 
        pBuffer += rc;
        lLeft -= rc;
    }

    /* -----<data all recv, now prepare for the next recv>----- */
    return ( lSize-lLeft );
}

/********************************************************************
 * NAME     : AIEU_TCPClose
 * FUNCTION : close socket
 * PROCESS  : 1. disable both send & receive
 *          : 2. close socket
 * INPUT    : sd : socket ID
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : 
 * PROGRAMMED    : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : close()
 *          : shutdown()
 * SYSTEM   : 
 ********************************************************************/
int AIEU_TCPClose(int  sd )
{
    shutdown( sd, 2 );    /* disable both send & recv    */
    close( sd );
    return 0;
}

/********************************************************************
 * NAME     : AIEU_CheckRecvBuffer
 * FUNCTION : check recv buffer  if have data arrived 
 * PROCESS  : 1. set nonblock mode for socket 
 *          : 2. check receive buffer 
 * INPUT    : sd       : socket ID
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : -1    : buffer is empty
 *          : other : buffer has data
 * PROGRAMMED    : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : ioctl()
 *          : recv()
 * SYSTEM   : 
 ********************************************************************/
int AIEU_CheckRecvBuffer(int sd)
{
    long    lEnable;        /* 0: blocking ; 1: non-blocking */
    char    buf[2];
    int     rc;
    
    lEnable = 1;
    ioctl(sd, FIONBIO, &lEnable);

    rc = recv( sd, buf, 1, MSG_PEEK );
    
    return rc;
}

/********************************************************************
 * NAME			: AIEU_DomainEstablishConnection
 * FUNCTION		: client connect to server
 * INPUT		: szSocketName    : socket file name
 *				: nTimeOut        : time out 
 * OUTPUT		: NONE
 * RETURN		: -1    : connect fail
 *				: other : connected socket ID
 * PROGRAMMED   : WTQ/aawant
 * DATE(ORG)	: 2018/07/12
 ********************************************************************/
#define CLIENT_SOCKET_PATH    "/var/tmp/"      /* +5 for pid = 14 chars */

int AIEU_DomainEstablishConnection(char *szServerSocketName)
{
    struct sockaddr_un sun;
    int    sd;        /* socket descript */
    
    /* -----<creat socket>----- */
    sd = socket(AF_UNIX, SOCK_STREAM, 0 );
    if(sd<0) {
        printf("PID %d::Domain Socket failed!\n",getpid());
        return ( -1 );
    }

    /* -----<connect to socket>----- */
    memset( &sun, 0, sizeof( struct sockaddr_un ) );
    sun.sun_family = AF_UNIX;
	sprintf(sun.sun_path, "%s%05d",CLIENT_SOCKET_PATH,getpid());
	unlink(sun.sun_path);        /* in case it already exists */

	int len = offsetof(struct sockaddr_un, sun_path) + strlen(sun.sun_path);
	if(bind(sd, (struct sockaddr *)&sun, len) < 0) {
        printf("PID %d::socket bind error, err:%s!\n",getpid(), strerror(errno));
        close(sd);
        return ( -1 );
	};

	/* fill socket address structure with server's address */
	 memset(&sun, 0, sizeof(struct sockaddr_un));
	 sun.sun_family = AF_UNIX;
	 strcpy(sun.sun_path, szServerSocketName);
	 len = offsetof(struct sockaddr_un, sun_path) + strlen(szServerSocketName);

    if(connect(sd, (const struct sockaddr *)&sun, len)<0) {
        close(sd);
		sprintf(sun.sun_path, "%s%05d",CLIENT_SOCKET_PATH,getpid());
		unlink(sun.sun_path);
		printf("PID %d::Connect failed on socket %s, err:%s!\n",getpid(),szServerSocketName,strerror(errno));
        return( -1 );
    }

    /* -----<we have connection>----- */
    return ( sd );
}

/********************************************************************
 * NAME     : AIEU_DomainListenForConnection
 * FUNCTION : create server and listen for connection
 * PROCESS  : 1. create socket
 *          : 2. bind to local address
 *          : 3. listen for connect
 * INPUT    : szServerSocketName : socket name
 * RETURN   : -1    : fail
 *          : other : socket ID
 * PROGRAMMED   : WTQ/aawant
 * DATE(ORG)	: 2018/07/12
 ********************************************************************/
int AIEU_DomainListenForConnection(char *szServerSocketName )
{
    struct sockaddr_un  sun;
    int    sd;        /* socket descript */
    long   lEnable;   /* 0: blocking ; 1: non-blocking */
	int    nEnable;
    
    /* -----<creat socket>----- */
    sd = socket(AF_UNIX, SOCK_STREAM, 0 );
    if ( sd < 0) {
        printf("PID %d::socket create error, err:%s!\n",getpid(),strerror(errno));
        return ( -1 );
    }
	unlink(szServerSocketName);   /* in case it already exists */

    /* -----<set non-blocking mode>----- */
    lEnable = 1;
    ioctl(sd, FIONBIO, &lEnable);

    /* -----<bind to local address>----- */
	memset(&sun, 0, sizeof(struct sockaddr_un));
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, szServerSocketName);
	int len = offsetof(struct sockaddr_un, sun_path) + strlen(szServerSocketName);

    nEnable = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &nEnable, sizeof(int)) !=0){
       printf("PID %d::Fail to setsockopt, err:%s!\n", getpid(), strerror(errno));  
    }

    if ( bind( sd, (const struct sockaddr *)&sun, len ) < 0 ) {
        printf("PID %d::socket bind error, err:%s!\n",getpid(), strerror(errno));
        close( sd );
        return ( -1 );
    }

    /* -----<listen for connection>----- */
    if(listen( sd, LISTENQUEUE )<0) {
        printf("PID %d::socket listen error, err:%s!\n",getpid(), strerror(errno));
		unlink(szServerSocketName);
		close(sd);
		return -1;
	};

    return ( sd );
}

/********************************************************************
 * NAME     : AIEU_DomainDoAccept
 * FUNCTION : server accept connection from client
 * PROCESS  : 1. set blocking mode
 *          : 2. waiting for connect
 * INPUT    : sd : socket ID 
 * OUTPUT   : 
 * UPDATE   :
 * RETURN   : new socket ID 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 97/05/20
 * CALL     : ioctl()
 *          : accept()
 * SYSTEM   : 
 ********************************************************************/
int AIEU_DomainDoAccept(int sd, char *szClientPid )
{
    int    newsd;
    struct sockaddr_un from;
    int    len,pid;
    long   lEnable;     /* 0: blocking ; 1: non-blocking */

    /* -----<set blocking mode>----- */
    lEnable = 0;
    ioctl(sd, FIONBIO, &lEnable);

    len = sizeof( from );
    memset( &from, 0, len );
    newsd = accept( sd, (struct sockaddr *)&from, (socklen_t *)&len );

    if( newsd < 0 ) {
        if( errno != EINTR ) {
            printf("PID %d::Accept failed, err:%s!\n", getpid(),strerror(errno));
        } else {
            printf("PID %d::User interrupted!\n", getpid());
        }

        return -1;
    }

	/* obtain the client's uid from its calling address */
	len -= offsetof(struct sockaddr_un, sun_path); /* len of pathname */
	from.sun_path[len] = 0;           /* null terminate */
	unlink(from.sun_path);        /* we're done with pathname now */

	if(szClientPid != NULL) {
		sprintf(szClientPid,"PID[%s]",from.sun_path+strlen(CLIENT_SOCKET_PATH));   /* return pid of caller */
	};

    return( newsd );
}

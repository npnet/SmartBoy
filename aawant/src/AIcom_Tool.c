/********************************************************************
 * NAME         : AIcom_File.c 
 * FUNCTION     : file operation 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00   97/07/03 created by wtq  
 ********************************************************************/

/*********************************************************************
 *      INCLUDE FILES
 ********************************************************************/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <signal.h>
#include    "AI_PKTHEAD.h"
#include    "AI_ErrorNo.h"

#ifdef WIN32
#include    <io.h>
#include    <direct.h>
#define ACCESS  _access
#define F_OK   00
#define X_OK   01
#define R_OK   02
#define W_OK   04
#else
#include    <unistd.h>
#define ACCESS  access
#endif

char SystemRunTop[80];

/********************************************************************
 * NAME         : AIcom_GetCurrentDate
 * FUNCTION     : Get Current Date
 * PROCESS      : Get Current Date
 * INPUT        : iFlag  =  0       return  YYYYMMDD HH24:MI
 *              :           1       return  YYYYMMDD
 *              :           2       return  YYYY/MM/DD HH24-MI
 *              :           3       return  YYYY/MM/DD
 *              :           4       return  HH24-MI
 *              :           5       return  YYYY.MM.DD HH24-MI
 *              :           6       return  YYYY.MM.DD HH24MISS
 * OUTPUT       : 
 * UPDATE       : szBuf
 * RETURN       : NONE
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * CALL         : time
 *              : localtime
 ********************************************************************/
void AIcom_GetCurrentDate(char *szBuf,int iFlag)
{
    struct tm           *ltime;
    long                ts;

    if(szBuf==NULL) {
        return;
    };

    ts    = time(NULL);
    ltime = localtime(&ts);

    if( ltime == NULL ){
        *szBuf = 0;
        return;
    }

    switch (iFlag)   {
      case  0:
        sprintf(szBuf, "%04d%02d%02d %02d:%02d",
            ltime->tm_year+1900,
            ltime->tm_mon + 1, 
            ltime->tm_mday,
            ltime->tm_hour,
            ltime->tm_min);
        break;

      case  1:
        sprintf(szBuf, "%04d%02d%02d",
            ltime->tm_year+1900,
            ltime->tm_mon + 1, 
            ltime->tm_mday);
        break;

      case  2:
        sprintf(szBuf, "%04d/%02d/%02d %02d-%02d",
            ltime->tm_year+1900,
            ltime->tm_mon + 1, 
            ltime->tm_mday,
            ltime->tm_hour,
            ltime->tm_min);
        break;

      case  3:
        sprintf(szBuf, "%04d/%02d/%02d",
            ltime->tm_year+1900,
            ltime->tm_mon + 1, 
            ltime->tm_mday);
        break;

      case  4:
        sprintf(szBuf, "%02d-%02d", ltime->tm_hour,ltime->tm_min);
        break;

      case  5:
        sprintf(szBuf, "%04d.%02d.%02d %02d-%02d",
                ltime->tm_year+1900,
                ltime->tm_mon + 1,
                ltime->tm_mday,
                ltime->tm_hour,
                ltime->tm_min);
        break;

      case  6:
        sprintf(szBuf, "%04d.%02d.%02d %02d%02d%02d", 
                1900+ltime->tm_year,
                ltime->tm_mon + 1,
                ltime->tm_mday, 
                ltime->tm_hour,
                ltime->tm_min,
                ltime->tm_sec);
        break;

      default:
        *szBuf = '\0';
        return;
    }

    return;
}

/********************************************************************
 * NAME         : AIcom_FileSize
 * FUNCTION     : Get File Size
 * PROCESS      : Get File Size
 * INPUT        : szFileName : file name
 * OUTPUT       :
 * UPDATE       :
 * RETURN       : Size        : OK
 *              : AI_NG       : FAIL
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/10
 * CALL         : stat
 ********************************************************************/
long AIcom_FileSize(char *szFileName)
{
    struct stat sbuf;

    if(szFileName == NULL){
        return AI_NG;
    }

    if(!strcmp(szFileName,"")){
        return 0;
    }

    if( stat(szFileName,&sbuf) < 0 ){
        return AI_NG;
    }

    return sbuf.st_size;
}

/********************************************************************
 * NAME         : AIcom_GetFileAttr
 * FUNCTION     : check File Attribute
 * PROCESS      : check File Attribute
 * INPUT        : szFileName : file name
 *              : szMode     : expected mode 
 * OUTPUT       :
 * UPDATE       :
 * RETURN       : have expected mode     : 0
 *              : not have expected mode : 1
 *              : fail             : AI_NG
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/10
 * CALL         : ACCESS
 ********************************************************************/
int AIcom_GetFileAttr(char *szFileName,char *szMode)
{
    int i;

    if(szFileName == NULL){
        return AI_NG;
    }

    if(!strcmp(szFileName,"")){
        return AI_NG;
    }

    i = 0;
    while(szMode[i]!='\0') {
      switch(szMode[i]) {
        case 'A':
          if(ACCESS(szFileName,F_OK)!=0) {
              return 1;
          };
          break;
        case 'R':
          if(ACCESS(szFileName,R_OK)!=0) {
              return 1;
          };
          break;
        case 'W':
          if(ACCESS(szFileName,W_OK)!=0) {
              return 1;
          };
          break;
        case 'X':
          if(ACCESS(szFileName,X_OK)!=0) {
              return 1;
          };
          break;
        default:
          return 1;
      };
      i++;
    };

    return 0;
}

/********************************************************************
 * NAME         : AIcom_GetFile
 * FUNCTION     : get file content
 * PROCESS      : 1. get size
 *              : 2. open file
 *              : 3. get content
 * INPUT        : 
 * OUTPUT       :
 * UPDATE       :
 * RETURN       : AI_OK : success
 *              : AI_NG : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * CALL         : AIcom_FileSize
 *              : ACCESS
 *              : fopen
 ********************************************************************/
int AIcom_GetFile(char *szFileName,char *szBuf)
{
    int     fd;
    int     file_len;

    if (szFileName == NULL)  {
        return  AI_NG;
    }

    if (ACCESS(szFileName,R_OK) != 0)    {
        return  AI_NG;
    }

    file_len = AIcom_FileSize(szFileName);
    if (szBuf == NULL)    {
        return  AI_NG;
    }

#ifdef WIN32
    fd = open(szFileName, O_RDONLY|O_BINARY);
#else
    fd = open(szFileName, O_RDONLY);
#endif

    if (fd < 0) {
        return  AI_NG;
    }

    if (read(fd,szBuf, file_len) < file_len ) {
        close(fd);
        return  AI_NG;
    }
    close(fd);

    return  AI_OK;
}

/********************************************************************
 * NAME         : AIcom_SetFileAttr
 * FUNCTION     : Set File Attribute
 * PROCESS      : Set File Attribute
 * INPUT        : szFileName : file name
 *              : iMode      : expected mode 
 * OUTPUT       :
 * UPDATE       :
 * RETURN       : AI_OK : success
 *              : AI_NG : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/10
 * CALL         : chmod
 ********************************************************************/
int AIcom_SetFileAttr(char *szFileName,int iMode)
{
    if(szFileName == NULL){
        return AI_NG;
    }

    if(!strcmp(szFileName,"")){
        return AI_NG;
    }

    if(chmod(szFileName,iMode)<0) {
        return AI_NG;
    };

    return AI_OK;
}

/********************************************************************
 * NAME         : AIcom_CreateDir
 * FUNCTION     : 
 * PROGRAMMED   : AI/AI
 * DATE(ORG)    : 
 * PROJECT      : AI
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    
 ********************************************************************/
int AIcom_CreateDir(char *szDirName)
{
    char szDirString[256];
    char *pStr;

#ifndef WIN32
    mode_t  oldmask;

    oldmask = umask(0);
#endif

    pStr = szDirName;

    while((pStr=strchr(pStr,'/'))!=NULL) {
        memcpy(szDirString,szDirName,pStr-szDirName);
        szDirString[pStr-szDirName] = 0;
        if(pStr!=szDirName && AIcom_GetFileAttr(szDirString,(char *)"R")!=0) {
#ifndef WIN32
            if (mkdir(szDirString,0777) !=0 )  {
                umask(oldmask);
#else
            if (_mkdir(szDirString) !=0 )  {
#endif
                return  AI_NG;
            };
        }
        pStr++;    
    };

    if(AIcom_GetFileAttr(szDirName,(char *)"R")!=0) {
#ifndef WIN32
           if (mkdir(szDirName,0777) !=0 )  {
               umask(oldmask);
#else
           if (_mkdir(szDirName) !=0 )  {
#endif
               return  AI_NG;
        };
    }

#ifndef WIN32
    umask(oldmask);
#endif
    return AI_OK;
}

/********************************************************************
 * NAME         : AIcom_CreateFile
 * FUNCTION     : Create file ID
 * PROCESS      : 1. Create DIR
 *              : 2. open file
 * INPUT        : 
 * OUTPUT       :
 * UPDATE       :
 * RETURN       : fd    : success
 *              : AI_NG : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * CALL         : write
 *              : fopen
 ********************************************************************/
int AIcom_CreateFile(char *szFileName)
{
    int    fd,i;
    char   szDirName[256];
    char   *pDir,sCh;

    if(AIcom_GetFileAttr(szFileName,(char *)"ARW")==0) {
#ifdef WIN32
        if((fd = open(szFileName,O_RDWR|O_BINARY|O_TRUNC,0644))<0) {
#else
        if((fd = open(szFileName,O_RDWR|O_TRUNC,0644))<0) {
#endif
            return AI_NG;
        };
        return fd;
    };

#ifdef WIN32
    sCh = '\\';
#else 
    sCh = '/';
#endif

    pDir = NULL;
    for(i=strlen(szFileName)-1;i>=0;i--) {
        if(szFileName[i]=='/' || szFileName[i]==sCh) {
            pDir = szFileName+i;
            break;
        }
    }

    if(pDir!=NULL) {
        memcpy(szDirName,szFileName,pDir-szFileName);
        szDirName[pDir-szFileName]=0;
        if(AIcom_CreateDir(szDirName)==AI_NG) {
            return AI_NG;
        };
    };
#ifdef WIN32
    if ((fd = _creat(szFileName,_S_IREAD | _S_IWRITE))<0) {
#else
    if ((fd = creat(szFileName,0644)) < 0)   {
#endif
        return AI_NG;
    }
    close(fd);

    return AIcom_CreateFile(szFileName);
}

/********************************************************************
 * NAME         : AIcom_PutFile
 * FUNCTION     : save file content
 * PROCESS      : 1. open file
 *              : 2. save
 * INPUT        : 
 * OUTPUT       :
 * UPDATE       :
 * RETURN       : AI_OK : success
 *              : AI_NG : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/07/03
 * CALL         : write
 *              : fopen
 ********************************************************************/
int AIcom_PutFile(char *szFileName,char *szBuf,int len)
{
    int    fd;

    if (szBuf==NULL) {
        return AI_NG;
    };

    fd = AIcom_CreateFile(szFileName);
    if (fd==AI_NG)   {
        return AI_NG;
    }

    if (write(fd,szBuf,len) < len )  {
        close(fd);
        return AI_NG;
    }
    close(fd);

    return AI_OK;
}

/********************************************************************
 * NAME     : AIcom_SetSystemRunTop
 * FUNCTION : 设置系统运行顶级目录环境变量
 * PROCESS  :
 * INPUT    :
 * OUTPUT   :
 * UPDATE   :
 * RETURN   :
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG):
 * CALL     :
 ********************************************************************/
void AIcom_SetSystemRunTop(char *sRunTop)
{
	if(NULL != sRunTop) {
		strcpy(SystemRunTop,sRunTop);
	} else {
		strcpy(SystemRunTop,"/");
	}
}

/********************************************************************
 * NAME     : AIcom_GetCurrentPath
 * FUNCTION : 根据环境变量取当前运行文件所在的路径
 * PROCESS  :
 * INPUT    :
 * OUTPUT   :
 * UPDATE   :
 * RETURN   :
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG):
 * CALL     :
 ********************************************************************/
char *AIcom_GetCurrentPath()
{
    return (char *)getenv(SystemRunTop);
}

/********************************************************************
 * NAME         : AIcom_ChangeToDaemon
 * FUNCTION     : 变为守护进程
 * PARAMETER    : 
 * RETURN       : AI_OK=成功，AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 把消息SIGHUP作为输出重定向的命令消息
 ********************************************************************/ 
int AIcom_ChangeToDaemon()
{
	setsid();
	signal(SIGHUP, SIG_IGN);

	return(AI_OK);
}

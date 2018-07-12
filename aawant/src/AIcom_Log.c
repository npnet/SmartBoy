/********************************************************************
 * NAME         : AIcom_Log.c
 * FUNCTION     : 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    
 ********************************************************************/

/********************************************************************
 *  INCLUDE FILES
 ********************************************************************/
#include    <stdio.h>
#include    <errno.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>
#include    <sys/timeb.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    "AIprofile.h"
#include    "AI_PKTHEAD.h"
#include    "AIcom_Tool.h"
#include    "AILogFile.h"

/********************************************************************
 * NAME     : AIcom_SysStartLog
 * FUNCTION : 
 * PROCESS  : 
 * INPUT    : 
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
int AIcom_SysStartLog(char *szSysName,char *szUser)
{
    int  fd;
    char *pPath;
    char szFileName[256];
    char szBuf[20];
    
    pPath = AIcom_GetConfigString((char *)"Path",(char *)"Log");
    if(pPath==NULL) {
        return AI_NG;
    };

    sprintf(szFileName,"%s/system.log",pPath);
    
    if(AIcom_GetFileAttr(szFileName,(char *)"AWR")==1) {
        fd = AIcom_CreateFile(szFileName);
    } else {
        fd = open(szFileName,O_RDWR|O_APPEND);
    };

    if(fd==AI_NG) {
        return AI_NG;
    };

    AIcom_GetCurrentDate(szBuf,6);
    if(szUser==NULL) {
        sprintf(szFileName,"%s %s启动执行\n",szBuf,szSysName);
    } else {
        sprintf(szFileName,"%s %s启动%s执行\n",szBuf,szUser,szSysName);
    };
    write(fd,szFileName,strlen(szFileName));
    close(fd);
    
    return AI_OK;
}

/********************************************************************
 * NAME     : AIcom_SysEndLog
 * FUNCTION : 
 * PROCESS  : 
 * INPUT    : 
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
int AIcom_SysEndLog(char *szSysName,char *szUser)
{

    int  fd;
    char *pPath;
    char szFileName[256];
    char szBuf[20];
    
    pPath = AIcom_GetConfigString((char *)"Path",(char *)"Log");
    if(pPath==NULL) {
        return AI_NG;
    };

    sprintf(szFileName,"%s/system.log",pPath);
    
    if(AIcom_GetFileAttr(szFileName,(char *)"AWR")==1) {
        fd = AIcom_CreateFile(szFileName);
    } else {
        fd = open(szFileName,O_RDWR|O_APPEND);
    };
    if(fd==AI_NG) {
        return AI_NG;
    };

    AIcom_GetCurrentDate(szBuf,6);
    if(szUser==NULL) {
        sprintf(szFileName,"%s %s终止执行\n",szBuf,szSysName);
    } else {
        sprintf(szFileName,"%s %s终止%s执行\n",szBuf,szUser,szSysName);
    };
    write(fd,szFileName,strlen(szFileName));
    close(fd);
    
    return AI_OK;
}

/********************************************************************
 * NAME         : WriteLog
 * FUNCTION     : 写日志
 * PARAMETER    : sFileName(文件名),sLogString(日志内容)
 * RETURN       : AI_OK=成功，AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
int WriteLog(char *sFileName, char *sLogString)
{
	FILE	*LogFile;
    char	*pPath;
	char	sFullFileName[100];
	char	sNow[20];

    pPath = AIcom_GetConfigString((char *)"Path",(char *)"Log");
    if(pPath==NULL) {
        return AI_NG;
    };
	sprintf(sFullFileName,"%s/%s",pPath,sFileName);
	LogFile=fopen(sFullFileName,"a");
	if(LogFile==NULL) {
		return(AI_NG);
	};

	ReturnNowTime(sNow);
	fprintf(LogFile, "%s(%s)\n", sLogString, sNow);

	fclose(LogFile);

	return(AI_OK);
}

/********************************************************************
 * NAME         : WriteSystemInfo
 * FUNCTION     : 保存系统信息
 * PARAMETER    : sFileName(文件名),sSysInfo(系统信息内容)
 * RETURN       : AI_OK=成功，AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
int WriteSystemInfo(char *sFileName, char *sSysInfo)
{
	FILE	*SysInfoFile;
	char	sFullFileName[100];
    char	*pPath;

    pPath = AIcom_GetConfigString((char *)"Path",(char *)"Log");
    if(pPath==NULL) {
        return AI_NG;
    };
	sprintf(sFullFileName,"%s/%s",pPath,sFileName);
	SysInfoFile=fopen(sFullFileName,"a");
	if(SysInfoFile==NULL) {
		return(AI_NG);
	};

	fprintf(SysInfoFile, "%s\n", sSysInfo);

	fclose(SysInfoFile);

	return(AI_OK);
}

/********************************************************************
 * NAME         : ReturnNowTime
 * FUNCTION     : 返回当前时间
 * PARAMETER    : lpNow(字符串指针)
 * RETURN       : AI_OK=成功，AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
int ReturnNowTime(char *lpNow)
{
	time_t		nNow;
	struct tm	*lptmNow;

	nNow=time(NULL);
	lptmNow=localtime(&nNow);
	sprintf(lpNow,"%04d-%02d-%02d %02d:%02d:%02d",
		1900+lptmNow->tm_year,lptmNow->tm_mon+1,lptmNow->tm_mday,
			lptmNow->tm_hour,lptmNow->tm_min,lptmNow->tm_sec);

	return(AI_OK);
}

/********************************************************************
 * NAME         : FormatTime
 * FUNCTION     : 格式化转换时间
 * PARAMETER    : nNow(转换时间), lpNow(字符串指针)
 * RETURN       : AI_OK=成功，AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
int FormatTime(time_t nNow, char *lpNow)
{
	struct tm	*lptmNow;

	lptmNow=localtime(&nNow);
	sprintf(lpNow,"%04d-%02d-%02d %02d:%02d:%02d",
		1900+lptmNow->tm_year,lptmNow->tm_mon+1,lptmNow->tm_mday,
			lptmNow->tm_hour,lptmNow->tm_min,lptmNow->tm_sec);

	return(AI_OK);
}

/********************************************************************
 * NAME         : WriteErrPacket
 * FUNCTION     : 写入错误包
 * PARAMETER    : sFileName(写入错误包的文件),lpBuffer(错误包指针),
  * RETURN      : AI_OK=成功,AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/07/29
 * UPDATE       :
 * MEMO         : 把core文件备份起来
 ********************************************************************/ 
int  WriteErrPacket(char *sFileName,char *lpBuffer)
{
	FILE		*lpLogFile;
	struct		timeb nowtime;
	int			nNumWritten;
	PacketHead	*lpPacketHead;
	char		sLog[300];
	char		sFullFileName[100];
	char		*pPath;

    pPath = AIcom_GetConfigString((char *)"Path",(char *)"Log");
    if(pPath==NULL) {
        return AI_NG;
    };
	sprintf(sFullFileName,"%s/%s",pPath,sFileName);
	lpLogFile=fopen(sFullFileName,"a");
	if (lpLogFile==NULL) { 
		sprintf(sLog,"Error: Fail to open log file:%s, err:%s",
					sFullFileName,strerror(errno));
		WriteLog((char *)RUN_TIME_LOG_FILE,sLog);
		return(AI_NG);
	}

	ftime(&nowtime);
	nNumWritten=fwrite(&nowtime, sizeof(nowtime), 1, lpLogFile);
	if (nNumWritten!=1)	{
		sprintf(sLog,"Error: Fail to write log file:%s, err:%s",
					sFileName,strerror(errno));
		WriteLog((char *)RUN_TIME_LOG_FILE,sLog);

		fclose(lpLogFile);
		return(AI_NG);
	}

	lpPacketHead=(PacketHead *)lpBuffer;
	nNumWritten=fwrite(lpBuffer, lpPacketHead->lPacketSize, 1, lpLogFile);
	if (nNumWritten!=1)	{
		sprintf(sLog,"Error: Fail to write log file:%s, err:%s",
					sFileName,strerror(errno));
		WriteLog((char *)RUN_TIME_LOG_FILE,sLog);

		fclose(lpLogFile);
		return(AI_NG);
	}

	fclose(lpLogFile);

	return(AI_OK);
}

/********************************************************************
 * NAME         : SetTraceFile
 * FUNCTION     : 设置跟踪信息文件
 * PARAMETER    : lpExtend(文件扩展名)
 * RETURN       : AI_OK=成功, AI_NG=失败
 * PROGRAMMER   : Jeffrey Du
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
int SetTraceFile(char *lpExtend,char *szConfigFileName)
{
	char	sFileName[100];
	char	*pPath;

    pPath = AIcom_GetConfigString((char *)"Path",(char *)"Log",szConfigFileName);
    if(pPath==NULL) {
        return AI_NG;
    };
	if(lpExtend!=NULL) {
		sprintf(sFileName,"%s/OUT%d.%s",pPath, getpid(), lpExtend);
	} else {
		strcpy(sFileName,"/dev/null");
	};

	if(freopen(sFileName, "w", stdout) == NULL) {
		return(AI_NG);
	}
	setbuf(stdout, NULL);

	return(AI_OK);
}

/********************************************************************
 * NAME         : AILogFile.h
 * FUNCTION     : Write LogFile 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    
 ********************************************************************/

#ifndef _AILOGFILE_
#define _AILOGFILE_

#define RUN_TIME_LOG_FILE			"RunTime.log"

int SetTraceFile(char *lpExtend,char *szConfigFileName);		// szConfigFileName为配置文件名

int AIcom_SysStartLog(char *szSysName,char *szUser);
int AIcom_SysEndLog(char * szSysName,char *szUser);
int WriteLog(char *sFileName, char *sLogString);
int WriteSystemInfo(char *sFileName, char *sSysInfo);
int ReturnNowTime(char *lpNow);
int FormatTime(time_t nNow, char *lpNow);
int WriteErrPacket(char *sFileName, char *lpBuffer);

#endif


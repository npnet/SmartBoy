/********************************************************************
 * NAME         : AIcom_profile.c 
 * FUNCTION     : profile file access 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 97/03/04
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/03/04  created by wtq
 ********************************************************************/

/*********************************************************************
 *      INCLUDE FILES
 ********************************************************************/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    "AIcom_Tool.h"
#include    "AI_PKTHEAD.h"
#include    "AI_ErrorNo.h"
#include    "AIcom_Tool.h"
#include    "AIprofile.h"
#include    "AIUComm.h"

static		char TMP_PATH_BUFFER[1024];
static      char szConfigFile[128]="";

/********************************************************************
 * NAME     : AIcom_StripSparc
 * FUNCTION : 去字符串中头尾空格
 * PROCESS  : 
 * INPUT    : szBuf1
 * OUTPUT   : 
 * UPDATE   : szBuf2
 * RETURN   : NONE
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
static void AIcom_StripSparc(char *szBuf1,char *szBuf2)
{
    int i,j;

    i = 0; 
    j = 0;
    while(szBuf1[i]==' '||szBuf1[i]=='\t') i++;
    while(szBuf1[i]!='\0' && szBuf1[i]!=' '&&szBuf1[i]!='\t') {
        szBuf2[j++] = szBuf1[i++];
    };
    szBuf2[j]='\0';
}

/********************************************************************
 * NAME     : AIcom_GetLine
 * FUNCTION : 从字符串中取得一行(包括换行符)
 * PROCESS  : 
 * INPUT    : 
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : 指向下一行的指针
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
static char *AIcom_GetLine(char *szBuf,char *szLine)
{
    int  i;

    i = 0;
    while(szBuf[i]!='\0'&&szBuf[i]!='\n') {
        szLine[i]=szBuf[i];
        i++;
    };
    szLine[i] = szBuf[i];
    szLine[i+1] = '\0';

    if(szBuf[i]=='\0') {
        return NULL;
    };

    return szBuf+i+1;
}

/********************************************************************
 * NAME     : AIcom_WriteProfileString
 * FUNCTION : write a string into profile file
 * PROCESS  : 1. open profile file 
 *            : 2. search for the segment 
 *            : 3. locate to the entry
 *            : 4. write string
 *            : 5. close file
 * INPUT      : 
 * OUTPUT     : 
 * UPDATE     : 
 * RETURN     : AI_OK : success
 *            : AI_NG : fail
 * PROGRAMMED : aisoft/aisoft
 * DATE(ORG)  : 
 * CALL       : 
 ********************************************************************/
static int AIcom_WriteProfileString(char *szSection,char *szEntry,char *szValue,char *szFileName)
{
    char *szBuf1,*szBuf2;
    long lFileLen;
    char szTmp[256];
    char *pPath,*p1,*p2;
    int  iFileExist;
    
    /* 看文件是否存在 */
    iFileExist = AIcom_GetFileAttr(szFileName,(char *)"A");
    if(iFileExist==AI_NG) {
        return AI_NG;
    };

    if(iFileExist==0) {  /* 文件存在 */
        iFileExist = AIcom_GetFileAttr(szFileName,(char *)"RW");
        if(iFileExist!=0) {
            return AI_NG;
        };
    };

    /* 取文件长度 */
    if(iFileExist==0) {
        lFileLen = AIcom_FileSize(szFileName);
    } else {
        lFileLen = 0;
    };

    if(lFileLen<0) {
        return AI_NG;
    };
    
    /* 分配MEMORY装文件内容 */
    szBuf1 = (char *)malloc(lFileLen+1);
    if(szBuf1 == NULL) {
        AIcom_SetErrorMsg(ERROR_MEMORYALLOC,NULL,NULL);
        return AI_NG;
    };
    memset(szBuf1,0,lFileLen+1);

    szBuf2 = (char *)malloc(lFileLen+80);
    if(szBuf2 == NULL) {
        AIcom_SetErrorMsg(ERROR_MEMORYALLOC,NULL,NULL);
        free(szBuf1);
        return AI_NG;
    };
    memset(szBuf2,0,lFileLen+80);

    /* 读入原文件 */
    if(iFileExist==0) {
        if(AIcom_GetFile(szFileName,szBuf1)==AI_NG) {
            free(szBuf1);
            free(szBuf2);
            return AI_NG;
        };
    } else {
        szBuf1[0] = '\0';
    };

    /* 先搜索段 */
    sprintf(szTmp,"[%s]",szSection);
    p1 = strstr(szBuf1,szTmp);

    if(p1==NULL) { /* 未发现段,在尾部加上 */
        if(strlen(szBuf1)>0) {
            sprintf(szBuf2,"%s\n[%s]\n%s=%s\n",szBuf1,szSection,
                szEntry,szValue);
        } else {
            sprintf(szBuf2,"[%s]\n%s=%s\n",szSection, szEntry,szValue);
        };
    } else { /* 找到该段 */
        char *p3;
    
        sprintf(szTmp,"%s=",szEntry);
        p3 = strstr(p1,szTmp);
        if(p3==NULL) { /* 未发现该入口,在段头部加 */
            memcpy(szBuf2,szBuf1,p1-szBuf1);
            szBuf2[p1-szBuf1] = '\0';
            p3 = AIcom_GetLine(p1,szTmp);
            strcat(szBuf2,szTmp);
            if(p3!=NULL) {
                sprintf(szBuf2,"%s%s=%s\n%s",szBuf2,szEntry,szValue,p3);
            } else {
                sprintf(szBuf2,"%s%s=%s\n",szBuf2,szEntry,szValue);
            }
        } else {  /* p3指向该入口 */
            /* 找下一段 */
            p2 = strstr(p1+1,"[");
            if(p2==NULL||p2>p3) { /* 无下一段 */
                 /* 该入口为正确的入口 */    
                memcpy(szBuf2,szBuf1,p3-szBuf1);
                szBuf2[p3-szBuf1] = '\0';
                p3 = AIcom_GetLine(p3,szTmp);
                if(p3!=NULL) {
                    sprintf(szBuf2,"%s%s=%s\n%s",szBuf2,szEntry,szValue,p3);
                } else {
                    sprintf(szBuf2,"%s%s=%s\n",szBuf2,szEntry,szValue);
                }
            } else {
                 /* 该入口为下一段的入口 */    
                memcpy(szBuf2,szBuf1,p1-szBuf1);
                szBuf2[p1-szBuf1] = '\0';
                p3 = AIcom_GetLine(p1,szTmp);
                sprintf(szBuf2,"%s%s=%s\n%s",szBuf2,szEntry,szValue,p3);
            };
        };
    };

    /* 写入原文件 */
    if(AIcom_PutFile(szFileName,szBuf2,strlen(szBuf2))==AI_NG) {
        free(szBuf1);
        free(szBuf2);
        return AI_NG;
    };

    free(szBuf1);
    free(szBuf2);
    return AI_OK;
}

/********************************************************************
 * NAME     : AIcom_WriteProfileInt
 * FUNCTION : write a string into profile file
 * PROCESS  : 1. open profile file 
 *          : 2. search for the segment 
 *          : 3. locate to the entry
 *          : 4. write int
 *          : 5. close file
 * INPUT    : 
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : AI_OK : success
 *          : AI_NG : fail
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
static int AIcom_WriteProfileInt(char *szSection,char *szEntry,int iValue,char *szFileName)
{
    char szBuf[10];

    sprintf(szBuf,"%d",iValue);

    return AIcom_WriteProfileString(szSection,szEntry,szBuf,szFileName);
}

/********************************************************************
 * NAME     : AIcom_GetProfileString
 * FUNCTION : 
 * PROCESS  : 
 * INPUT    : 
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : NONE
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
static void AIcom_GetProfileString(char *szSection,char *szEntry,char *szDefault,char *szValue,int len, char *szFileName)
{
    char *szBuf1;
    long lFileLen;
    char szTmp[256],t1[256],t2[256];
    char *pPath,*p1,*p2,*p3;
    int  iFileExist;
    
    if(strlen(szDefault)>len) {
        memcpy(szValue,szDefault,len);
        szValue[len] = 0;
    } else {
        strcpy(szValue,szDefault);
    };

    iFileExist = AIcom_GetFileAttr(szFileName,(char *)"A");
    if(iFileExist==AI_NG) {
        return;
    };

    if(iFileExist==1) {  /* 文件不存在 */
        return;
    };

    iFileExist = AIcom_GetFileAttr(szFileName,(char *)"R");
    if(iFileExist!=0) {
        return;
    };

    /* 取文件长度 */
    lFileLen = AIcom_FileSize(szFileName);
    if(lFileLen<0) {
        return;
    };
    
    /* 分配MEMORY装文件内容 */
    szBuf1 = (char *)malloc(lFileLen+1);
    if(szBuf1 == NULL) {
        return;
    };
    memset(szBuf1,0,lFileLen+1);

    /* 读入原文件 */
    if(AIcom_GetFile(szFileName,szBuf1)==AI_NG) {
        free(szBuf1);
        return;
    };

    /* 先搜索段 */
    sprintf(szTmp,"[%s]",szSection);
    p1 = strstr(szBuf1,szTmp);

    if(p1==NULL) { /* 未发现段 */
	free(szBuf1);
        return;
    }; 

    /* 找到该段 */
    sprintf(szTmp,"%s=",szEntry);
    p2 = strstr(p1,szTmp);
    if(p2==NULL) { /* 未发现该入口 */
	free(szBuf1);
        return;
    };

    /* p2指向该入口,找下一段 */
    p3 = strstr(p1+1,"[");
    if(p3!=NULL&&p2>p3) { /* 该入口为下一段的入口 */
	free(szBuf1);
        return;
    };

    /* 该入口为正确的入口 */    
    AIcom_GetLine(p2,szTmp);
    free(szBuf1);

    sscanf(szTmp,"%[^=]=%[^\n]",t1,t2);

    AIcom_StripSparc(t2,t1);

    if(strlen(t1)>len) {
        memcpy(szValue,t1,len);
        szValue[len] = 0;
    } else {
        strcpy(szValue,t1);
    };

    return;
}

/********************************************************************
 * NAME     : AIcom_GetProfileInt
 * FUNCTION : 
 * PROCESS  : 
 * INPUT    : 
 * OUTPUT   : 
 * UPDATE   : 
 * RETURN   : NONE
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG): 
 * CALL     : 
 ********************************************************************/
static void AIcom_GetProfileInt(char *szSection,char *szEntry,int iDefault,int *piValue, char *szFileName)
{
    char szDefault[10];
    char szValue[20];

    sprintf(szDefault,"%d",iDefault);

    AIcom_GetProfileString(szSection,szEntry,szDefault,szValue,10,szFileName);
    *piValue = atoi(szValue);
}

char *AIcom_GetConfigString(char *szSection, char *szEntry)
{
	if(szConfigFile[0]!='\0') {
		return AIcom_GetConfigString(szSection, szEntry, szConfigFile);
	};
	return NULL;
}

char *AIcom_GetConfigString(char *szSection, char *szEntry, char *szFileName)
{
	if(szConfigFile[0]=='\0') {
		strcpy(szConfigFile,szFileName);
	};

	AIcom_GetProfileString(szSection, szEntry, (char *)"NONE", TMP_PATH_BUFFER, 1024, szFileName);
	if(strcmp(TMP_PATH_BUFFER,"NONE")==0) {
		return NULL;
	};
	return TMP_PATH_BUFFER;
}

/********************************************************************
 * NAME         : AIprofile.h
 * FUNCTION     : 
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    97/07/03    created by wtq
 ********************************************************************/
#ifndef _AIPROFILE_
#define _AIPROFILE_

char *AIcom_GetConfigString(char *szSection, char *szEntry,char *szFileName);
char *AIcom_GetConfigString(char *szSection, char *szEntry);
int AIcom_WriteProfileString(char *szSection,char *szEntry,char *szValue,char *szFileName);
int AIcom_WriteProfileInt(char *szSection,char *szEntry,int iValue,char *szFileName);

#endif


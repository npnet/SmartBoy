/********************************************************************
 * NAME         : AIcom_File.h
 * FUNCTION     : Define many File function.
 * PROGRAMMED   : aisoft/aisoft
 * DATE(ORG)    : 
 * PROJECT      : aisoft
 * OS           : DEC ALPHA
 * HISTORY      :
 * ID  -- DATE -------- NOTE-----------------------------------------
 * 00    
 ********************************************************************/

#ifndef _AICOMFILE_
#define _AICOMFILE_

void AIcom_GetCurrentDate(char *szBuf,int iFlag);
long AIcom_FileSize(char *szFileName);
int  AIcom_GetFileAttr(char *szFileName,char *szMode);
int  AIcom_GetFile(char *szFileName,char *szBuf);
int  AIcom_SetFileAttr(char *szFileName,int Mode);
int  AIcom_CreateFile(char *szFileName);
int  AIcom_PutFile(char *szFileName,char *szBuf,int len);
int  AIcom_CreateDir(char *szDirName);
int  AIcom_ChangeToDaemon();

int  AIcom_CreateSem(int sem_key);			// 返回semid
int  AIcom_InitSem(int semid);				// 初始化信号灯
void AIcom_P_Sem(int semid);				// P操作
void AIcom_V_Sem(int semid);				// V操作
int  AIcom_DelSem(int semid);				// 删除信号灯

int  AIcom_CreateShm(int shm_key,int nSize);// 返回shmid
char *AIcom_AttachShm(int shmid);			// 绑定到本进程
int  AIcom_DetachShm(char *sShm);			// 与本进程解绑共享内存
int  AIcom_DelShm(int shmid);				// 删除该共享内存

#endif

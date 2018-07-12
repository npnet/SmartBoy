#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

union semun {
  int                val;           /* for setval */
  struct semid_ds    *buf;          /* ipc_stat ipc_set */
  unsigned short    *array;           /* ipc_getall ipc_setall */
} sem_arg;

static struct sembuf    p_system={0,-1, 0};
static struct sembuf    v_system={0, 1, 0};

/*********************************************************************
 * NAME         : AIcom_CreateSem
 * FUNCTION     : Semphone create
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : >=0  semid
 *              : <0   AI_NG;
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
int AIcom_CreateSem(int sem_key)
{
    int semid;

    semid=semget(sem_key,1,0640|IPC_EXCL);
    if(semid >=0) {
         return semid;
    };

    semid=semget(sem_key,1,0640|IPC_CREAT);
    if(0 > semid) {
         printf("Semaphore %d create fail!\n",sem_key);
         return -1;
    }

    return semid;
}

/*********************************************************************
 * NAME         : AIcom_InitSem
 * FUNCTION     : Semphone initialization
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : AI_OK
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
int AIcom_InitSem(int semid)
{
    sem_arg.val=1;
    return semctl(semid,0,SETVAL,sem_arg);
}

/*********************************************************************
 * NAME         : AIcom_DelSem
 * FUNCTION     : Semphone Delete
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : AI_OK
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
int AIcom_DelSem(int semid)
{
    return semctl(semid,0,IPC_RMID,0);
}

/********************************************************************
 * NAME         : AIcom_P_Sem
 * FUNCTION     : 信号灯P操作
 * PARAMETER    : semid
 * RETURN       : 
 * PROGRAMMER   : aisoft/aisoft 
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
void AIcom_P_Sem(int semid)
{
    while(1) {
        if(semop(semid,&p_system,1) == -1) {
            if(errno == EINTR) {
                continue;
            }
            printf("*** semop %d error(AIcom_P_Sem)!, errno:%d\n", semid, errno);
			return;
        }
        break;        
    }
}

/********************************************************************
 * NAME         : AIcom_V_Sem
 * FUNCTION     : 信号灯V操作
 * PARAMETER    : semid
 * RETURN       : 
 * PROGRAMMER   : aisoft/aisoft 
 * DATE(ORG)    : 98/06/27
 * UPDATE       :
 * MEMO         : 
 ********************************************************************/ 
void AIcom_V_Sem(int semid)
{
    while(1) {
        if(semop(semid,&v_system,1) == -1) {
            if(errno == EINTR) {
                continue;
            }
            printf("*** semop %d error(AIcom_V_Sem)!, errno:%d\n", semid, errno);
            return;
        } 
        break;        
    }
}

/*********************************************************************
 * NAME         : AIcom_CreateShm
 * FUNCTION     : Share memory create
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : >=0  shmid
 *              : <0   AI_NG;
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
int AIcom_CreateShm(int shm_key,int nSize)
{
    int shmid;

    shmid=shmget(shm_key,nSize,0640|IPC_EXCL);
    if(shmid >=0) {
         return shmid;
    };

    shmid=shmget(shm_key,nSize,0640|IPC_CREAT);
    if(0 > shmid) {
         printf("Share Memroy %d %dcreate fail! err:%d\n",shm_key,nSize,errno);
         return -1;
    }

    return shmid;
}

/*********************************************************************
 * NAME         : AIcom_AttachShm
 * FUNCTION     : Share Memory attach
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : AI_OK
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
char *AIcom_AttachShm(int shmid)
{
    return (char *)shmat(shmid,0,0);
}

/*********************************************************************
 * NAME         : AIcom_DetachShm
 * FUNCTION     : Share Memory Detach
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : AI_OK
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
int AIcom_DetachShm(char *sShm)
{
    return shmdt(sShm);
}

/*********************************************************************
 * NAME         : AIcom_DelShm
 * FUNCTION     : Share Memory Delete
 * PROCESS      : 
 * PROGRAMMED   : aisoft/aisoft
 * RETURN       : AI_OK
 * DATE(ORG)    : 94.09.10
 ********************************************************************/
int AIcom_DelShm(int shmid)
{
    return shmctl(shmid,IPC_RMID,0);
}

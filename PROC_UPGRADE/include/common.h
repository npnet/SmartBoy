//
// Created by sine on 18-7-11.
//

#ifndef DVLP_COMMON_H
#define DVLP_COMMON_H



typedef unsigned char uchar;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long  uint64;

typedef signed int     int32;
typedef signed short    int16;

typedef signed long long  int64;

typedef uint8  boolean;
#define True  (1)
#define False (0)



#ifdef ISNEED
uint32 ui4_disable_print;
uint32 ui4_enable_all_log;

/* Retun values. */
#define DBGR_OK                 ((int32)   0)
#define DBGR_OPEN_FAIL          ((int32)  -1)
#define DBGR_INV_ARG            ((int32)  -2)
#define DBGR_NOT_ENOUGH_MEM     ((int32)  -3)
#define DBGR_ALREADY_INIT       ((int32)  -4)
#define DBGR_NOT_INIT           ((int32)  -5)
#define DBGR_NO_TRACE_BUFFER    ((int32)  -6)
#define DBGR_NO_OUTPUT_DEVICE   ((int32)  -7)
#define DBGR_INV_OUTPUT_DEVICE  ((int32)  -8)
#define DBGR_NOT_ENABLED        ((int32)  -9)
#define DBGR_DUMP_IN_PROGRESS   ((int32) -10)
#define DBGR_REG_CB_ACTIVE      ((int32) -11)


/* Debug level defines. */
#define DBG_LEVEL_NONE   ((uint16) 0x0000)
#define DBG_LEVEL_ERROR  ((uint16) 0x0001)
#define DBG_LEVEL_API    ((uint16) 0x0002)
#define DBG_LEVEL_INFO   ((uint16) 0x0004)
#define DBG_LEVEL_SANITY   ((uint16) 0x0008)
//#define DBG_LEVEL_ALL    ((uint16) 0xffff)
#define DBG_LEVEL_ALL    ((uint16) 0x00ff)
#define DBG_LEVEL_MASK   DBG_LEVEL_ALL

#define DBG_LAYER_APP    ((uint16) 0x0100)
#define DBG_LAYER_MMW    ((uint16) 0x0200)
#define DBG_LAYER_MW     ((uint16) 0x0400)
#define DBG_LAYER_SYS    ((uint16) 0x0800)
#define DBG_LAYER_DRV    ((uint16) 0x1000)
#define DBG_LAYER_ALL    ((uint16) 0xFF00)
#define DBG_LAYER_MASK   DBG_LAYER_ALL



/*control by /data/log_all file exist or not*/
#define printf(format, args...)                \
do{                                            \
    if(1 == ui4_enable_all_log)                \
	{                                      \
		printf(format, ##args);        \
   	}                                      \
}while(0)

#define DBG_PRINT(format, args...)                      \
do{                                                     \
    if ((ui4_disable_print & DBG_LAYER_MASK) == 0)          \
        printf(format,##args);                          \
}while(0)

#define DBG_LVL_PRINT(set_layer_lvl, lvl, format, args...)     \
do{                                                     \
    if (ui4_disable_print & (set_layer_lvl & DBG_LAYER_MASK))     \
        break;                                          \
    if ((lvl & (set_layer_lvl & DBG_LEVEL_MASK)) != 0)        \
        printf(format,##args);                          \
}while(0)

#define DBG_ERROR_EX(stat...)                                     \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_ERROR, stat)

#define DBG_API_EX(stat...)                                       \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_API, stat)

#define DBG_INFO_EX(stat...)                                     \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_INFO, stat)

#define DBG_SANITY_EX(stat...)                                    \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_INFO, stat)

#undef DBG_ERROR
#undef DBG_API
#undef DBG_INFO

#define DBG_ERROR(_stmt)  DBG_ERROR_EX _stmt
#define DBG_API(_stmt)    DBG_API_EX _stmt
#define DBG_INFO(_stmt)   DBG_INFO_EX _stmt
#if 1
#define DBG_API_IN
#define DBG_API_OUT
#else
#define DBG_API_IN  printf("IN  -> %s\n",__FUNCTION__)
#define DBG_API_OUT printf("OUT -> %s\n",__FUNCTION__)
#endif
#define DBG_LOG_HERE printf("%s:%d\n",__FUNCTION__,__LINE__)

#define dbg_print(stat...) DBG_PRINT(stat)


#define CHECK_FAIL(func) do\
	{\
		int ret = func; \
		if(ret!=0)DBG_ERROR(("fail ret=%d\n",ret));\
	}while(0)
#define CHECK_FAIL_RET(func) do\
	{\
		int ret = func; \
		if(ret!=0){DBG_ERROR(("fail ret=%d\n",ret));return ret;}\
	}while(0)

/* Macro for debug abort / assert. */
#define DBG_ABORT(_code)  do{printf("%s,%d\n",((CHAR*) __FILE__), ((UINT32) __LINE__)); pthread_exit(_code);}while(0)
#define DBG_ASSERT(_expr, _code)  { if (! (_expr)) DBG_ABORT (_code); }

#define dbg_abort(_code)  DBG_ABORT (_code)


extern int dbg_init(void);
#endif

#endif //DVLP_COMMON_H

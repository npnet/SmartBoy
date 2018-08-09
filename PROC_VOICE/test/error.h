#ifndef __ERROR_H__
#define __ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

enum
{
	SUCCESS								= 0,
	ERROR_FAIL							= -1,
	ERROR_EXCEPTION						= -2,

	/* General errors 10100(0x2774) */
	ERROR_GENERAL						= 10100, 	
	ERROR_OUT_OF_MEMORY					= 10101, 	
	ERROR_OPEN_FILE						= 10102, 	
	
};


#ifdef __cplusplus
} /* extern "C" */	
#endif /* C++ */

#endif /* __RAECORD_H__ */


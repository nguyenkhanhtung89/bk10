#ifndef __RFTRANS_95HF_MISC_H
#define __RFTRANS_95HF_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
	 
#define MAX(x,y) 				((x > y)? x : y)
#define MIN(x,y) 				((x < y)? x : y)  
#define ABS(x) 					((x)>0 ? (x) : -(x))  
#define CHECKVAL(val, min,max) 	((val < min || val > max) ? false : true) 
	 
#define GETMSB(val) 		((val & 0xFF00 )>>8 ) 
#define GETLSB(val) 		( val & 0x00FF ) 
 
#define RESULTOK 							0x00 
#define ERRORCODE_GENERIC 		1

#ifndef errchk
#define errchk(fCall) if (status = (fCall), status != RESULTOK) \
	{goto Error;} else
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_H */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _LIB_95HF_H
#define _LIB_95HF_H


/* Includes ------------------------------------------------------------------*/
#include "drv_95HF.h"
#include "common.h"

#include "lib_nfctype5pcd.h"

/* Analogue configuration register for protocol initialization -------------------------------------------*/
//#define TIMER_WINDOW_REG_ADD												0x3A
//#define TIMER_WINDOW_UPDATE_CONFIRM_CMD							0x04


#define AFE_ANALOG_CONF_REG_SELECTION               0x68
//#define AFE_ANALOG_CONF_REG_UPDATE                  0x69

//#define AFE_ACCONFIGA_OFFSET												0x04

/* ROM CODE Revision  --------------------------------------------------------------*/
//#define ROM_CODE_REVISION_OFFSET			13

/* RFtrans 95HF family command definition  ---------------------------------------------------------------*/
#define IDN																					0x01
#define PROTOCOL_SELECT 														0x02
#define POLL_FIELD 																	0x03
#define SEND_RECEIVE																0x04
#define LISTEN																			0x05
#define SEND																				0x06
#define IDLE																				0x07
#define READ_REGISTER																0x08
#define WRITE_REGISTER															0x09
#define BAUD_RATE																		0x0A
#define SUB_FREQ_RES																0x0B
#define AC_FILTER																		0x0D
#define TEST_MODE																		0x0E
#define SLEEP_MODE																	0x0F
#define ECHO																				0x55


/*  poll field status ------------------------------------------------------------------ */
//#define POLLFIELD_RESULTSCODE_OK										0x00



//  Send command field status
//#define SEND_RESULTSCODE_OK													0x00
//#define SEND_ERRORCODE_LENGTH												0x82
//#define SEND_ERRORCODE_PROTOCOL											0x83
/*  Idle command field status ----------------------------------------------------------------- */
#define IDLE_RESULTSCODE_OK													0x00
#define IDLE_ERRORCODE_LENGTH												0x82
/*  read register command field status -------------------------------------------------------- */
#define READREG_RESULTSCODE_OK											0x00
#define READREG_ERRORCODE_LENGTH										0x82
/*  write register command field status ------------------------------------------------------- */
#define WRITEREG_RESULTSCODE_OK											0x00
/*  Baud rate command field status ------------------------------------------------------------ */
//#define BAUDRATE_RESULTSCODE_OK											0x55
/*  AC filter command field status ------------------------------------------------------------ */
//#define ACFILTER_RESULTSCODE_OK											0x00
//#define ACFILTER_ERRORCODE_LENGTH										0x82
/*  sub freq command field status ------------------------------------------------------------- */
//#define SUBFREQ_RESULTSCODE_OK											0x00
/*  Test mode command field status ------------------------------------------------------------ */
//#define TESTMODE_RESULTSCODE_OK											0x00

//#define ASK_FOR_SESSION											0x0000
//#define TAKE_SESSION												0xFFFF

//#define XX95_ACTION_COMPLETED							  0x9000

#endif /* _LIB_95HF_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/




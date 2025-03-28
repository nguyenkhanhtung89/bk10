#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "cmsis_os.h"
#include "serial.h"
#include "interface.h"
#include "j1939.h"
#include "canbus.h"
#include "utils.h"

#define CANBUS_MAX_FAULTCODE		16

#define ODBII_PID_REQUEST				0x7DF
#define ODBII_PID_REQUEST_END		0x7EF
#define ODBII_PID_REPLY					0x7E8
#define ODBII_PID_REQUEST2			0x7E0

#define	PID_MODE_01					0x01
#define	PID_MODE_03					0x03
#define	PID_MODE_07					0x07
#define	PID_MODE_0A					0x0A
#define	PID_MODE_09					0x09

#define PID_01_SUPPORTED		0x00
#define OBD_DTC							0x01
#define OBD_ENGINE_COOLANT	0x05
#define OBD_ENGINE_RPM			0x0C
#define OBD_VEHICLE_SPEED		0x0D
#define OBD_MAF_SENSOR			0x10
#define OBD_THROTTLE				0x11
#define OBD_O2_VOLTAGE			0x14
#define OBD_FUELLEVEL				0x2F
#define OBD_TOTAL_KM				0x31
#define OBD_ODOMETER				0xA6
#define OBD_BATTERY					0x42
#define OBD_VIN_RECEIVE			0x49

#define OBD_VIN_REQUEST			0x02

#define CAN_RES_TIMEOUT 100

#define J1939_SRC_ADRR	249

#define J1939_DM1		65226		/* Diagnostic Message 1 -> Active Diagnostic Trouble Codes */
#define J1939_DM2		65227		/* Diagnostic Message 2 -> Previousl Active Diagnostic Trouble Codes */
#define J1939_DM3		65228		/* Diagnostic Message 3 -> Diagnostic Data Clear/Reset for previously active DTCs */
#define J1939_DM4		65229		/* Diagnostic Message 4 -> Freeze Frame Parameters */
#define J1939_DM5		65230 	/* Diagnostic Message 5 -> Diagnostic Readiness */
#define J1939_DM6		65231 	/* Diagnostic Message 6 -> Continuously Monitored Systems Test Results */
#define J1939_DM7		58112  	/* Diagnostic Message 7 -> Command Non-Continuously Monitored Test */
#define J1939_DM8		65232  	/* Diagnostic Message 8 -> Test Results for non-continuously monitored systems */
#define J1939_DM9		65233 	/* Diagnostic Message 9 -> Oxygen Sensor Test Results */
#define J1939_DM10	65234 	/* Diagnostic Message 10 -> Non-Continuously Monitored Systems Test Identifier Support */
#define J1939_DM11	62235		/* Diagnostic Message 11 -> Diagnostic Data Clear/Reset for Active DTCs */

#define J1939_ADDC	60928		/* Address Claimed */

CanTxMsg TxMessage;

CanRxMsg RxMessage;

ODBDef canbus;

//u32 val;

int32_t time_irq ;
/* fix tempory */
u32 PGN = 0, SPN = 0;  							// 18bit
u8 nSrc = 0, FMI = 0, Count = 0;		// 8bit
u8 nPriority = 0, idx;							// 3bit

double d_val = 0;

static void ECU_SendFaultCode(void);

u8 iLampStatus, sLampStatus, wLampStatus, pLampStatus;

FaultCodeJ1939 sFaultCodeJ1939[CANBUS_MAX_FAULTCODE];

FaulCodeODBII sFaultCodeODB[CANBUS_MAX_FAULTCODE];

u8 sFaultCodeSize = 0;

FaultCodeJ1939 fc;

u8 s_fc_send = 0;

u16 idx_dtc = 0;

u8 flg_dtc = 0;

u8 flg_has = 0;

char tmp_src[128];

static void CAN_Config(int baud);

static void ODBIIHandler(CanRxMsg* RxMessage);

static void J1939Handler(CanRxMsg* RxMessage);

static uint8_t J1939_Request_PGN(uint32_t s_png, uint32_t d_pgn);

static u8 SendRequest(u8 mode, u8 pid_code) ;

/* Thread OBD task parser */
osThreadId vJ1939ParserThread;
osThreadDef(vJ1939Task, osPriorityNormal, 1, 1024);


uint8_t CAN_service_init(void) {
	if((mcu_config.TypeOBD != J1939_TYPE) && (mcu_config.TypeOBD != OBDII_TYPE)){
		return 0;
	}
	
	vJ1939ParserThread = osThreadCreate(osThread(vJ1939Task), NULL);
	if(!vJ1939ParserThread){
		return 0;
	}
	return 1;
}

void vJ1939Task(const void* args){
	u16 StepResq = 0;

	/* Get data blackbox */
	if(mcu_config.TypeOBD == J1939_TYPE){
		CAN_Config(250);		
	}else if(mcu_config.TypeOBD == OBDII_TYPE){
		CAN_Config(500);
	}else{
		return;	
	}
	
	/* Task manager */
	for(;;){
		osDelay(1000);
			
		/* Check allow */
		if(mcu_config.TypeOBD == OBD_NONE){
			CAN_DeInit(CAN1);
			/* Clear data */
			SetECUClearData();
			
			return;
		}

		/* Acc off */
		if(ACC_STATUS == 0){
			/* Clear data */
			SetECUClearData();
			osDelay(1000);
			continue;
		}		  
		
//sprintf(tmp_src ,"RPM: %d, SP: %d, TEMP: %d, FUEL: %u, HOUR: %u, KM: %u, BAT: %u", canbus.rpm, canbus.sp, canbus.temp, canbus.s_fuel, canbus.s_hour, canbus.s_km, canbus.bat);
//SerialPushStr(tmp_src, END_LF);

		if(mcu_config.TypeOBD == J1939_TYPE){
			/* Check for request */
			switch(StepResq){
				case 10:
					J1939_Request_PGN(59905, 65248); 		/* Total Vehicle Distance Request */
					break;
				case 11:
					J1939_Request_PGN(60153, 65248); 		/* Total Vehicle Distance Request */
					break;
				case 12:
					J1939_Request_PGN(60159, 65248); 		/* Total Vehicle Distance Request */
					break;
				case 15:
					J1939_Request_PGN(59905, 65253);		/* Total Engine Hours (On request) */
					break;
				case 16:
					J1939_Request_PGN(60159, 65253);		/* Total Engine Hours (On request) */
					break;
				case 20:
					J1939_Request_PGN(59905, 65257);		/* Fuel Consumption (On request) */
					break;
				case 21:
					J1939_Request_PGN(60159, 65257);		/* Fuel Consumption (On request) */
					break;
				case 22:
					J1939_Request_PGN(59905, 65226);		/* Active Diagnostic Trouble Codes */
					break;
				case 30:
					J1939_Request_PGN(60153, 65226);		/* Active Diagnostic Trouble Codes */
					break;
				case 31:
					J1939_Request_PGN(60159, 65226);		/* Active Diagnostic Trouble Codes */
					break;
				default:
					break;
			}
			
			/* Send fault code when reconnected */
			if((StepResq > 45) && (s_fc_send == 0)) {
				StepResq = 0;
				
				/* Send fault code to server */
				if(sFaultCodeSize > 0){
					ECU_SendFaultCode();
				}
				
				s_fc_send = 1;
			}
		}else if(mcu_config.TypeOBD == OBDII_TYPE){
			/* Reset time */
			switch(StepResq){
				case 1:
					SendRequest(PID_MODE_09, OBD_VIN_REQUEST);
					break;
				case 2:
					SendRequest(PID_MODE_09, OBD_VIN_REQUEST);
					break;
				case 3:
					SendRequest(PID_MODE_09, OBD_VIN_REQUEST);
					break;
				case 4:
					SendRequest(PID_MODE_01, OBD_ENGINE_COOLANT);
					break;
				case 5:
					SendRequest(PID_MODE_01, OBD_ENGINE_RPM);
					break;
				case 6:
					SendRequest(PID_MODE_01, OBD_VEHICLE_SPEED);
					break;
				case 7:
					SendRequest(PID_MODE_01, OBD_FUELLEVEL);
					break;
				case 8:
					SendRequest(PID_MODE_01, OBD_TOTAL_KM);
					break;
				case 9:
					SendRequest(PID_MODE_01, OBD_ODOMETER);
					break;
				case 10:
					SendRequest(PID_MODE_01, OBD_ODOMETER);
					break;
				case 13:  
					SendRequest(2, 1);
					break;
				case 14: 
					SendRequest(PID_MODE_03, 0);
					osDelay(50);
					SendRequest(PID_MODE_03, 0);
					osDelay(50);
					SendRequest(PID_MODE_03, 0);
					break;
				case 15: 
					SendRequest(PID_MODE_07, 0);
					osDelay(50);
					SendRequest(PID_MODE_07, 0);
					osDelay(50);
					SendRequest(PID_MODE_07, 0);
					break;
				case 16:
					SendRequest(PID_MODE_0A, 0);
					osDelay(50);
					SendRequest(PID_MODE_0A, 0);
					osDelay(50);
					SendRequest(PID_MODE_0A, 0);
					break;
				default: break;
			}
			/* Fault code send server */
			if((sFaultCodeSize > 0) && (s_fc_send == 0)){
				StepResq = 0;
				
				ECU_SendFaultCode();

				s_fc_send = 1;
			}
		}
		
		if(StepResq > 17){
			StepResq = 0;
		}
		
		/* Increment step */
		StepResq += 1;
	}
}


void USB_LP_CAN1_RX0_IRQHandler(void){
	if(CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET){
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
	}
	
	if(RxMessage.IDE == CAN_ID_STD){					/* ODB-II */
		ODBIIHandler(&RxMessage);	
	}else if(RxMessage.IDE == CAN_ID_EXT){		/* J1939 */
		J1939Handler(&RxMessage);
	}
	
	CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
}

static uint8_t SendRequest_continuos(void)
{
	TxMessage.StdId		= ODBII_PID_REQUEST2;
	TxMessage.ExtId		= 0;
	TxMessage.RTR		= CAN_RTR_DATA;
	TxMessage.IDE		= CAN_ID_STD;
	TxMessage.DLC		= 8;
	
	/* Set length request */
	TxMessage.Data[0] = 0x30;
	TxMessage.Data[1]	= 0;
	TxMessage.Data[2]	= 0;
	TxMessage.Data[3]	= 0;
	TxMessage.Data[4]	= 0;
	TxMessage.Data[5]	= 0;
	TxMessage.Data[6]	= 0;
	TxMessage.Data[7]	= 0;
	
	return CAN_Transmit(CANx, &TxMessage);
}

static void ODBIIHandler(CanRxMsg* RxMessage)
{
	u8 idx = 0;
	if((RxMessage->StdId >= ODBII_PID_REQUEST) && (RxMessage->StdId <= ODBII_PID_REQUEST_END))
	{
		canbus.connected = 1;
		
		/* Read fault code */		
		if(RxMessage->Data[1] == 0x43 || RxMessage->Data[1] == 0x47)
		{
			if(RxMessage->Data[2] == 0){
				return;
			}
			else
			{
				idx = 3;
				if(RxMessage->Data[2] > 0x02)
				{
					flg_dtc = 1;
					idx_dtc = 0;
					flg_has = RxMessage->Data[2] - 2;
				}
			}
			while(idx < RxMessage->Data[0])
			{
				/* Store fault code */
				StoreFaultCodeODBII(RxMessage->Data[idx], RxMessage->Data[idx + 1]);
				
				/* Imcream indexs */
				idx += 2;
			}
		}
		if(flg_dtc == 1){
			idx = 0;
			if(idx_dtc < flg_has /4 + 1){
				while(idx < (flg_has % 8)) 
				{
					StoreFaultCodeODBII(RxMessage->Data[idx], RxMessage->Data[idx + 1]);
					/* Imcream indexs */
					idx += 2;
//					sprintf(tmp_src ,"DTC: %02x %02x %02x %02x %02x %02x %02x %02x : %d", RxMessage->Data[0], RxMessage->Data[1], RxMessage->Data[2], RxMessage->Data[3], RxMessage->Data[4], RxMessage->Data[5], RxMessage->Data[6], RxMessage->Data[7], flg_has);
//					SerialPushStr(tmp_src, END_LF);
				}
			}
		}
		idx_dtc ++;
	
		if(RxMessage->Data[0] == 0x21){
//			SerialPushStr("V1", END_LF);
			memcpy(canbus.VIN_ID + 3, RxMessage->Data + 1, 7);
		}else if(RxMessage->Data[0] == 0x22){
//			SerialPushStr("V2", END_LF);
			memcpy(canbus.VIN_ID + 10, RxMessage->Data + 1, 7);
		}
		
	//	SerialPushStr(RxMessage->Data + 1, END_LF);
		
		if(RxMessage->Data[1] == 0x41){
		/* Get information ECU */
		switch(RxMessage->Data[2]){
			case OBD_ENGINE_COOLANT:
				canbus.temp = (RxMessage->Data[3]) - 40;
				break;
			case OBD_ENGINE_RPM:
				canbus.rpm = (256 * RxMessage->Data[3] + RxMessage->Data[4]) / 4;
				break;
			case OBD_VEHICLE_SPEED:
				canbus.sp = RxMessage->Data[3];
				break;
			case OBD_FUELLEVEL:
				canbus.p_fuel = ((double)(RxMessage->Data[3])) * 100.0 / 255.0;
				break;
			case OBD_TOTAL_KM:
				canbus.s_km = (256 * RxMessage->Data[3] + RxMessage->Data[4]);
				break;
			case OBD_ODOMETER:
				canbus.odometer = bytes_to_u32(RxMessage->Data + 3)/10;
				break;
			case OBD_BATTERY:
				canbus.bat = ((double)RxMessage->Data[3] * 256.0 + RxMessage->Data[4]);
				break;
			case OBD_MAF_SENSOR:
			case OBD_THROTTLE:
			case OBD_O2_VOLTAGE:
			case OBD_DTC:		
				break;
			default:
				break;
			}
		}else if(RxMessage->Data[1] == 0x49){
			if(RxMessage->Data[2] == OBD_VIN_RECEIVE){
//				SerialPushStr("V0", END_LF);
				canbus.VIN_ID[0] = RxMessage->Data[5];
				canbus.VIN_ID[1] = RxMessage->Data[6];
				canbus.VIN_ID[2] = RxMessage->Data[7];
				SendRequest_continuos();
			}
		}
	}
}

/*******************************************************************************
* Function Name	:	J1939Handler
* Description	:	J1939 parser
*******************************************************************************/
static void J1939Handler(CanRxMsg* RxMessage){
	u32 temp = 0;
	
	canbus.connected = 1;
	
	/* Get 8 bit source address */
	nSrc = (RxMessage->ExtId & 0xFF);
	
	/* Get 18 bit parameter group number PGN */
	PGN = ((RxMessage->ExtId >> 8) & 0x3FFFF);
	
	/* Get 3 bit priority */
	nPriority = ((RxMessage->ExtId >> 24) & 0x03);
	
	switch(PGN){
		/************************************************************************
		* http://www.simmasoftware.com/sae-j1939-73.html
		*	PGN 65226             Active Diagnostic Codes
 
		*	Transmission Repetition:	See J1939-73 spec
		*	Data Length:	Variable
		*	Default Priority:	6
		*	Parameter Group Number:	65226
			 
		*	Start Position	Length	 Parameter Name	SPN
		*	1.8-1.7	2 bits	Malfunction Indicator Lamp Status	1213
		*	1.6-1.5	2 bits	Red Stop Lamp Status	623
		*	1.4-1.3	2 bits	Amber Warning Lamp Status	624
		*	1.2-1.1	2 bits	Protect Lamp Status	987
		*	3-6	4 bytes	DTC
		*************************************************************************/
		case 65226:				
			iLampStatus =	(RxMessage->Data[0] >> 6) & 0x03;
		
			sLampStatus =	(RxMessage->Data[0] >> 4) & 0x03;
		
			wLampStatus =	(RxMessage->Data[0] >> 2) & 0x03;
		
			pLampStatus =	RxMessage->Data[0] & 0x03;
			
			/*  Parser DTC */
			//if(RxMessage->DLC > 2 && (RxMessage->DLC - 2) % 4 == 0)
			if(RxMessage->DLC > 2){
				u8 nDTC = (RxMessage->DLC - 2) / 4;
				u8 idx = 0, pos = 0;
				
				/* Set max size */
				nDTC = (nDTC > 16) ? 16 : nDTC;
								
				/* Get DTCs */
				for(idx = 0; idx < nDTC; idx++){
					pos = 4 * idx + 2;
					
					SPN = ((RxMessage->Data[pos + 2] >> 5) << 16) | (RxMessage->Data[pos + 1] << 8) | RxMessage->Data[pos];
					
					FMI = RxMessage->Data[pos + 2] & 0x1F;
					
					Count = RxMessage->Data[pos + 3] & 0x7F;
					
					/* Set fault code */
					fc.SPN = SPN;
					fc.FMI = FMI;
					fc.Count = Count;
					fc.iLampStatus = iLampStatus;
					fc.sLampStatus = sLampStatus;
					fc.wLampStatus = wLampStatus;
					fc.pLampStatus = pLampStatus;
					
					StoreFaultCode(fc);
				}
			}
			break;
		case 65262:				/* Engine Temperature */
			canbus.temp = RxMessage->Data[0] - 40;
			break;
		case 65265:				/* Cruise Control/Vehicle Speed */
			canbus.sp = ((RxMessage->Data[2] << 8) | RxMessage->Data[1]) / 256;	
			canbus.sp = (canbus.sp > 200) ? 0 : canbus.sp;
			break;
		case 61444:				/* Wheel Speed Information */
			temp = (RxMessage->Data[4] << 8) | RxMessage->Data[3];
			
			if(temp != 0xFFFF){
				canbus.rpm = ((RxMessage->Data[4] << 8) | RxMessage->Data[3]) / 8;
			}
			break;
		case 65271:				/* Vehicle Electrical Power – VEP */			
			temp = 10 * 5 * ((RxMessage->Data[5] << 8) | RxMessage->Data[4]);
			canbus.bat = (temp > 65000) ? canbus.bat : temp;
			break;
		case 61443:				/* Electronic Engine Controller */
			canbus.p_load = RxMessage->Data[2];
			break;
		case 65217:				/* High Resolution Vehicle Distance – VDHR */
			d_val = ((RxMessage->Data[3] << 24) | (RxMessage->Data[2] << 16) | (RxMessage->Data[1] << 8) | RxMessage->Data[0]);
			d_val = (d_val / 1000.0) * 5;
			canbus.s_km = (u32)d_val;
			break;
		case 65266:
			canbus.econ_fuel = (double)((RxMessage->Data[3] << 8) | RxMessage->Data[2]) / 512.0;			
			canbus.avg_econ_fuel = (double)((RxMessage->Data[5] << 8) | RxMessage->Data[4]) / 512.0;		
			if(canbus.avg_econ_fuel == 128){
				canbus.avg_econ_fuel = 0;
			}
			break;
		case 65276:
			canbus.p_fuel = 40 * RxMessage->Data[1];			/* 100 * p_fuel */
			break;		
		/*************************************************************************************/
		case 65248:				/* Total Vehicle Distance (On request) */
			d_val = ((RxMessage->Data[7] << 24) | (RxMessage->Data[6] << 16) | (RxMessage->Data[5] << 8) | RxMessage->Data[4]);
			d_val = d_val / 8.0;
			canbus.s_km = (u32)d_val;
			break;
		case 65253:				/* Total Engine Hours (On request) */
			canbus.s_hour = ((RxMessage->Data[3] << 24) | (RxMessage->Data[2] << 16) | (RxMessage->Data[1] << 8) | RxMessage->Data[0]) / 20;		
			break;
		case 65257:				/* Fuel Consumption (On request) */
			canbus.s_fuel = ((RxMessage->Data[7] << 24) | (RxMessage->Data[6] << 16) | (RxMessage->Data[5] << 8) | RxMessage->Data[4]) / 2;
			break;
		default:
			break;
	}
}

/*******************************************************************************
* Function Name	:	J1939_Request_PGN
* Description	:	J1939 request PGN
*******************************************************************************/
static uint8_t J1939_Request_PGN(uint32_t s_png, uint32_t d_pgn){	
	/* Config type ID extends */
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.ExtId = (0x18000000 | (s_png << 8) | J1939_SRC_ADRR);
	
	/* Config data */
	TxMessage.DLC = 3;
	TxMessage.Data[0] = (d_pgn & 0xFF);
	TxMessage.Data[1] = ((d_pgn >> 8) & 0xFF);
	TxMessage.Data[2] = ((d_pgn >> 16) & 0xFF);
	
	/* Send to canbus CAN_NO_MB */
	if(CAN_NO_MB == CAN_Transmit(CAN1, &TxMessage)){
		return 0;
	}else{
		return 1;
	}	
}

uint8_t findFaultCode(FaultCodeJ1939 fc){
	uint8_t idx ;
	uint8_t val = 0;
	
	for(idx = 0; idx < sFaultCodeSize; idx++){
		if((sFaultCodeJ1939[idx].SPN == fc.SPN) && (sFaultCodeJ1939[idx].FMI == fc.FMI) && (sFaultCodeJ1939[idx].Count == fc.Count)){
			val = 1;
			break;
		}
	}
	
	return val;
}

uint8_t FindFaultCodeODB(u8 fst, u8 sec){
	uint8_t idx ;
	uint8_t val = 0;
	
	for(idx = 0; idx < sFaultCodeSize; idx++){
		if(sFaultCodeODB[idx].first == fst && sFaultCodeODB[idx].second == sec){
			val = 1;
			break;
		}
	}
	return val;
}

uint8_t StoreFaultCode(FaultCodeJ1939 fc){
	uint8_t val = 0;
	
	if(fc.SPN == 0) return val;
	
	val = findFaultCode(fc);
	
	if(val == 1) return val;
	
//	if(sFaultCodeSize > CANBUS_MAX_FAULTCODE ) return val;
//	
	/* Set fault code */	
	sFaultCodeJ1939[sFaultCodeSize] = fc;
		
	sFaultCodeSize += 1;
	if(sFaultCodeSize >= CANBUS_MAX_FAULTCODE){
		sFaultCodeSize = 0;
	}
	
	return val;
}

uint8_t StoreFaultCodeODBII(u8 fst, u8 sec){
	char msg [128];
	uint8_t val = 0;

	if((fst == 0) || (fst == 255)) return val;
	
	if((sec == 0) || (sec == 255)) return val;
	
	val = FindFaultCodeODB(fst, sec);
	
	if(val == 1) return val;
	
	//if(sFaultCodeSize >= CANBUS_MAX_FAULTCODE) return val;	
	
	/* Set fault code */
	sFaultCodeODB[sFaultCodeSize].first = fst;
	sFaultCodeODB[sFaultCodeSize].second = sec;
	
	/* Get code char */
	memset(msg, 0, 64);
	sprintf(msg, "P%02x%02x", fst, sec);
	switch(msg[1]){
			case '0':
				msg[0] = 'P';
				msg[1] = '0';
				break;
			case '1':
				msg[0] = 'P';
				msg[1] = '1';
				break;
			case '2':
				msg[0] = 'P';
				msg[1] = '2';
				break;
			case '3':
				msg[0] = 'P';
				msg[1] = '3';
				break;
			case '4':
				msg[0] = 'C';
				msg[1] = '0';
				break;
			case '5':
				msg[0] = 'C';
				msg[1] = '1';
				break;
			case '6':
				msg[0] = 'C';
				msg[1] = '2';
				break;
			case '7':
				msg[0] = 'C';
				msg[1] = '3';
				break;
			case '8':
				msg[0] = 'B';
				msg[1] = '0';
				break;
			case '9':
				msg[0] = 'B';
				msg[1] = '1';
				break;
			case 'A':
				msg[0] = 'B';
				msg[1] = '2';
				break;
			case 'B':
				msg[0] = 'B';
				msg[1] = '3';
				break;
			case 'C':
				msg[0] = 'U';
				msg[1] = '0';
				break;
			case 'D':
				msg[0] = 'U';
				msg[1] = '1';
				break;
			case 'E':
				msg[0] = 'U';
				msg[1] = '2';
				break;
			case 'F':
				msg[0] = 'U';
				msg[1] = '3';
				break;
			default:
				break;
	}
	//SerialPushStr(msg, END_LF);
	
	sFaultCodeSize += 1;
	
	sFaultCodeSize += 1;
	if(sFaultCodeSize >= CANBUS_MAX_FAULTCODE){
		sFaultCodeSize = 0;
	}
	
	return val;	
}

static void ECU_SendFaultCode(void)
{
//	uint8_t tcp_msg[128];
//	uint8_t tcp_len;
//	uint32_t val = 0;
//	
//	tcp_len = 0;
//	memset(tcp_msg, 0, 128);
//	
//	/* Frame header */
//	tcp_msg[0] = 0xFF;
//	tcp_msg[1] = 0xFF;
//	
//	/* Device ID */
//	val = get_deviceid();
//	tcp_msg[2] = (val >> 24) & 0xFF;
//	tcp_msg[3] = (val >> 16) & 0xFF;
//	tcp_msg[4] = (val >> 8) & 0xFF;
//	tcp_msg[5] =  val & 0xFF;
//	
//	/* Version */
//	tcp_msg[6] = (PROTOCOL_VERSION << 4) | ECU_FAULT_CODE;
//	
//	/* Length data */
//	tcp_msg[7] = 0;
//	tcp_msg[8] = 0;
//	
//	if(canbus.type == CANBUS_J1939)
//	{
//		tcp_msg[9] = 2;			/* 2 Is fault code J1939 */
//		
//		tcp_msg[10] = sFaultCodeSize & 0xFF;
//		
//		for(int idx = 0; idx < sFaultCodeSize; idx++)
//		{
//			tcp_msg[11 + idx * 6 + 0] = (sFaultCodeJ1939[idx].SPN >> 24) & 0xFF;
//			tcp_msg[11 + idx * 6 + 1] = (sFaultCodeJ1939[idx].SPN >> 16) & 0xFF;
//			tcp_msg[11 + idx * 6 + 2] = (sFaultCodeJ1939[idx].SPN >> 8) & 0xFF;
//			tcp_msg[11 + idx * 6 + 3] = (sFaultCodeJ1939[idx].SPN & 0xFF);
//			tcp_msg[11 + idx * 6 + 4] = (sFaultCodeJ1939[idx].FMI & 0xFF);
//			tcp_msg[11 + idx * 6 + 5] = (sFaultCodeJ1939[idx].Count & 0xFF);
//		}

//		tcp_len = 11 + 6 * sFaultCodeSize;
//	}
//	
//	else if(canbus.type == CANBUS_ODBII)
//	{
//		tcp_msg[9] = 1;    /* 1 Is fault code OBDII */
//		
//		tcp_msg[10] = sFaultCodeSize & 0xFF;
//		
//		for(int idx = 0; idx < sFaultCodeSize; idx++)
//		{		
//			tcp_msg[11 + idx * 2 + 0] = sFaultCodeODB[idx].first & 0xFF;
//			tcp_msg[11 + idx * 2 + 1] = sFaultCodeODB[idx].second & 0xFF;
//		}

//		tcp_len = 11 + 2 * sFaultCodeSize;
//	}
//	
//	/* Length data */
//	tcp_msg[7] = tcp_len >> 8;
//	tcp_msg[8] = tcp_len & 0xFF;
//	
//	PushTCPData(tcp_msg, tcp_len);
}



/*******************************************************************************
* Function Name	:	CAN_Config
* Description	:	CAN1 init
*******************************************************************************/
static void CAN_Config(int baud){
  GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
  CAN_InitTypeDef        CAN_InitStructure;
  CAN_FilterInitTypeDef  CAN_FilterInitStructure;
    
  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(CAN_GPIO_CLK, ENABLE);
  /* Enable CAN clock */
  RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
	  /* CAN register init */
  CAN_DeInit(CAN1);
	
  /* Connect CAN pins to AF7 */
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
  GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT); 
  
  /* Configure CAN RX and TX pins */
  GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN | CAN_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	if(baud == 250){
		CAN_InitStructure.CAN_Prescaler = 8;
	}else if (baud == 500){
		CAN_InitStructure.CAN_Prescaler = 4;
	}

	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;  			/* CAN Baudrate = 1MBps (CAN clocked at 36 MHz) */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_Init(CANx, &CAN_InitStructure);
	
	/* CAN filter init */
	CAN_FilterInitStructure.CAN_FilterNumber			= 0;
	CAN_FilterInitStructure.CAN_FilterMode				= CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale				= CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh			= 0x0000;//(((u32)ODBII_PID_REQUEST << 21) & 0xFFFF0000) >> 16;
	CAN_FilterInitStructure.CAN_FilterIdLow				= 0x0000;//(((u32)ODBII_PID_REQUEST << 21) | CAN_ID_STD | CAN_RTR_DATA) & 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh		= 0x0000; //0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow			= 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment	= 0;
	CAN_FilterInitStructure.CAN_FilterActivation		= ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);	
	  
  /* Enable FIFO 0 message pending Interrupt */
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
	
	/* NVIC configuration *******************************************************/
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0xFF;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}

void SetECUClearData(){
	u8 idx = 0;
	
	/* Clear fault code J1939 */
	for(idx = 0; idx < sFaultCodeSize; idx++){
		sFaultCodeJ1939[idx].SPN = 0;
		sFaultCodeJ1939[idx].FMI = 0;
		sFaultCodeJ1939[idx].Count = 0;
	}			
	sFaultCodeSize = 0;
		
	/* Reset value */
	canbus.avg_econ_fuel = 0;
	canbus.bat = 0;
	canbus.econ_fuel = 0;
	canbus.p_fuel = 0;
	canbus.p_load = 0;
	canbus.rpm = 0;
	canbus.sp = 0;
	canbus.s_fuel = 0;
	canbus.p_load = 0;
	canbus.s_km = 0;
	canbus.sp = 0;
	canbus.s_fuel = 0;
	canbus.s_hour = 0;
	canbus.p_load = 0;
	canbus.temp = 0;
	canbus.connected = 0;
	s_fc_send = 0;
}

static u8 SendRequest(u8 mode, u8 pid_code){
	TxMessage.StdId		= ODBII_PID_REQUEST;
	TxMessage.ExtId		= 0;
	TxMessage.RTR		= CAN_RTR_DATA;
	TxMessage.IDE		= CAN_ID_STD;
	TxMessage.DLC		= 8;
	
	/* Set length request */
	if(pid_code != 0) 
		TxMessage.Data[0] = 0x02;
	else 
		TxMessage.Data[0] = 1;	
	TxMessage.Data[1]	= mode;
	TxMessage.Data[2]	= pid_code;
	TxMessage.Data[3]	= 0;
	TxMessage.Data[4]	= 0;
	TxMessage.Data[5]	= 0;
	TxMessage.Data[6]	= 0;
	TxMessage.Data[7]	= 0;
	
	return CAN_Transmit(CANx, &TxMessage);
}


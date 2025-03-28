#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "cmsis_os.h"
#include "serial.h"
#include "utils.h"				  
#include "canbus.h"
#include "j1939.h"
#include "interface.h"

#define CANBUS_ODBII						10
#define CANBUS_J1939						11
#define CANBUS_MAX_FAULTCODE		16

#define ODBII_PID_REQUEST					0x7DF
#define ODBII_PID_REQUEST_END			0x7EF
#define ODBII_PID_REPLY						0x7E8
#define ODBII_PID_REQUEST2				0x7E0

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

can_trasnmit_message_struct TxMessage;

can_receive_message_struct RxMessage;

ODBDef canbus;

uint32_t val = 0;

static void ECU_SendFaultCode(void);
int32_t time_irq ;
/* fix tempory */
uint32_t PGN = 0, SPN = 0;  							// 18bit
uint8_t nSrc = 0, FMI = 0, Count = 0;		// 8bit
uint8_t nPriority = 0, idx;							// 3bit

double d_val = 0;

uint8_t iLampStatus, sLampStatus, wLampStatus, pLampStatus;

FaultCodeJ1939 sFaultCodeJ1939[CANBUS_MAX_FAULTCODE];

FaulCodeODBII sFaultCodeODB[CANBUS_MAX_FAULTCODE];

uint8_t sFaultCodeSize = 0;

FaultCodeJ1939 fc;

uint8_t s_fc_send = 0;

uint16_t idx_dtc = 0;

uint8_t flg_dtc = 0;

uint8_t flg_has = 0;

char tmp_src[128];

static void CAN_Config(int baud);

static void ODBIIHandler(can_receive_message_struct* RxMessage);

static void J1939Handler(can_receive_message_struct* RxMessage);

static uint8_t J1939_Request_PGN(uint32_t s_png, uint32_t d_pgn);

static uint8_t SendRequest(uint8_t mode, uint8_t pid_code) ;

/* Thread J1939 task parser */
osThreadId vJ1939ParserThread;
osThreadDef(vJ1939Task, osPriorityNormal, 1, 0);

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
	uint16_t StepResq = 0;
	
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
			
		if(mcu_config.TypeOBD == OBD_NONE){
			can_deinit(CAN0);
			
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
		
//		sprintf(tmp_src ,"RPM: %d, SP: %d, TEMP: %d, FUEL: %u, HOUR: %u, KM: %u, BAT: %u", canbus.rpm, canbus.sp, canbus.temp, canbus.s_fuel, canbus.s_hour, canbus.s_km, canbus.bat);
//		SerialPushStr(tmp_src, END_LF);

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
		}
		
		if(StepResq > 17){
			StepResq = 0;
		}
		
		/* Increment step */
		StepResq += 1;
	}
}

void CAN0_RX1_IRQHandler(void){
	can_message_receive(CAN0, CAN_FIFO1, &RxMessage);

	if(RxMessage.rx_ff == CAN_FF_STANDARD){					/* ODB-II */
		ODBIIHandler(&RxMessage);	
	}else if(RxMessage.rx_ff == CAN_FF_EXTENDED){		/* J1939 */
		J1939Handler(&RxMessage);
	}
	
	can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
}

static uint8_t SendRequest_continuos(void)
{
	TxMessage.tx_sfid		= ODBII_PID_REQUEST2;
	TxMessage.tx_efid		= 0;
	TxMessage.tx_ft		= CAN_FT_DATA;
	TxMessage.tx_ff		= CAN_FF_STANDARD;
	TxMessage.tx_dlen		= 8;
	
	/* Set length request */
	TxMessage.tx_data[0] = 0x30;
	TxMessage.tx_data[1]	= 0;
	TxMessage.tx_data[2]	= 0;
	TxMessage.tx_data[3]	= 0;
	TxMessage.tx_data[4]	= 0;
	TxMessage.tx_data[5]	= 0;
	TxMessage.tx_data[6]	= 0;
	TxMessage.tx_data[7]	= 0;
	
	return can_message_transmit(CAN0, &TxMessage);
}

static void ODBIIHandler(can_receive_message_struct* RxMessage){
	if((RxMessage->rx_sfid >= ODBII_PID_REQUEST) && (RxMessage->rx_sfid <= ODBII_PID_REQUEST_END)){
		canbus.connected = 1;
		
		/* Read fault code */
		if((RxMessage->rx_data[1] == 0x43) || (RxMessage->rx_data[1] == 0x47)){
			if(RxMessage->rx_data[2] == 0){
				return;
			}else{
				idx = 3;
				if(RxMessage->rx_data[2] > 0x02){
					flg_dtc = 1;
					idx_dtc = 0;
					flg_has = RxMessage->rx_data[2] - 2;
				}
			}
			while(idx < RxMessage->rx_data[0]){
				/* Store fault code */
				StoreFaultCodeODBII(RxMessage->rx_data[idx], RxMessage->rx_data[idx + 1]);
				
				/* Imcream indexs */
				idx += 2;
			}
		}
		if(flg_dtc == 1){
			idx = 0;
			if(idx_dtc < flg_has /4 + 1){
				while(idx < (flg_has % 8)){
					StoreFaultCodeODBII(RxMessage->rx_data[idx], RxMessage->rx_data[idx + 1]);
					/* Imcream indexs */
					idx += 2;
//					sprintf(tmp_src ,"DTC: %02x %02x %02x %02x %02x %02x %02x %02x : %d", RxMessage->Data[0], RxMessage->Data[1], RxMessage->Data[2], RxMessage->Data[3], RxMessage->Data[4], RxMessage->Data[5], RxMessage->Data[6], RxMessage->Data[7], flg_has);
//					SerialPushStr(tmp_src, END_LF);
				}
			}
		}
		idx_dtc ++;
		
		if(RxMessage->rx_data[0] == 0x21){
			memcpy(canbus.VIN_ID + 3, RxMessage->rx_data + 1, 7);
		}else if(RxMessage->rx_data[0] == 0x22){
			memcpy(canbus.VIN_ID + 10, RxMessage->rx_data + 1, 7);
		}
	
		if(RxMessage->rx_data[1] == 0x41){
			/* Get information ECU */
			switch(RxMessage->rx_data[2]){
				case OBD_ENGINE_COOLANT:
					canbus.temp = (RxMessage->rx_data[3]) - 40;
					break;
				case OBD_ENGINE_RPM:
					canbus.rpm = (256 * RxMessage->rx_data[3] + RxMessage->rx_data[4]) / 4;
					break;
				case OBD_VEHICLE_SPEED:
					canbus.sp = RxMessage->rx_data[3];
					break;
				case OBD_FUELLEVEL:
					canbus.p_fuel = ((double)(RxMessage->rx_data[3])) * 100.0 / 255.0;
					break;
				case OBD_TOTAL_KM:
					canbus.s_km = (256 * RxMessage->rx_data[3] + RxMessage->rx_data[4]);
					break;
				case OBD_ODOMETER:
				canbus.odometer = bytes_to_u32(RxMessage->rx_data + 3)/10;
				break;
				case OBD_BATTERY:
					canbus.bat = ((double)RxMessage->rx_data[3] * 256.0 + RxMessage->rx_data[4]);
					break;
				case OBD_MAF_SENSOR:
				case OBD_THROTTLE:
				case OBD_O2_VOLTAGE:
				case OBD_DTC:		
					break;
				default:
					break;
			}
	  }else if(RxMessage->rx_data[1] == 0x49){
			if(RxMessage->rx_data[2] == OBD_VIN_RECEIVE){
				canbus.VIN_ID[0] = RxMessage->rx_data[5];
				canbus.VIN_ID[1] = RxMessage->rx_data[6];
				canbus.VIN_ID[2] = RxMessage->rx_data[7];
				SendRequest_continuos();
			}
		}
	}
}

/*******************************************************************************
* Function Name	:	J1939Handler
* Description	:	J1939 parser
*******************************************************************************/
static void J1939Handler(can_receive_message_struct* RxMessage){	
	uint32_t temp = 0;
	canbus.connected = 1;
	
	/* Get 8 bit source address */
	nSrc = (RxMessage->rx_efid & 0xFF);
	
	/* Get 18 bit parameter group number PGN */
	PGN = ((RxMessage->rx_efid >> 8) & 0x3FFFF);
	
	/* Get 3 bit priority */
	nPriority = ((RxMessage->rx_efid >> 24) & 0x03);
	
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
			iLampStatus =	(RxMessage->rx_data[0] >> 6) & 0x03;
		
			sLampStatus =	(RxMessage->rx_data[0] >> 4) & 0x03;
		
			wLampStatus =	(RxMessage->rx_data[0] >> 2) & 0x03;
		
			pLampStatus =	RxMessage->rx_data[0] & 0x03;
			
			/*  Parser DTC */
			//if(RxMessage->DLC > 2 && (RxMessage->DLC - 2) % 4 == 0)
			if(RxMessage->rx_dlen > 2){
				uint8_t nDTC = (RxMessage->rx_dlen - 2) / 4;
				uint8_t idx = 0, pos = 0;
				
				/* Set max size */
				nDTC = (nDTC > 16) ? 16 : nDTC;
								
				/* Get DTCs */
				for(idx = 0; idx < nDTC; idx++){
					pos = 4 * idx + 2;
					
					SPN = ((RxMessage->rx_data[pos + 2] >> 5) << 16) | (RxMessage->rx_data[pos + 1] << 8) | RxMessage->rx_data[pos];
					
					FMI = RxMessage->rx_data[pos + 2] & 0x1F;
					
					Count = RxMessage->rx_data[pos + 3] & 0x7F;
					
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
			canbus.temp = RxMessage->rx_data[0] - 40;
			break;
		case 65265:				/* Cruise Control/Vehicle Speed */
			canbus.sp = ((RxMessage->rx_data[2] << 8) | RxMessage->rx_data[1]) / 256;	
			canbus.sp = (canbus.sp > 200) ? 0 : canbus.sp;
			break;
		case 61444:				/* Wheel Speed Information */
			val = (RxMessage->rx_data[4] << 8) | RxMessage->rx_data[3];
			
			if(val != 0xFFFF){
				canbus.rpm = ((RxMessage->rx_data[4] << 8) | RxMessage->rx_data[3]) / 8;
			}
			break;
		case 65271:				/* Vehicle Electrical Power – VEP */			
			val = 10 * 5 * ((RxMessage->rx_data[5] << 8) | RxMessage->rx_data[4]);
			canbus.bat = (val > 65000) ? canbus.bat : val;
			break;
		case 61443:				/* Electronic Engine Controller */
			canbus.p_load = RxMessage->rx_data[2];
			break;
		case 65217:				/* High Resolution Vehicle Distance – VDHR */
			d_val = ((RxMessage->rx_data[3] << 24) | (RxMessage->rx_data[2] << 16) | (RxMessage->rx_data[1] << 8) | RxMessage->rx_data[0]);
			d_val = (d_val / 1000.0) * 5;
			canbus.s_km = (uint32_t)d_val;
			break;
		case 65266:
			canbus.econ_fuel = (double)((RxMessage->rx_data[3] << 8) | RxMessage->rx_data[2]) / 512.0;			
			canbus.avg_econ_fuel = (double)((RxMessage->rx_data[5] << 8) | RxMessage->rx_data[4]) / 512.0;		
			if(canbus.avg_econ_fuel == 128){
				canbus.avg_econ_fuel = 0;
			}
			break;
		case 65276:
			canbus.p_fuel = 40 * RxMessage->rx_data[1];			/* 100 * p_fuel */
			break;		
		/*************************************************************************************/
		case 65248:				/* Total Vehicle Distance (On request) */
			d_val = ((RxMessage->rx_data[7] << 24) | (RxMessage->rx_data[6] << 16) | (RxMessage->rx_data[5] << 8) | RxMessage->rx_data[4]);
			d_val = d_val / 8.0;
			canbus.s_km = (uint32_t)d_val;
			break;
		case 65253:				/* Total Engine Hours (On request) */
			canbus.s_hour = ((RxMessage->rx_data[3] << 24) | (RxMessage->rx_data[2] << 16) | (RxMessage->rx_data[1] << 8) | RxMessage->rx_data[0]) / 20;		
			break;
		case 65257:				/* Fuel Consumption (On request) */
			canbus.s_fuel = ((RxMessage->rx_data[7] << 24) | (RxMessage->rx_data[6] << 16) | (RxMessage->rx_data[5] << 8) | RxMessage->rx_data[4]) / 2;
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
	TxMessage.tx_ff = CAN_FF_EXTENDED;//ok
	TxMessage.tx_efid = (0x18000000 | (s_png << 8) | J1939_SRC_ADRR);//ok
	
	/* Config data */
	TxMessage.tx_dlen = 3;//ok
	TxMessage.tx_data[0] = (d_pgn & 0xFF);//ok
	TxMessage.tx_data[1] = ((d_pgn >> 8) & 0xFF);//ok
	TxMessage.tx_data[2] = ((d_pgn >> 16) & 0xFF);//ok
	
	/* Send to canbus CAN_NO_MB */
	if(CAN_TRANSMIT_NOMAILBOX == can_message_transmit(CAN0, &TxMessage)){
		return 0;
	}else{
		return 1;
	}
}
uint8_t findFaultCode(FaultCodeJ1939 fc){
	uint8_t val = 0;
	uint8_t idx = 0;
	
	for(idx = 0; idx < sFaultCodeSize; idx++){
		if((sFaultCodeJ1939[idx].SPN == fc.SPN) && (sFaultCodeJ1939[idx].FMI == fc.FMI) && (sFaultCodeJ1939[idx].Count == fc.Count)){
			val = 1;
			break;
		}
	}
	
	return val;
}

uint8_t FindFaultCodeODB(uint8_t fst, uint8_t sec){
	uint8_t val = 0;
	uint8_t idx = 0;
	
	for(idx = 0; idx < sFaultCodeSize; idx++){
		if((sFaultCodeODB[idx].first == fst) && (sFaultCodeODB[idx].second == sec)){
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
	
	//if(sFaultCodeSize > CANBUS_MAX_FAULTCODE ) return val;
	
	/* Set fault code */
	sFaultCodeJ1939[sFaultCodeSize] = fc;
		
	sFaultCodeSize += 1;
	
	if(sFaultCodeSize >= CANBUS_MAX_FAULTCODE){
		sFaultCodeSize = 0;
	}
	
	return val;
}

uint8_t StoreFaultCodeODBII(uint8_t fst, uint8_t sec){
	char msg [128];
	uint8_t val = 0;

	if((fst == 0) || (fst == 255)) return val;
	
	if((sec == 0) || (sec == 255)) return val;
	
	val = FindFaultCodeODB(fst, sec);
	
	if(val == 1) return val;
	
//	if(sFaultCodeSize >= CANBUS_MAX_FAULTCODE) return val;	
	
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
	sFaultCodeSize += 1;
	
	sFaultCodeSize += 1;
	if(sFaultCodeSize >= CANBUS_MAX_FAULTCODE){
		sFaultCodeSize = 0;
	}
	
	return val;
}

static void ECU_SendFaultCode(void){

}

/*******************************************************************************
* Function Name	:	CAN_Config
* Description	:	CAN0 init
*******************************************************************************/
static void CAN_Config(int baud){
	can_parameter_struct  can_parameter;
	can_filter_parameter_struct can_filter;
	
	/* enable CAN clock */
	rcu_periph_clock_enable(RCU_CAN0);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_AF);

	/* configure CAN0 GPIO */
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
	
	/*---------------------------------------------------------------*/
	 nvic_irq_enable(CAN0_RX1_IRQn,0,0);
	/*---------------------------------------------------------------*/

	/* initialize CAN register */
	can_deinit(CAN0);

	/* initialize CAN */
	can_parameter.time_triggered = DISABLE;
	can_parameter.auto_bus_off_recovery = DISABLE;
	can_parameter.auto_wake_up = DISABLE;
	can_parameter.auto_retrans = DISABLE;
	can_parameter.rec_fifo_overwrite = DISABLE;
	can_parameter.trans_fifo_order = DISABLE;
	can_parameter.working_mode = CAN_NORMAL_MODE;
	can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
	can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
	can_parameter.time_segment_2 = CAN_BT_BS2_4TQ;
	/* baudrate 1Mbps */
	if(baud == 500){
		can_parameter.prescaler = 12;//ok
	}else if(baud == 250){
		can_parameter.prescaler = 24;//ok
	}
	can_init(CAN0, &can_parameter);

	/* initialize filter */
	/* CAN0 filter number */
	can_filter.filter_number = 0;

	/* initialize filter */    
	can_filter.filter_mode = CAN_FILTERMODE_MASK;
	can_filter.filter_bits = CAN_FILTERBITS_32BIT;
	can_filter.filter_list_high = 0x0000;
	can_filter.filter_list_low = 0x0000;
	can_filter.filter_mask_high = 0x0000;
	can_filter.filter_mask_low = 0x0000;  
	can_filter.filter_fifo_number = CAN_FIFO1;
	can_filter.filter_enable = ENABLE;
	can_filter_init(&can_filter);
	
	/* enable CAN receive FIFO1 not empty interrupt */
  can_interrupt_enable(CAN0, CAN_INT_RFNE1);
}

void SetECUClearData(){
	uint8_t idx = 0;
	
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

static uint8_t SendRequest(uint8_t mode, uint8_t pid_code){
	TxMessage.tx_sfid		= ODBII_PID_REQUEST;
	TxMessage.tx_efid		= 0;
	TxMessage.tx_ft		= CAN_FT_DATA;
	TxMessage.tx_ff		= CAN_FF_STANDARD;
	TxMessage.tx_dlen		= 8;
	
	/* Set length request */
	if(pid_code != 0) 
		TxMessage.tx_data[0] = 0x02;
	else 
		TxMessage.tx_data[0] = 1;	
	TxMessage.tx_data[1]	= mode;
	TxMessage.tx_data[2]	= pid_code;
	TxMessage.tx_data[3]	= 0;
	TxMessage.tx_data[4]	= 0;
	TxMessage.tx_data[5]	= 0;
	TxMessage.tx_data[6]	= 0;
	TxMessage.tx_data[7]	= 0;
	
	return can_message_transmit(CAN0, &TxMessage);
}

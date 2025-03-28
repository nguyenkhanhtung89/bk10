#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "cmsis_os.h"
#include "serial.h"
#include "utils.h"				  
#include "canbus.h"
#include "interface.h"
#include "power.h"

#define VFE34_PID_REQUEST				0x693
#define VF8_PID_REQUEST					0x684

can_trasnmit_message_struct TxMessage;

can_receive_message_struct RxMessage;

ODBDef canbus;

static uint8_t VF_Send_Request(u32 CanId, u8 service, u8 pid, u8 did);

static void CAN_Config(int baud);

static void ODBIIHandler(can_receive_message_struct* RxMessage);

/* Thread J1939 task parser */
osThreadId vOBDIITransmitThread;
osThreadDef(vOBDIITransmit, osPriorityNormal, 1, 0);

uint8_t CAN_service_init(void){
	vOBDIITransmitThread = osThreadCreate(osThread(vOBDIITransmit), NULL);
	if(!vOBDIITransmitThread){
		return 0;
	}
	return 1;
}

u8 VIN_NUMBER[32];
u8 flg_rcv_vin_number = 0;
u16 time_disable_obd = 0;
u8 flg_check_update_vin = 0;
u8 flg_rcv_vin_part = 0;

u8 flg_rcv_speed_part = 0;

void vOBDIITransmit(const void* args){
	static uint16_t StepResq = 0;
	
	CAN_Config(500);
	
	/* Task manager */
	for(;;){
		osDelay(500);

		switch(StepResq){
			case 0:
				break;
			case 1:{
				flg_rcv_speed_part = 0;
				if(flg_rcv_vin_number == 0){
					VF_Send_Request(0x681, 0x22, 0xF1, 0x90);//GET VIN PART 1
				}
				break;
			}
			case 2:
				VF_Send_Request(0x693, 0x22, 0xF1, 0x21);//ODO
				break;
			case 3:
				VF_Send_Request(0x683, 0x22, 0xF1, 0x24);//ODO new
				break;
			case 4:
				VF_Send_Request(0x693, 0x22, 0xF0, 0x26);//SOC
				break;
			case 5:
				VF_Send_Request(0x693, 0x22, 0xF0, 0x27);//SOH
				break;
			case 6:
				VF_Send_Request(0x6C4, 0x22, 0xFA, 0xC1);//ghe phu truoc
				break;
			case 7:
				VF_Send_Request(0x688, 0x22, 0x33, 0x36);//ghe sau trai
				break;
			case 8:
				VF_Send_Request(0x688, 0x22, 0x33, 0x37);//ghe sau phai
				break;
			case 9:
				VF_Send_Request(0x688, 0x22, 0x33, 0x3B);//ghe sau giua
				break;
			case 10:
				if(canbus.isOBD_VF8 == 1){//VF8
					VF_Send_Request(0x6A9, 0x22, 0xDD, 0x09);//toc do
				}else{//VF5/VFe34
					VF_Send_Request(0x6A9, 0x22, 0x30, 0x07);//toc do
				}
				break;
			default:
				StepResq = 0;
				break;
		}

		/* Increment step */
		StepResq += 1;
		osThreadYield();
	}
}

static uint8_t VF_Send_Request_VIN(void){
	TxMessage.tx_sfid		= 0x681;
	TxMessage.tx_efid		= 0;
	TxMessage.tx_ft			= CAN_FT_DATA;
	TxMessage.tx_ff			= CAN_FF_STANDARD;
	TxMessage.tx_dlen		= 8;
	
	/* Set length request */
	TxMessage.tx_data[0] 	= 0x30;
	TxMessage.tx_data[1]	= 0x08;
	TxMessage.tx_data[2]	= 0xF1;
	TxMessage.tx_data[3]	= 0;
	TxMessage.tx_data[4]	= 0;
	TxMessage.tx_data[5]	= 0;
	TxMessage.tx_data[6]	= 0;
	TxMessage.tx_data[7]	= 0;
	
	return can_message_transmit(CAN0, &TxMessage);
}

static uint8_t VF_Send_Request_Speed(void){
	TxMessage.tx_sfid		= 0x6A9;
	TxMessage.tx_efid		= 0;
	TxMessage.tx_ft			= CAN_FT_DATA;
	TxMessage.tx_ff			= CAN_FF_STANDARD;
	TxMessage.tx_dlen		= 8;
	
	/* Set length request */
	TxMessage.tx_data[0] 	= 0x30;
	TxMessage.tx_data[1]	= 0x00;
	TxMessage.tx_data[2]	= 0x01;
	TxMessage.tx_data[3]	= 0;
	TxMessage.tx_data[4]	= 0;
	TxMessage.tx_data[5]	= 0;
	TxMessage.tx_data[6]	= 0;
	TxMessage.tx_data[7]	= 0;
	
	return can_message_transmit(CAN0, &TxMessage);
}

void CAN0_RX1_IRQHandler(void){
	can_message_receive(CAN0, CAN_FIFO1, &RxMessage);
	
	if(RxMessage.rx_ff == CAN_FF_STANDARD){					/* ODB-II */
		ODBIIHandler(&RxMessage);	
	}
	
	can_interrupt_flag_clear(CAN0, CAN_INT_FLAG_RFF0);
}

//05 62 F0 27 00 64 00 00   SOH
//05 62 F0 26 00 24 00 00   SOC
//03 7F 22 31 00 00 00 00   speed
//06 62 F1 21 00 3A DF 00   ODO
static void ODBIIHandler(can_receive_message_struct* RxMessage){
	canbus.timeout = 0;
	canbus.connected = 1;
	
	if(flg_rcv_vin_part == 1){
		if(RxMessage->rx_data[0] == 0x21){//GET VIN PART 2 
			memcpy(VIN_NUMBER + 3, RxMessage->rx_data + 1, 7); 
		}else if(RxMessage->rx_data[0] == 0x22){//GET VIN PART 3
			memcpy(VIN_NUMBER + 10, RxMessage->rx_data + 1, 7); 
			flg_rcv_vin_part = 0;//finished get vin number
			flg_rcv_vin_number = 1;//finished get vin number
			flg_check_update_vin = 1;//check change value and write new value to flash
		}
	}
	
	if((RxMessage->rx_data[3] == 0xF1) && (RxMessage->rx_data[4] == 0x90)){//10 14 62 F1 90   GET VIN PART 1 ex RLL
		VIN_NUMBER[0] = RxMessage->rx_data[5];
		VIN_NUMBER[1] = RxMessage->rx_data[6];
		VIN_NUMBER[2] = RxMessage->rx_data[7];
		flg_rcv_vin_part = 1;
		VF_Send_Request_VIN();
	}else if((RxMessage->rx_data[2] == 0xF1) && (RxMessage->rx_data[3] == 0x21)){//ODO old
		dnumber_uni dnum;
		dnum.byte[0] = RxMessage->rx_data[6];
		dnum.byte[1] = RxMessage->rx_data[5];
		dnum.byte[2] = RxMessage->rx_data[4];
		canbus.s_km = dnum.num;
		canbus.timeout = 0;
		canbus.connected = 1;
	}else if((RxMessage->rx_data[2] == 0xF1) && (RxMessage->rx_data[3] == 0x24)){//ODO new
		dnumber_uni dnum;
		dnum.byte[0] = RxMessage->rx_data[6];
		dnum.byte[1] = RxMessage->rx_data[5];
		dnum.byte[2] = RxMessage->rx_data[4];
		
		canbus.s_100m = dnum.num;
		canbus.timeout = 0;
		canbus.connected = 1;
	}else if((RxMessage->rx_data[2] == 0xF0) && (RxMessage->rx_data[3] == 0x26)){//SOC
		canbus.s_fuel = RxMessage->rx_data[5];//ok
	}else if((RxMessage->rx_data[2] == 0xF0) && (RxMessage->rx_data[3] == 0x27)){//SOH
		canbus.p_load = RxMessage->rx_data[5];//ok
	}else if((RxMessage->rx_data[2] == 0xFA) && (RxMessage->rx_data[3] == 0xC1)){//truoc phu
		canbus.seat_value = RxMessage->rx_data[5];
		if(RxMessage->rx_data[5] > mcu_config.seat_level){//0x18
			canbus.front_passenger_seat = 1;
		}else{
			canbus.front_passenger_seat = 0;
		}
		canbus.timeout = 0;
		canbus.connected = 1;
		mcu_config.TypeOBD = VF8_TYPE;
	}else if((RxMessage->rx_data[2] == 0x33) && (RxMessage->rx_data[3] == 0x36)){//sau lai
		canbus.isOBD_VF8 = 1;
		if(RxMessage->rx_data[4] == 0x12){
			canbus.left_rear_passenger_seat = 1;
		}else{
			canbus.left_rear_passenger_seat = 0;
		}
	}else if((RxMessage->rx_data[2] == 0x33) && (RxMessage->rx_data[3] == 0x37)){//sau phu
		if(RxMessage->rx_data[4] == 0x12){
			canbus.right_rear_passenger_seat = 1;
		}else{
			canbus.right_rear_passenger_seat = 0;
		}
	}else if((RxMessage->rx_data[2] == 0x33) && (RxMessage->rx_data[3] == 0x3B)){//sau giua
		if(RxMessage->rx_data[4] == 0x20){
			canbus.mid_rear_passenger_seat = 1;
		}else{
			canbus.mid_rear_passenger_seat = 0;
		}
	}else if((RxMessage->rx_data[2] == 0xDD) && (RxMessage->rx_data[3] == 0x09)){//toc do VF8
		canbus.speedOBD = RxMessage->rx_data[4];
		//memcpy(buffer_speed, RxMessage->rx_data, 8);
	}
	
	if(RxMessage->rx_sfid == 0x629){//done speed VF5 and VFe34
		if((RxMessage->rx_data[3] == 0x30) && (RxMessage->rx_data[4] == 0x07)){
			VF_Send_Request_Speed();
			flg_rcv_speed_part = 1;
		}
		
		if((RxMessage->rx_data[0] == 0x21) && (flg_rcv_speed_part == 1)){
			u16 obd_speed = 0;
			float fnum = 0;
			
			obd_speed = (RxMessage->rx_data[6] << 8 | (RxMessage->rx_data[5]));
			
			fnum = ((float)(obd_speed) * (float)(0.015625));
			canbus.speedOBD = (u8)fnum;
			flg_rcv_speed_part = 0;
		}
	}
	
	if(canbus.isOBD_VF8 == 1){
		sensorIR.status_seat = canbus.front_passenger_seat | canbus.left_rear_passenger_seat | canbus.mid_rear_passenger_seat | canbus.right_rear_passenger_seat;
	}else{
		sensorIR.status_seat = 0;
	}
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

static uint8_t VF_Send_Request(u32 CanId, u8 service, u8 pid, u8 did){
	TxMessage.tx_sfid		= CanId;
	TxMessage.tx_efid		= 0;
	TxMessage.tx_ft			= CAN_FT_DATA;
	TxMessage.tx_ff			= CAN_FF_STANDARD;
	TxMessage.tx_dlen		= 8;
	
	/* Set length request */
	TxMessage.tx_data[0] 	= 0x03;
	TxMessage.tx_data[1]	= service;
	TxMessage.tx_data[2]	= pid;
	TxMessage.tx_data[3]	= did;
	TxMessage.tx_data[4]	= 0;
	TxMessage.tx_data[5]	= 0;
	TxMessage.tx_data[6]	= 0;
	TxMessage.tx_data[7]	= 0;
	
	return can_message_transmit(CAN0, &TxMessage);
}

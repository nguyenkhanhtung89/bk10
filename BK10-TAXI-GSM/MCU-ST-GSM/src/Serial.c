#include "Serial.h"
#include "j1939.h"
#include "utils.h"
#include "osObjects.h"
#include "power.h"
#include "flash.h"
#include "ds18b20.h"
#include "digital.h"
#include "utils.h"

void vSerialReceiver(const void* args);

void vSerialTransfer(const void* args);
void vSerialTask(const void* args);

void SerialUARTInit(void);

void SerialPushBufferWithCS(uint8_t * data, uint16_t size);

ec21_pw_def ec21_status;

ec21_message_def ec21_data;

mcu_config_def mcu_config;

/* MsgBox to store data received from Serial port */
osMessageQDef(SerialRxMsgBox, SerialRxBuffer_SIZE, uint8_t);
osMessageQId  SerialRxMsgBox;

osMailQDef(SerialPkgMailBox, 3, Serial_Mail_Typedef);
osMailQId SerialPkgMailBox;

uint8_t SerialRxBuffer[SerialRxBuffer_SIZE];
uint16_t SerialRxBufferIdx = 0;

osThreadId serial_receiver_thread;
osThreadDef(vSerialReceiver, osPriorityNormal, 1, 0);

osThreadId serial_transfer_thread;
osThreadDef(vSerialTransfer, osPriorityNormal, 1, 0);
osThreadId serial_thread;
osThreadDef(vSerialTask, osPriorityNormal, 1, 1024);

osSemaphoreId sSerialFree;
osSemaphoreDef(sSerialFree);

void load_mcu_config(void){
//	//load baudrate 2
//	mcu_config.baud_2_value = FLASH_ReadData(BAURATE2_ADDR);
//	if (mcu_config.baud_2_value == 0) {
//		mcu_config.baud_2_value = 9600;
//   // FLASH_WriteData(BAURATE2_ADDR, mcu_config.baud_2_value, 0);
//  }
	
	mcu_config.use_release_button = (uint8_t)FLASH_ReadData(USE_RELEASE_BUTTON);
	if(mcu_config.use_release_button > 1){
		mcu_config.use_release_button = 0;
	}
	
	mcu_config.TaxiMode = (uint8_t)FLASH_ReadData(MODE_TAXI_ADDR);
	if(mcu_config.TaxiMode != 0){
		mcu_config.TaxiMode = 1;
	}
	
//	//load baudrate 4
//	mcu_config.baud_4_value = FLASH_ReadData(BAURATE4_ADDR);
//	if (mcu_config.baud_4_value == 0) {
//		mcu_config.baud_4_value = 9600;
//    //FLASH_WriteData(BAURATE4_ADDR, mcu_config.baud_4_value, 0);
//  }
	
	mcu_config.buzz_level = (u8)FLASH_ReadData(BUZZER_LEVEL_ADDR);
	//mcu_config.PulseKm = (u16)FLASH_ReadData(PULSE_KM_ADDR);
//	mcu_config.slience_mode = (u8)FLASH_ReadData(SLIENCE_MODE_ADDR);
	mcu_config.TypeOBD = (enum_OBD_type)FLASH_ReadData(OBD_TYPE_ADDR);
	mcu_config.TypePulseADC = (Fuel_type)FLASH_ReadData(TYPE_FULSE_ADC_ADDR);
}

void Serial_Init(void){
	/* Create TX Serial semaphore */
	sSerialFree = osSemaphoreCreate(osSemaphore(sSerialFree), 1);
	
	SerialRxMsgBox = osMessageCreate(osMessageQ(SerialRxMsgBox), NULL);
	
  /* Create mail queue */
  SerialPkgMailBox = osMailCreate(osMailQ(SerialPkgMailBox), NULL);
  if (SerialPkgMailBox == NULL) {
    return;
  }
	
	serial_receiver_thread = osThreadCreate(osThread(vSerialReceiver), NULL);
  if(!serial_receiver_thread) return;
	
	serial_transfer_thread = osThreadCreate(osThread(vSerialTransfer), NULL);
  if(!serial_transfer_thread) return;
	
	serial_thread = osThreadCreate(osThread(vSerialTask), NULL);
  if(!serial_thread) return;
	
	/* Init usart */
	SerialUARTInit();
}

uint8_t serial_data_rpt[64];
void serial_report_data(void){
	uint8_t crc = 0;
	uint16_t tval = 0;
	memset(serial_data_rpt, 0x0, 64);
	
	serial_data_rpt[0] = '>';
	serial_data_rpt[1] = 0x04;
	
	serial_data_rpt[2] = MCU_VERSION;
	
	serial_data_rpt[3] = (ADCData.main_power_val >> 8) & 0xFF;
	serial_data_rpt[4] = ADCData.main_power_val & 0xFF;
	
	serial_data_rpt[5] = (ADCData.battery_val >> 8) & 0xFF;
	serial_data_rpt[6] = ADCData.battery_val & 0xFF;
	
	serial_data_rpt[7] = (ADCData.adc_pulse_val >> 8) & 0xFF;
	serial_data_rpt[8] = ADCData.adc_pulse_val & 0xFF;
	
	serial_data_rpt[9] = (ADCData.v_5v6_val >> 8) & 0xFF;
	serial_data_rpt[10] = ADCData.v_5v6_val & 0xFF;
	
	if(mcu_config.TaxiMode == 1){
		serial_data_rpt[11] = (counter_ir_1 >> 8) & 0xFF; //(fuel_232_value.fuel1_val >> 8) & 0xFF;
		serial_data_rpt[12] = counter_ir_1 & 0xFF; //fuel_232_value.fuel2_val& 0xFF;
		
		serial_data_rpt[13] = (counter_ir_2 >> 8) & 0xFF; //(fuel_232_value.fuel2_val >> 8) & 0xFF;
		serial_data_rpt[14] = counter_ir_2 & 0xFF; //fuel_232_value.fuel1_val& 0xFF;
		
		serial_data_rpt[15] = sensorIR.statusHiring;
		serial_data_rpt[16] = sensorIR.statusIR1;
		serial_data_rpt[17] = sensorIR.statusIR2;
		
		if(canbus.isOBD_VF8 == 1){
			serial_data_rpt[16] = sensorIR.status_seat;
			serial_data_rpt[17] = sensorIR.status_seat;
		}
		
		serial_data_rpt[18] = mcu_config.use_release_button;
	}
	
	//Temperature
	serial_data_rpt[19] = (uint8_t)ADCData.temperature;
	
	/* Speed ECU */
	serial_data_rpt[20] = canbus.connected | (ACC_STATUS << 1) | (DOOR_STATUS << 2) | (AIR_STATUS << 3) | (PowerData.power_mode << 4);
	
	/* Speed ECU */
	serial_data_rpt[21] = mcu_config.TypeOBD;
	
	/* Speed ECU */
	serial_data_rpt[22] = canbus.sp & 0xFF;
	
	/* Temp ECU */
	serial_data_rpt[23] = canbus.temp & 0xFF;
	
	/* RPM ECU */
	serial_data_rpt[24] = (canbus.rpm >> 8) & 0xFF;
	serial_data_rpt[25] = (canbus.rpm & 0xFF);
	
	/* Fuel ECU */
	serial_data_rpt[26] = ((canbus.p_fuel) >> 8) & 0xFF;
	serial_data_rpt[27] = (canbus.p_fuel & 0xFF);
	
	/* Total km running from ECU */
	serial_data_rpt[28] = (canbus.s_km >> 24) & 0xFF;
	serial_data_rpt[29] = (canbus.s_km >> 16) & 0xFF;
	serial_data_rpt[30] = (canbus.s_km >> 8) & 0xFF;
	serial_data_rpt[31] = (canbus.s_km & 0xFF);
	
	/* Total oil from ECU */
	serial_data_rpt[32] = (canbus.s_fuel >> 24) & 0xFF;
	serial_data_rpt[33] = (canbus.s_fuel >> 16) & 0xFF;
	serial_data_rpt[34] = (canbus.s_fuel >> 8) & 0xFF;
	serial_data_rpt[35] = (canbus.s_fuel & 0xFF);
	
	/* Bat from ECU */
	serial_data_rpt[36] = (canbus.bat >> 8) & 0xFF;
	serial_data_rpt[37] = (canbus.bat & 0xFF);
	
	/* Km/Lit */
	tval = (uint16_t)(100 * canbus.avg_econ_fuel);
	serial_data_rpt[38] = (tval >> 8) & 0xFF;
	serial_data_rpt[39] = (tval & 0xFF);
	
	/* Km/Lit */
	tval = (uint16_t)(100 * canbus.econ_fuel);
	serial_data_rpt[40] = (tval >> 8) & 0xFF;
	serial_data_rpt[41] = (tval & 0xFF);
	
	/* % load */
	serial_data_rpt[42] = canbus.p_load;
	
	crc = CRC_8BitsCompute(serial_data_rpt, 43);
	
	serial_data_rpt[43] = crc;
	
	SerialPushBuffer(serial_data_rpt, 44);
}

static void SerialrxBufferPush(uint8_t data){
	SerialRxBuffer[SerialRxBufferIdx++] = data;
	if(SerialRxBufferIdx >= SerialRxBuffer_SIZE)
		SerialRxBufferIdx = 0;
}

static void SerialkgBufferPush(uint8_t * data, uint16_t size) {
  /* Copy data */
  Serial_Mail_Typedef * msg;

  msg = (Serial_Mail_Typedef*)osMailAlloc(SerialPkgMailBox, 1000);

  if (msg == NULL) return;
  
  if (size > SerialRxBuffer_SIZE) size = SerialRxBuffer_SIZE;

  msg->size = size;
  memcpy(msg->data, data, size);

  /* Put mail */
  osMailPut(SerialPkgMailBox, msg);
}

static void EC21_data_parser(u8 * msg){
//	u8 hirring_led = 0;
	/* gps status */
	ec21_data.gps_status = msg[0] & 0x01;
	/* server connected */
	ec21_data.tcp_status = (msg[0] >> 1) & 0x01;
	/* internet connected */
	ec21_data.internet_status = (msg[0] >> 2) & 0x01;
	/* login / logout status */
	ec21_data.login_status = (msg[0] >> 3) & 0x01;
	/* over speed warning status */
	ec21_data.ov_speed_status = (msg[0] >> 4) & 0x01;
	/* warning of driver continue */
	ec21_data.drv_continue_status = (msg[0] >> 5) & 0x01;

	/* sim status */
	ec21_data.sim_status = (E_QL_SIM_STATES_T)msg[1];
	
	ec21_data.TypeOBD = (enum_OBD_type)msg[2];
	ec21_data.TypePulseADC = (Fuel_type)msg[3];
	ec21_data.buzz_level = msg[4];
	
	/* setting by -> slience mode */
	ec21_data.disable_release_button = msg[5];
	if(ec21_data.disable_release_button == 1){
		mcu_config.LedMode = 0;
	}
	
	/*Mode taxi*/
	ec21_data.TaxiMode = msg[6];
	if(ec21_data.TaxiMode != mcu_config.TaxiMode){
		mcu_config.TaxiMode = ec21_data.TaxiMode;
		FLASH_WriteData(MODE_TAXI_ADDR, (uint32_t)mcu_config.TaxiMode, 0);
	}
	
	/* sync type OBD to mcu setting */
	if(mcu_config.TypeOBD != ec21_data.TypeOBD){
		mcu_config.TypeOBD = ec21_data.TypeOBD;
		FLASH_WriteData(OBD_TYPE_ADDR, (u32)mcu_config.TypeOBD, 0);
	}
	
	/* sync type Pulse or ADC to mcu setting */
	if(mcu_config.TypePulseADC != ec21_data.TypePulseADC){
		mcu_config.TypePulseADC = ec21_data.TypePulseADC;
		FLASH_WriteData(TYPE_FULSE_ADC_ADDR, (u32)mcu_config.TypePulseADC, 0);
	}
	
	/* sync buzzer level to mcu setting */
	if(mcu_config.buzz_level != ec21_data.buzz_level){
		mcu_config.buzz_level = ec21_data.buzz_level;
		FLASH_WriteData(BUZZER_LEVEL_ADDR, (u32)mcu_config.buzz_level, 0);
	}
}

uint8_t serial_buffer[512];
uint8_t flash_buffer[512];
void Serial_Parse(uint8_t* data, uint16_t Len){//     HD   PARAM
	uint8_t cmd = 0;
	u8 crc = 0;
	//process data
	if(data[0] == '>'){    // >0x01 msg
		cmd = data[1];
		switch(cmd){
			case EC21_DATA_MESSAGE://0x01
				EC21_data_parser(data + 2);
				break;
			case USART2_FORWARD_MSG://0x02
				USART2PushBuffer(data + 2, Len - 2);
				break;
			case UART4_FORWARD_MSG://0x03
				UART4PushBuffer(data + 2, Len - 2);
				break;
			case STORE_FLASH_DATA:{//0x04
				uint16_t data_len = 0;
				uint32_t crc32_flash = 0;
				uint32_t crc32_data = 0;
				
				/* compare data */
				data_len = data[2] << 8 | data[3];
				
				if(data_len != (Len - 4)){
					//SerialPushStr("~~~INVALID DATA~~~", END_LF);
					break;
				}
				
				if(data_len >= 500){
					//SerialPushStr("***INVALID DATA***", END_LF);
					break;
				}
				
				/* read old data */
				FLASH_ReadBufData(DEV_CONFIG_ADDR, serial_buffer, data_len);
				
				crc32_data = CRC32Software(data + 4, (data_len - 4));
				crc32_flash = CRC32Software(serial_buffer, (data_len - 4));
				
				if(crc32_data != crc32_flash){
					FLASH_WriteBufData(DEV_CONFIG_ADDR, data + 4, data_len);
					SerialPushStr("###STORE_FLASH_DATA###", END_LF);
				}else{
					SerialPushStr("###DATA NO CHANGE###", END_LF);
				}
				break;
			}
			case GET_FLASH_DATA:{//0x05
				u16 rlen_ = 0;
				u16 request_len = 0;
				u32 crc32 = 0;
				u32 cal_crc32 = 0;
				request_len = data[2] << 8 | data[3];
				serial_buffer[0] = '>';
				serial_buffer[1] = 11;
				rlen_ = 2;
				
				FLASH_ReadBufData(DEV_CONFIG_ADDR, flash_buffer, (request_len + 4));
				
				cal_crc32 = CRC32Software(flash_buffer, request_len);
				
				crc32 = (flash_buffer[request_len] << 24) | (flash_buffer[request_len + 1] << 16) | (flash_buffer[request_len + 2] << 8) | flash_buffer[request_len + 3];
				
				if(cal_crc32 == crc32){
					memcpy(serial_buffer + rlen_, flash_buffer, (request_len + 4));
					rlen_ = rlen_ + (request_len + 4);
					crc = CRC_8BitsCompute(serial_buffer, rlen_);
					serial_buffer[rlen_] = crc;
					rlen_ = rlen_ + 1;
					SerialPushBuffer(serial_buffer, rlen_);
					osDelay(500);
				}else{
					SerialPushStr("***INVALID CRC FROM FLASH***", END_LF);
				}
				
				break;
			}
			default:
				break;
		}
	}else{
		if (!strncmp((char*)data, "GOTO_BOOT_MODE", 14)) {
			SerialPushBufferWithCS((uint8_t*)">>OK", 4);
			//set flag update firmware on bootloader
			FLASH_WriteData(FLG_UPDATE_FW_ADDR, (u32)0x01, 1);
		} else if (!strncmp((char*)data, "VERSION?", 8)) {
			serial_buffer[0] = '>';
			serial_buffer[1] = '>';
			serial_buffer[2] = 'V';
			serial_buffer[3] = MCU_VERSION;
			SerialPushBufferWithCS(serial_buffer, 4);
		} else if (!strncmp((char*)data, "REBOOT", 6)) {
			SerialPushBufferWithCS((uint8_t*)">>OK", 4);
			ec21_power_rst(5);
		} else if (!strncmp((char*)data, "BATOFF", 6)) {
			SerialPushBufferWithCS((uint8_t*)">>OK", 4);
			device_battery_off(5);
		}
		if(mcu_config.TaxiMode == 0){
			if (!strncmp((char*)data, "CAMON", 5)) {
				SerialPushBufferWithCS((uint8_t*)">>OK", 4);
				osDelay(1000);
				GPIO_ResetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
			} else if (!strncmp((char*)data, "CAMOFF", 6)) {
				SerialPushBufferWithCS((uint8_t*)">>OK", 4);
				GPIO_SetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
			} else if (!strncmp((char*)data, "CAMRST", 6)) {
				SerialPushBufferWithCS((uint8_t*)">>OK", 4);
				GPIO_SetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
				osDelay(4000);
				GPIO_ResetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
			}
		}
	}
}

void vSerialReceiver(const void* args){
osEvent RxEvent;
	static uint8_t rxChar;
	uint32_t cmd_timeout = osWaitForever;
	
	static enum {
		SERIAL_RX_STATE_S0,
		SERIAL_RX_STATE_S1,
	} state;
	
	for(;;){
		/*RX*/
		RxEvent = osMessageGet(SerialRxMsgBox, cmd_timeout);
		
		// Process state
		switch(state){
		case SERIAL_RX_STATE_S0:
				// Check data
			if(RxEvent.status == osEventMessage){
				rxChar = (uint8_t) RxEvent.value.v;
				
				SerialrxBufferPush(rxChar);
				
				// Change state
				cmd_timeout = 10;
				state = SERIAL_RX_STATE_S1;
			}
			break;
		case SERIAL_RX_STATE_S1:
			// Data received
			if(RxEvent.status == osEventMessage){
				rxChar = (uint8_t) RxEvent.value.v;
				
				// Write to rxBuffer
				SerialrxBufferPush(rxChar);
				
				continue;
			}
			// Pack data if timeout
			if(RxEvent.status == osEventTimeout){
				
				// @todo debug pack data and send to mailbox
				SerialRxBuffer[SerialRxBufferIdx] = 0;
				
				// push mail data
				SerialkgBufferPush(SerialRxBuffer, SerialRxBufferIdx);
				
				// clear time out
				ec21_status.ec21_cnn_timeout = 0;
				
				// Change state
				SerialRxBufferIdx = 0;
				state = SERIAL_RX_STATE_S0;
				cmd_timeout = osWaitForever;
				
				continue;
			}
			break;
		}
		osThreadYield(); 
	}
}

void vSerialTransfer(const void* args){
	for(;;){
		//report data to ec21
		serial_report_data();
		
		osDelay(1000);
		osThreadYield(); 
	}
}

u16 time_reset_crash = 180;
static uint16_t loss_gps_cnt = 0;
void vSerialTask(const void* args){
	osEvent evt;
  Serial_Mail_Typedef * msg;
	Buzzer_StatusDef buzz;
	for(;;){
		evt = osMailGet(SerialPkgMailBox, 1000);
    if (evt.status == osEventMail) {
			/* Get message*/
			msg = (Serial_Mail_Typedef*)evt.value.p;
			msg->data[msg->size] = 0;
			//process data
			if(msg->data[0] == '>'){
				u8 cs = CRC_8BitsCompute(msg->data, msg->size - 1);
				if(cs == msg->data[msg->size - 1]){
					msg->data[msg->size - 1] = 0;
					Serial_Parse((msg->data + 1), (msg->size - 2));
				}else{
					SerialPushStr("~~~~~~~~~~~~~~~~ERROR CHECK SUM!!", END_LF);
				}
				ec21_status.flg_ec21_rdy = 1;
				ec21_status.flg_ec21_crash = 0;
			}
			/* Free data block mail box */
			osMailFree(SerialPkgMailBox, msg);
		}else if(evt.status == osEventTimeout){
			ec21_status.ec21_cnn_timeout ++;
			
			if((ec21_status.flg_ec21_crash == 0) && (ec21_status.flg_ec21_rdy == 1)){
				if((buzz == OverSpeed) || (buzz == DriveContinuousOverTime)){
					Set_Led_Status(BUZZER, None);
				}
					
				if(ec21_status.ec21_cnn_timeout >= 120){
					ec21_status.ec21_cnn_timeout = 0;
					EC21_PW_Reset();
					ec21_status.ec21_pw_rst_cnt ++;
					if(ec21_status.ec21_pw_rst_cnt > 5){
						//waranty notify -> LCD
						ec21_status.flg_ec21_crash = 1;
					}
					
					/* clear ec21 data */
					memset(&ec21_data, 0x0, sizeof(ec21_message_def));
				}
			}else	if(ec21_status.ec21_cnn_timeout >= time_reset_crash){
					if(time_reset_crash == 180){
						time_reset_crash = 900;
					}
					
					ec21_status.ec21_cnn_timeout = 0;
					EC21_PW_Reset();
					ec21_status.ec21_pw_rst_cnt ++;					
					/* clear ec21 data */
					memset(&ec21_data, 0x0, sizeof(ec21_message_def));
			}
			if(ec21_status.ec21_cnn_timeout > 10){
				//clear data
				memset(&ec21_data, 0x0, sizeof(ec21_message_def));
			}
		}
		/* get buzzer state */
		buzz = get_buzer_state();
		if(ec21_data.gps_status == 1){
			if(ec21_data.ov_speed_status == 1){
				Set_Led_Status(BUZZER, OverSpeed);
			}else if(ec21_data.drv_continue_status == 1){
				Set_Led_Status(BUZZER, DriveContinuousOverTime);
			}else if((buzz == OverSpeed) || (buzz == DriveContinuousOverTime)){
				Set_Led_Status(BUZZER, None);
			}
			loss_gps_cnt = 0;
		}else{
			if((buzz == OverSpeed) || (buzz == DriveContinuousOverTime))
				Set_Led_Status(BUZZER, None);
			
			if(PowerData.power_mode == POWER_SAVE){
				loss_gps_cnt ++;
				if(loss_gps_cnt >= 300){
					loss_gps_cnt = 0;
					/* poweroff device */
					SerialPushStr("<<<<<<- NO GPS OVER 5-MINUTE IN BATTERY MODE => POWER OFF AFTER 10s ->>>>>>", END_LF);
					osDelay(2000);
					device_battery_off(10);
				}
			}
		}
    osThreadYield();
	}
}

void SerialPushByte(uint8_t data){
	UART_SendByte(USART_EC21, data);
}

void SerialPushStr(char* data, ENDLINE Type){
	uint16_t idx  = 0;
	uint16_t size = strlen(data);
	
	osSemaphoreWait(sSerialFree,osWaitForever);
	
	for(; idx < size; idx++){
		/* Send byte through the UART peripheral */
		SerialPushByte(data[idx]);	
	}
	
	switch(Type){
		case END_NONE:	
			break;
		case END_CR:
			SerialPushByte(0x0D);	
			break;
		case END_LF:	
			SerialPushByte(0x0A);
			break;
		case END_CRLF:
			SerialPushByte(0x0D);	
		  SerialPushByte(0x0A);	
			break;
		case END_SUB:	
			SerialPushByte(0x1A);	
			break;
	}
	osDelay(300);
	osSemaphoreRelease(sSerialFree);
}

void SerialPushBuffer(uint8_t* data, uint16_t size){
	uint16_t idx;
	
	osSemaphoreWait(sSerialFree,osWaitForever);
	
	for(idx = 0; idx < size; idx++){
		/* Send byte through the UART peripheral */
		SerialPushByte(data[idx]);	
	}
	osSemaphoreRelease(sSerialFree);
}

void SerialPushBufferWithCS(uint8_t * data, uint16_t size) {
  uint8_t cs = 0;
  cs = CRC_8BitsCompute(data, size);
  SerialPushBuffer(data, size);
  SerialPushByte(cs);
}

void UART_SendByte(USART_TypeDef* USARTx, uint8_t data){
	int32_t tick, delayPeriod;
	tick = osKernelSysTick(); // get start value of the Kernel system tick
	delayPeriod = osKernelSysTickMicroSec(50000);//50ms timeout
	
	/* Wait for USART Tx buffer empty */
	while((USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET) && (osKernelSysTick() - tick) < delayPeriod);
	/* Send byte through the UART peripheral */
	USART_SendData(USARTx, data);
}

void PrintError(char* err){
	uint16_t len = strlen(err);
	uint16_t idx;
	for(idx = 0; idx < len; idx++)
		UART_SendByte(USART_EC21, err[idx]);
}

void SerialUARTInit(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* Enable USART_EC21 GPIO clock */
	RCC_AHBPeriphClockCmd(USART_EC21_GPIO_CLK_TX, ENABLE);
	RCC_AHBPeriphClockCmd(USART_EC21_GPIO_CLK_RX, ENABLE);
	
	/* Enable USART_EC21 Clock */
	RCC_APB2PeriphClockCmd(USART_EC21_CLK, ENABLE);
	
	/* CFG AF mode */
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	/* Configure USART_COM_Tx */
	GPIO_InitStructure.GPIO_Pin = USART_EC21_TxPin;
	GPIO_Init(USART_EC21_GPIO_TX, &GPIO_InitStructure);
	GPIO_PinAFConfig(USART_EC21_GPIO_TX, USART_EC21_TxPinSource, USART_EC21_AF);
	
	/* Configure USART_COM_Rx */
	GPIO_InitStructure.GPIO_Pin = USART_EC21_RxPin;
	GPIO_Init(USART_EC21_GPIO_RX, &GPIO_InitStructure);
	GPIO_PinAFConfig(USART_EC21_GPIO_RX, USART_EC21_RxPinSource, USART_EC21_AF);
	
	/* USART_COM configuration*/
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = 230400;
	USART_Init(USART_EC21, &USART_InitStructure);
	
  USART_ITConfig(USART_EC21, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART_EC21, ENABLE);
	
	NVIC_EnableIRQ(USART_EC21_IRQn);
}

void USART_EC21_IRQHandler(void){
	static uint8_t rxd = 0;
	/* Check DR register in not empty */
	if(USART_GetITStatus(USART_EC21, USART_IT_RXNE) != RESET){
		/* Receive the character */
		rxd =  USART_ReceiveData(USART_EC21);
		if(SerialRxMsgBox != NULL)
			osMessagePut(SerialRxMsgBox, (uint32_t)rxd, 0);
	}
}


#include "Serial.h"
#include "osObjects.h"
#include "RFID.h"

void vSerialReceiver(const void* args);

void vSerialSendData(const void* args);

void vSerialTask(const void* args);

void vWatchdogTask(const void* args);

void SerialUARTInit(void);

static void SerialSendBuffer(uint8_t* data, uint16_t size);

ec21_message_def ec21_data;
mcu_config_def mcu_config;

/* MsgBox to store data received from Serial port */
osMessageQDef(SerialRxMsgBox, SerialBuffer_SIZE, uint8_t);
osMessageQId  SerialRxMsgBox;

osMailQDef(SerialRXMailBox, 5, Serial_Mail_Typedef);
osMailQId SerialRXMailBox;

osMailQDef(SerialTXMailBox, 5, Serial_Mail_Typedef);
osMailQId SerialTXMailBox;

uint8_t SerialRxBuffer[SerialBuffer_SIZE];
uint16_t SerialRxBufferIdx = 0;

osThreadId serial_receiver_thread;
osThreadDef(vSerialReceiver, osPriorityNormal, 1, 2024);

osThreadId serial_send_thread;
osThreadDef(vSerialSendData, osPriorityNormal, 1, 2048);

osThreadId serial_thread;
osThreadDef(vSerialTask, osPriorityNormal, 1, 2048);

void load_mcu_config(void){
	mcu_config.buzz_level = FLASH_ReadData(BUZZER_LEVEL_ADDR);

	mcu_config.slience_mode = FLASH_ReadData(SLIENCE_MODE_ADDR);
	
	mcu_config.device_id = FLASH_ReadData(RF_DEVICE_ID);
}

void Serial_Init(void){
	/* load rfid config */
	load_mcu_config();
	
	SerialRxMsgBox = osMessageCreate(osMessageQ(SerialRxMsgBox), NULL);
	
  /* Create mail queue */
  SerialRXMailBox = osMailCreate(osMailQ(SerialRXMailBox), NULL);
  if (SerialRXMailBox == NULL) {
    return;
  }
	
	SerialTXMailBox = osMailCreate(osMailQ(SerialTXMailBox), NULL);
  if (SerialTXMailBox == NULL) {
    return;
  }
	
	serial_receiver_thread = osThreadCreate(osThread(vSerialReceiver), NULL);
  if(!serial_receiver_thread) return;
	
	serial_send_thread = osThreadCreate(osThread(vSerialSendData), NULL);
  if(!serial_send_thread) return;
	
	serial_thread = osThreadCreate(osThread(vSerialTask), NULL);
  if(!serial_thread) return;
	
	/* Init usart */
	SerialUARTInit();
}

static void SerialrxBufferPush(uint8_t data){
	SerialRxBuffer[SerialRxBufferIdx++] = data;
	if(SerialRxBufferIdx >= SerialBuffer_SIZE)
		SerialRxBufferIdx = 0;
}

static void SerialkgBufferPush(uint8_t * data, uint16_t size) {
  /* Copy data */
  Serial_Mail_Typedef * msg;

  msg = (Serial_Mail_Typedef*)osMailAlloc(SerialRXMailBox, 1000);

  if (msg == NULL) {
    return;
  }
  if (size > SerialBuffer_SIZE) size = SerialBuffer_SIZE;

  msg->size = size;
  memcpy(msg->data, data, size);

  /* Put mail */
  osMailPut(SerialRXMailBox, msg);
}

void SerialTxBufferPush(uint8_t * data, uint16_t size) {
  /* Copy data */
  Serial_Mail_Typedef * msg;

  msg = (Serial_Mail_Typedef*)osMailAlloc(SerialTXMailBox, 1000);

  if (msg == NULL) {
    return;
  }
  if (size > SerialBuffer_SIZE) size = SerialBuffer_SIZE;

  msg->size = size;
  memcpy(msg->data, data, size);

  /* Put mail */
  osMailPut(SerialTXMailBox, msg);
}

static void device_status_parser(u8 * msg){
	/* gps status */
	ec21_data.gps_status = msg[0] & 0x01;
	/* server connected */
	ec21_data.tcp_status = (msg[0] >> 1) & 0x01;
	/* internet connected */
	ec21_data.internet_status = (msg[0] >> 2) & 0x01;
	/* acc on/off status */
	ec21_data.acc_status = (msg[0] >> 3) & 0x01;
	/* login / logout status */
	ec21_data.login_status = (msg[0] >> 4) & 0x01;
	/* over speed warning status */
	ec21_data.ov_speed_status = (msg[0] >> 5) & 0x01;
	/* warning of driver continue */
	ec21_data.drv_continue_status = (msg[0] >> 6) & 0x01;
	/* sim status */
	ec21_data.sim_status = (msg[0] >> 7) & 0x01;
	/* buzzer level */
	ec21_data.buzz_level = msg[1];
	/* slience mode */
	ec21_data.slience_mode = msg[2];
	
	/* sync buzzer level to mcu setting */
	if(mcu_config.buzz_level != ec21_data.buzz_level){
		mcu_config.buzz_level = ec21_data.buzz_level;
		FLASH_WriteData(BUZZER_LEVEL_ADDR, (uint32_t)mcu_config.buzz_level, 0);
	}
	
	/* sync slience mode to mcu setting */
	if(mcu_config.slience_mode != ec21_data.slience_mode){
		mcu_config.slience_mode = ec21_data.slience_mode;
		FLASH_WriteData(SLIENCE_MODE_ADDR, (uint32_t)mcu_config.slience_mode, 0);
	}
}

static void rf_device_id_parser(u8 * msg){
	u8 cs = 0;
	u8 send_msg[16];

	if(msg[0] == 1){
		mcu_config.device_id = (msg[1] << 24) | (msg[2] << 16) | (msg[3] << 8) | msg[4];
		FLASH_WriteData(RF_DEVICE_ID, (u32)mcu_config.device_id, 0);
	}
	memset(send_msg, 0x0, 16);
	send_msg[0] = '>';
	send_msg[1] = 10;
	send_msg[2] = (mcu_config.device_id >> 24) & 0xFF;
	send_msg[3] = (mcu_config.device_id >> 16) & 0xFF;
	send_msg[4] = (mcu_config.device_id >> 8) & 0xFF;
	send_msg[5] = mcu_config.device_id & 0xFF;
	cs = CRC_8BitsCompute(send_msg, 6);
	send_msg[6] = cs;
	SerialTxBufferPush(send_msg, 7);
}

static void driver_data_parser(u8 * msg){
	ec21_data.DriverInfo.LoginStatus = (enum_login_status)(msg[0] & 0x0F);
	ec21_data.DriverInfo.sync = msg[0] >> 4;	
	ec21_data.DriverInfo.UID = (uint64_t)(((uint64_t)msg[5] << 32) | ((uint64_t)msg[4] << 24) | ((uint64_t)msg[3] << 16) | ((uint64_t)msg[2] << 8) | ((uint64_t)msg[1] & 0xFF));

	memcpy(ec21_data.DriverInfo.Name, msg + 8, MAX_SIZE_DRIVER_NAME);
	memcpy(ec21_data.DriverInfo.License, msg + 8 + MAX_SIZE_DRIVER_NAME, MAX_SIZE_DRIVER_LICENSE);
	memcpy(ec21_data.DriverInfo.IssDate, msg + 8 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE, MAX_SIZE_TAG_DATE);
	memcpy(ec21_data.DriverInfo.ExpDate, msg + 8 + MAX_SIZE_DRIVER_NAME + MAX_SIZE_DRIVER_LICENSE + MAX_SIZE_TAG_DATE, MAX_SIZE_TAG_DATE);
	
	if(ec21_data.DriverInfo.LoginStatus == STATUS_LOGOUT){
		Set_Led_Status(LED_DRIVER, PutOutTag);
		/* Set buzzer logout  */
		Set_Led_Status(BUZZER, bDriverLogout);
	}else if(ec21_data.DriverInfo.LoginStatus == STATUS_LOGIN){
		Set_Led_Status(LED_DRIVER, DriverLogin);
		/* Set buzzer login  */
		Set_Led_Status(BUZZER, bDriverLogin);
	}
	
	if((ec21_data.DriverInfo.sync == 1) && (UID__ == ec21_data.DriverInfo.UID)){
		if(!WriteDriverInfoToTag(ec21_data.DriverInfo)){
			send_tag_uid(TagUID, SYNC_CMD);
		}
	}
}

uint8_t serial_buffer[128];
void Serial_Parse(uint8_t* data, uint16_t Len){//     HD   PARAM
	u8 cmd = 0;
	u8 cs = 0;
	//process data
	if(data[0] == '>'){    // >>0x01
		cmd = data[1];
		switch(cmd){
			case GPS_DATA_MESSAGE://0x01
				device_status_parser(data + 2);
				break;
			case DEVICE_SETTING_MESSAGE://ID
				rf_device_id_parser(data + 2);
				break;
			case DRIVER_MESSAGE://uid, name, license, iss date, exp date 
				drv_reload_time = 0;
				driver_data_parser(data + 2);
				break;
			default:
				break;
		}
	}else{
		if (!strncmp((char*)data, "GOTO_BOOT_MODE", 15)) {
			serial_buffer[0] = '>';
			serial_buffer[1] = '>';
			serial_buffer[2] = 'O';
			serial_buffer[3] = 'K';
			cs = CRC_8BitsCompute(serial_buffer, 4);
			serial_buffer[4] = cs;
			SerialTxBufferPush(serial_buffer, 5);
			osDelay(3000);
			//set flag update firmware on bootloader
			FLASH_WriteData(FLG_UPDATE_FW_ADDR, (u32)0x01, 1);
		} else if (!strncmp((char*)data, "VERSION?", 8)) {
			serial_buffer[0] = '>';
			serial_buffer[1] = '>';
			serial_buffer[2] = 'V';
			serial_buffer[3] = RFID_VERSION;
			cs = CRC_8BitsCompute(serial_buffer, 4);
			serial_buffer[4] = cs;
			SerialTxBufferPush(serial_buffer, 5);
		} else if (!strncmp((char*)data, "REBOOT", 6)) {
			NVIC_SystemReset();
		}
	}
}

void vSerialSendData(const void* args){
	osEvent evt;
  Serial_Mail_Typedef * msg;

	for(;;){
		evt = osMailGet(SerialTXMailBox, 1000);
    if (evt.status == osEventMail) {
			/* Get message*/
			msg = (Serial_Mail_Typedef*)evt.value.p;
			
			SerialSendBuffer(msg->data, msg->size);
			osDelay(200);

			/* Free data block mail box */
			osMailFree(SerialTXMailBox, msg);
		}
		
    osThreadYield();
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
				cmd_timeout = 100;
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

u8 ec21_time = 0;
void vSerialTask(const void* args){
	osEvent evt;
  Serial_Mail_Typedef * msg;

	for(;;){
		evt = osMailGet(SerialRXMailBox, 1000);
    if (evt.status == osEventMail) {
			/* Get message*/
			msg = (Serial_Mail_Typedef*)evt.value.p;
			msg->data[msg->size] = 0;
			//process data
			if(msg->data[0] == '>'){
				u8 cs = CRC_8BitsCompute(msg->data, msg->size - 1);
				if(cs == msg->data[msg->size - 1]){
					msg->data[msg->size - 1] = 0;
					Serial_Parse((msg->data + 1), (msg->size - 2));//hd param
				}
			}
			gpio_bit_reset(LED_POWER_GPIO_PORT,LED_POWER_GPIO_PIN);
			ec21_time = 0;
			/* Free data block mail box */
			osMailFree(SerialRXMailBox, msg);
		}else if(evt.status == osEventTimeout){
			ec21_time ++;
			if(ec21_time > 60){
				ec21_time = 0;
				gpio_bit_set(LED_POWER_GPIO_PORT,LED_POWER_GPIO_PIN);
			}
		}
		
    osThreadYield();
	}
}

static void SerialSendByte(uint8_t data){
	UART_SendByte(USART_EC21, data);
}

static void SerialSendBuffer(uint8_t* data, uint16_t size){
	uint16_t idx;
	
	for(idx = 0; idx < size; idx++){
		/* Send byte through the UART peripheral */
		SerialSendByte(data[idx]);	
	}
}

void UART_SendByte(u32 USARTx, uint8_t data){
	int32_t tick, delayPeriod;
	tick = osKernelSysTick(); // get start value of the Kernel system tick
	delayPeriod = osKernelSysTickMicroSec(50 * 1000);//5s timeout
	/* Wait for USART Tx buffer empty or timeout*/
	while((usart_flag_get(USARTx, USART_FLAG_TBE) == RESET) && (osKernelSysTick() - tick) < delayPeriod);
	usart_data_transmit(USARTx,(uint8_t)data);
	
}

void SerialUARTInit(void){
		rcu_periph_clock_enable(RCU_GPIOA);
	
		rcu_periph_clock_enable(RCU_USART0);
	
	 /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	
		/* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0,115200);
		usart_parity_config(USART0, USART_PM_NONE);
		usart_word_length_set(USART0, USART_WL_8BIT);
		usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
    nvic_irq_enable(USART0_IRQn, 0, 0);
		
		usart_interrupt_enable(USART0, USART_INT_RBNE);
}

void USART_EC21_IRQHandler(void){
	static uint8_t rxd = 0;
	/* Check DR register in not empty */
	if(usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE) != RESET){
		/* Receive the character */
		rxd =  usart_data_receive(USART0);
		if(SerialRxMsgBox != NULL)
			osMessagePut(SerialRxMsgBox, (uint32_t)rxd, 0);
	}
}


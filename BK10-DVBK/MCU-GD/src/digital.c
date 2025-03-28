#include "digital.h"
#include "utils.h"
#include "osObjects.h"
#include "canbus.h"

void UART3Init(uint32_t baud);
void USART1Init(uint32_t baud);
void vUART3Receiver(const void* args);
void vUSART1Receiver(const void* args);
void vJ1587HandlerTask(const void* args);
void vJ1587RequestTask(const void* args);
static void J1587PkgBufferPush(uint8_t* data, uint16_t size);

static void FuelTimeoutHanler (void const *arg);
uint8_t flg_init_j1587_uart = 0;

/* argument for the timer call back function */
static osTimerDef (FuelTimer, FuelTimeoutHanler);

/* MsgBox to store data received from UART3 */
osMessageQDef(UART3RxMsgBox, UART3RxBuffer_SIZE, uint8_t);
osMessageQId  UART3RxMsgBox;

/* MsgBox to store data received from USART1 */
osMessageQDef(USART1RxMsgBox, USART1RxBuffer_SIZE, uint8_t);
osMessageQId  USART1RxMsgBox;

/* UART3  buffer */
uint8_t UART3RxBuffer[UART3RxBuffer_SIZE];
uint16_t UART3RxBufferIdx = 0;

/* USART1  buffer */
uint8_t USART1RxBuffer[USART1RxBuffer_SIZE];
uint16_t USART1RxBufferIdx = 0;

/* thread reveiver data for UART3 */
osThreadId UART3_receiver_thread;
osThreadDef(vUART3Receiver, osPriorityNormal, 1, 0);

/* thread reveiver data for USART1 */
osThreadId USART1_receiver_thread;
osThreadDef(vUSART1Receiver, osPriorityNormal, 1, 0);

/* Thread J1587 task parser */
osThreadId vJ1587HandlerThread;
osThreadDef(vJ1587HandlerTask, osPriorityNormal, 1, 1024);

osThreadId vJ1587RequestThread;
osThreadDef(vJ1587RequestTask, osPriorityNormal, 1, 0);

osMailQDef(J1587PkgMailBox, 5, J1587_Mail_Typedef);
osMailQId J1587PkgMailBox;

fuel_232_def fuel_232_value;

static void FuelTimeoutHanler (void const *arg){
	//check timeout fule 1
	if(fuel_232_value.fuel1_cnn == 1){
		fuel_232_value.fuel1_tm ++;
		if(fuel_232_value.fuel1_tm > 30){
			fuel_232_value.fuel1_cnn = 0;
			fuel_232_value.fuel1_val = 0;
			fuel_232_value.fuel1_tm = 0;
		}
	}
	//check timeout fuel 2
	if(fuel_232_value.fuel2_cnn == 1){
		fuel_232_value.fuel2_tm ++;
		if(fuel_232_value.fuel2_tm > 30){
			fuel_232_value.fuel2_cnn = 0;
			fuel_232_value.fuel2_val = 0;
			fuel_232_value.fuel2_tm = 0;
		}
	}
}

void digital_init(void){
	static osTimerId uTimId;  
	
	//init os messsage queue
	UART3RxMsgBox = osMessageCreate(osMessageQ(UART3RxMsgBox), NULL);
	USART1RxMsgBox = osMessageCreate(osMessageQ(USART1RxMsgBox), NULL);
	
	//create thread UART3 receiver
	UART3_receiver_thread = osThreadCreate(osThread(vUART3Receiver), NULL);
  if(!UART3_receiver_thread) return;
	
	//create thread USART1 receiver
	USART1_receiver_thread = osThreadCreate(osThread(vUSART1Receiver), NULL);
  if(!USART1_receiver_thread) return;
	
	if(mcu_config.TypeOBD == J1587_TYPE){
		//Create mail queue
		J1587PkgMailBox = osMailCreate(osMailQ(J1587PkgMailBox),NULL);
		if(J1587PkgMailBox == NULL) return;
		//create thread J1587 handler Task
		vJ1587HandlerThread = osThreadCreate(osThread(vJ1587HandlerTask), NULL);
		if(!vJ1587HandlerThread) return;
		
		//create thread J1587 request Task
		vJ1587RequestThread = osThreadCreate(osThread(vJ1587RequestTask), NULL);
		if(!vJ1587RequestThread) return;
	}
	
	uTimId = osTimerCreate (osTimer(FuelTimer), osTimerPeriodic, (void*)0);
	if (uTimId != NULL){
		osStatus status;
    // start timer with delay 1000ms
    status = osTimerStart(uTimId, 1000);
    if (status != osOK) {
      return;
    }
  }
	
	//init uart 2&4
	USART1Init(mcu_config.baud_2_value);
	UART3Init(mcu_config.baud_4_value);
}

static uint16_t Fuel_232_parse(uint8_t* data, uint8_t data_len){
	uint16_t result = 0;
	int32_t tmp_value = 0;
	if((data[0] == 0xFA) && (data[1] == 0xFA)){//FA FA 01 B3 B8 60
		uint8_t cs = cal_sum_buffer(data, 5);
		uint8_t decr = 0;
		if(cs == data[5]){
			decr = data[3] ^ data[4];
			tmp_value = decr << 8 | data[4];
			if((tmp_value > 0) && (tmp_value < 4200)){
				result = tmp_value;
			}
		}
	}else if(!strncmp((char*)data, "*RFV01", 6)){//*RFV01000.0097
		if(data[9] != '.') return 0;
		str_trim(data);
		if(CLS2_check_mes((char*)data, strlen((char*)data))){
			float cls2_ad = 0;
			char s_cls2_ad[8] = {0x0,0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
			memcpy(s_cls2_ad, data + 6, 6);
			cls2_ad = atof(s_cls2_ad);
			
			if(cls2_ad > (float)100.0) cls2_ad = 100.0;
			if(cls2_ad < (float)0.0) cls2_ad = 0.0;
			
			tmp_value = ((cls2_ad * (float)4095.0) / (float)100.0);
			
			if((tmp_value > 0) && (tmp_value < 4200)){
				result = tmp_value;
			}
		}
	}else if(data[data_len - 1] == CRC_8BitsCompute(data, data_len - 1)){
		tmp_value = (data[5] << 8 | data[4]);
		if(tmp_value >= 0){
			result = tmp_value;
		}
	}
	if(result == 0) result = 1;
	return result;
}

void USART1PushBuffer(uint8_t* data, uint16_t size){
	uint16_t idx;

	for(idx = 0; idx < size; idx++){
		/* Send byte through the UART peripheral */
		UART_SendByte(USART1, data[idx]);	
	}
}

void UART3PushBuffer(uint8_t* data, uint16_t size){
	uint16_t idx;

	for(idx = 0; idx < size; idx++){
		/* Send byte through the UART peripheral */
		UART_SendByte(UART3, data[idx]);	
	}
}

void vUSART1Receiver(const void* args){
osEvent RxEvent;
	static uint8_t rxChar;
	uint32_t cmd_timeout = osWaitForever;
	
	static enum {
		USART1_RX_STATE_S0,
		USART1_RX_STATE_S1,
	} state;
	
	for(;;){
		/*RX*/
		RxEvent = osMessageGet(USART1RxMsgBox, cmd_timeout);
		
		// Process state
		switch(state){
		case USART1_RX_STATE_S0:
				// Check data
			if(RxEvent.status == osEventMessage){
				rxChar = (uint8_t) RxEvent.value.v;
				
				USART1RxBuffer[USART1RxBufferIdx++] = rxChar;
				if(USART1RxBufferIdx >= USART1RxBuffer_SIZE)
					USART1RxBufferIdx = 0;
				
				// Change state
				cmd_timeout = 5;
				state = USART1_RX_STATE_S1;
			}
			break;
		case USART1_RX_STATE_S1:
			// Data received
			if(RxEvent.status == osEventMessage){
				rxChar = (uint8_t) RxEvent.value.v;
				
				// Write to rxBuffer
				USART1RxBuffer[USART1RxBufferIdx++] = rxChar;
				if(USART1RxBufferIdx >= USART1RxBuffer_SIZE)
					USART1RxBufferIdx = 0;
					
				continue;
			}
			// Pack data if timeout
			if(RxEvent.status == osEventTimeout){
				
				// @todo debug pack data and send to mailbox
				USART1RxBuffer[USART1RxBufferIdx] = 0;
				
				if(mcu_config.TypeOBD == J1587_TYPE){
					J1587PkgBufferPush(USART1RxBuffer, USART1RxBufferIdx);
				}else{
					// push mail data
					if(USART1RxBufferIdx > 5){
						fuel_232_value.fuel2_val = Fuel_232_parse(USART1RxBuffer, USART1RxBufferIdx);
						if(fuel_232_value.fuel2_val != 0){
							fuel_232_value.fuel2_tm = 0;
							fuel_232_value.fuel2_cnn = 1;
						}
					}else if((USART1RxBuffer[0] == '#') && (USART1RxBuffer[1] == '#')){
						USART1PushBuffer((uint8_t*)"UART2 OK\n", 9);
					}
				}
				// Change state
				USART1RxBufferIdx = 0;
				state = USART1_RX_STATE_S0;
				cmd_timeout = osWaitForever;
				
				continue;
			}
			break;
		}
		osThreadYield(); 
	}
}

void vUART3Receiver(const void* args){
osEvent RxEvent;
	static uint8_t rxChar;
	uint32_t cmd_timeout = osWaitForever;
	
	static enum {
		UART3_RX_STATE_S0,
		UART3_RX_STATE_S1,
	} state;
	
	for(;;){
		/*RX*/
		RxEvent = osMessageGet(UART3RxMsgBox, cmd_timeout);
		
		// Process state
		switch(state){
		case UART3_RX_STATE_S0:
				// Check data
			if(RxEvent.status == osEventMessage){
				rxChar = (uint8_t) RxEvent.value.v;
				UART3RxBuffer[UART3RxBufferIdx++] = rxChar;
				if(UART3RxBufferIdx >= UART3RxBuffer_SIZE)
					UART3RxBufferIdx = 0;
				
				// Change state
				cmd_timeout = 10;
				state = UART3_RX_STATE_S1;
			}
			break;
		case UART3_RX_STATE_S1:
			// Data received
			if(RxEvent.status == osEventMessage){
				rxChar = (uint8_t) RxEvent.value.v;
				
				// Write to rxBuffer
				UART3RxBuffer[UART3RxBufferIdx++] = rxChar;
				if(UART3RxBufferIdx >= UART3RxBuffer_SIZE)
					UART3RxBufferIdx = 0;

				continue;
			}
			// Pack data if timeout
			if(RxEvent.status == osEventTimeout){
				
				// @todo debug pack data and send to mailbox
				UART3RxBuffer[UART3RxBufferIdx] = 0;
				
				// push mail data
				if(UART3RxBufferIdx > 5){
					fuel_232_value.fuel1_val = Fuel_232_parse(UART3RxBuffer, UART3RxBufferIdx);
					if(fuel_232_value.fuel1_val != 0){
						fuel_232_value.fuel1_tm = 0;
						fuel_232_value.fuel1_cnn = 1;
					}
				}else if((UART3RxBuffer[0] == '#') && (UART3RxBuffer[1] == '#')){
					UART3PushBuffer((uint8_t*)"UART1 OK\n", 9);
				}
				// Change state
				UART3RxBufferIdx = 0;
				state = UART3_RX_STATE_S0;
				cmd_timeout = osWaitForever;
				
				continue;
			}
			break;
		}
		osThreadYield(); 
	}
}

void USART1Init(uint32_t baud){
		rcu_periph_clock_enable(RCU_GPIOA);
	
		rcu_periph_clock_enable(RCU_USART1);
	
		rcu_periph_clock_enable(J1587_RDE_GPIO_CLK);
		gpio_init(J1587_RDE_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, J1587_RDE_PIN);
		gpio_bit_set(J1587_RDE_GPIO_PORT, J1587_RDE_PIN);
		
	  /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	
		/* USART configure */
    usart_deinit(USART1);
	
		if(mcu_config.TypeOBD == J1587_TYPE){
			usart_baudrate_set(USART1, 9600);
		}else{
			usart_baudrate_set(USART1, baud);
		}
		
		usart_parity_config(USART1, USART_PM_NONE);
		usart_word_length_set(USART1, USART_WL_8BIT);
		usart_stop_bit_set(USART1, USART_STB_1BIT);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);
    nvic_irq_enable(USART1_IRQn, 0, 0);
		
		usart_interrupt_enable(USART1, USART_INT_RBNE);
		
		flg_init_j1587_uart = 1;
}

void UART3Init(uint32_t baud){
		rcu_periph_clock_enable(RCU_GPIOC);
	
		rcu_periph_clock_enable(RCU_UART3);
		
	  /* connect port to USARTx_Tx */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
		
    /* connect port to USARTx_Rx */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

		/* USART configure */
    usart_deinit(UART3);
    usart_baudrate_set(UART3, baud);
		usart_parity_config(UART3, USART_PM_NONE);
		usart_word_length_set(UART3, USART_WL_8BIT);
		usart_stop_bit_set(UART3, USART_STB_1BIT);
    usart_receive_config(UART3, USART_RECEIVE_ENABLE);
    usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
    usart_enable(UART3);
    nvic_irq_enable(UART3_IRQn, 0, 0);
		
		usart_interrupt_enable(UART3, USART_INT_RBNE);
		
		UART_SendByte(UART3, 0x05);
		UART_SendByte(UART3, 0x05);
		UART_SendByte(UART3, 0x05);
		UART_SendByte(UART3, 0x05);
}

void USART1_IRQHandler(void){
	static uint8_t rxd = 0;
	/* Check DR register in not empty */
	if(usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE) != RESET){
		/* Receive the character */
		rxd =  usart_data_receive(USART1);
		if(USART1RxMsgBox != NULL)
			osMessagePut(USART1RxMsgBox, (uint32_t)rxd, 0);
	}	
}

void UART3_IRQHandler(void){
	static uint8_t rxd = 0;
	/* Check DR register in not empty */
	if(usart_interrupt_flag_get(UART3,USART_INT_FLAG_RBNE) != RESET){
		/* Receive the character */
		rxd =  usart_data_receive(UART3);
		if(UART3RxMsgBox != NULL)
			osMessagePut(UART3RxMsgBox, (uint32_t)rxd, 0);
	}
}

//////////////////////////// ECU J1587 //////////////////////////////

uint8_t f_mid = 0;
static void J1587PkgBufferPush(uint8_t* data, uint16_t size){
	/* Copy data */
	J1587_Mail_Typedef *msg;
	
	msg = (J1587_Mail_Typedef*)osMailAlloc(J1587PkgMailBox, 1000);
	
	if(msg == NULL){
		SerialPushStr("Error: Full J1587PkgMailBox", END_LF);
		return;
	}
	if(size > 512) size = 512;
	
	msg->size = size;
	memcpy(msg->data, data, size);
	
	/* Put mail */
	osMailPut(J1587PkgMailBox, msg);
}

void ECUJ1587_Data_Request(uint8_t pid){
	uint32_t sum = 0;
	
	gpio_bit_set(J1587_RDE_GPIO_PORT, J1587_RDE_PIN);		
	osDelay(10);
	
	UART_SendByte(USART1, 0xAC);	
	UART_SendByte(USART1, 0x00);	
	UART_SendByte(USART1, pid);	
	
	/* Calculate checksum */
	sum = 0xAC + 0x00 + pid;
	sum = sum & 0xFF;
	sum = 256 - sum;															/* Two's Complement check sum */
	UART_SendByte(USART1, sum & 0xFF);				/* Send check sum */
	
	osDelay(10);
	gpio_bit_reset(J1587_RDE_GPIO_PORT, J1587_RDE_PIN);
}

void J1587HandlerRequest(uint32_t val){
	uint8_t pid = 0;
	
	switch(val){
		case 0:						
			pid = 168;			/* Battery Potential m (Voltage) */
			break;
		case 2:
			pid = 185;			/* Average Fuel Economy Km/L */
			break;
		case 4:
			pid = 194;			/* Transmitter System Diagnostic Code / Occurrence Count Table */
			break;
		case 6:
			pid = 245;			/* Total Vehicle Distance Km */
			break;
		case 8:
			pid = 247;			/* Total Engine Hours (On request) hour */
			break;
		case 10:
			pid = 250;			/* Total Fuel Used (On request) Lit */
			break;
		default:
			break;
	}
	
	/* Send request to ECU J1587 */
	if((pid != 0) && (pid != 250)){
		ECUJ1587_Data_Request(pid);
	}else{	
		ECUJ1587_Data_Request(250);
	}
}

FaultCodeJ1587 fc_j1587;
FaultCodeJ1587 sFaultCodeJ1587[16];
uint8_t sFaultCodeSizeJ1587 = 0;

uint8_t findFaultCodeJ1587(FaultCodeJ1587 fc){
	uint8_t val = 0;
	uint8_t idx = 0;
	
	for(idx = 0; idx < sFaultCodeSizeJ1587; idx++){
		if((sFaultCodeJ1587[idx].mid == fc.mid) && (sFaultCodeJ1587[idx].spid == fc.spid) && (sFaultCodeJ1587[idx].fmi == fc.fmi) && (sFaultCodeJ1587[idx].Count == fc.Count)){
			val = 1;
			break;
		}
	}
	
	return val;
}

uint8_t StoreFaultCodeJ1587(FaultCodeJ1587 fc){
	uint8_t val = 0;
	
	if(fc.spid == 0) return val;
	
	val = findFaultCodeJ1587(fc);
	
	if(val == 1) return val;
	
	if(sFaultCodeSizeJ1587 == 16) return val;	
	
	/* Set fault code */
	sFaultCodeJ1587[sFaultCodeSizeJ1587] = fc;
	
	sFaultCodeSizeJ1587 ++;
	if(sFaultCodeSizeJ1587 >= 16){
		sFaultCodeSizeJ1587 = 0;
	}
	
	return val;	
}

uint8_t J1587ParserPID(uint8_t* buf, uint8_t buf_size){
	double dal = 0;
	
	uint32_t val = 0;
	
	uint8_t len = 0, idx = 0;
	
	uint8_t pid = buf[0];
	
	canbus.connected = 1;
	
	switch(pid){
		/* Single Byte Parameters */
		case 33:			/* Clutch Cylinder Position */
		case 34:			/* Engine Retarder Switches Status */
		case 41: 			/* Cruise Control Switches Status */
		case 43:			/* Ignition Switch Status */
		case 44:			/* Attention/Warning Indicator Lamps Status */
		case 62:			/* Retarder Inhibit Status */
		case 64:			/* Direction Switch Status */
		case 65:			/* Service Brake Status */
		case 68:			/* Torque Limiting Factor */
		case 69:			/* Two-Speed Axle Switch Status */
		case 70:			/* Parking Brake Switch Status */
		case 71:			/* Idle Shutdown Timer Status */
		case 74:			/* Vehicle Speed Set Limit */
		case 83:			/* Vehicle Speed Limit Status */
			len = 2;
			break;
		case 84:			/* Speed */
			len = 2;	
			val = 805 * buf[1] / 1000;
			canbus.sp = (val & 0xFF);
			break;
		case 85:			/* Cruise Control Switch Status */
		case 86:			/* Cruise Control Set Speed */
		case 89:			/* VSG Switch Status */
		case 91:			/* Percent Throttle */
			len = 2;	
			break;
		case 92:			/* Percent Engine Load */
			len = 2;				
			val = 5 * buf[1] / 10;
			canbus.p_load = val & 0xFF;
			break;
		case 93:			/* Output Torque */
		case 94:			/* Fuel Delivery Pressure */
		case 98:			/* Engine Oil Level */
		case 100:			/* Engine Oil Pressure */
		case 102:			/* Turbo Boost Pressure (Gage) */
		case 105:			/* Intake Manifold Temperature */
		case 106:			/* Air Inlet Pressure */
		case 107:			/* Air Filter Differential Pressure */
		case 108:			/* Barometric Pressure */
			len = 1;	
			break;
		case 110:			/* Coolant Temperature */
			len = 2;
			canbus.temp = 5 * (buf[1] - 32) / 9;		
			break;
		case 111:			/* Coolant Level */
		case 121:			/* Engine Retarder Status */
		case 122:			/* Engine Retarder Percent */
			len = 2;	
			break;
		
		/* Double Byte Parameters */
		case 168:			/* Battery Voltage */
			len = 3;
			val = 50 * ((buf[2] << 8) | buf[1]);
			canbus.bat = val & 0xFFFF;
			break;
		case 171:			/* Ambient Air Temperature */
		case 173:			/* Exhaust Gas Temperature (DOC Inlet Temperature) */
		case 174:			/* Fuel Temperature */
		case 175:			/* Engine Oil Temperature */
		case 182:			/* Trip Fuel */
		case 183:			/* Instantaneous Fuel Economy (MPG) */
		case 184:			/* Instantaneous Fuel Economy (MPG) */
			len = 3;
			break;
		case 185:			/* Average Fuel Economy (MPG) */
			len = 3;
			canbus.econ_fuel = 1.66072 * ((buf[2] << 8) | buf[1]) / 1000.0;
			break;
		case 187:			/* VSG Set Speed */
		case 188:			/* Idle Set Speed */
		case 189:			/* Rated Engine Speed */
		case 203:			/* DPF Outlet Temperature */
			len = 3;
			break;
		case 190:			/* Engine Speed */
			len = 3;
			val = ((buf[2] << 8) | buf[1]) / 4;
			canbus.rpm = val & 0xFFFF;	
			break;
		/* Variable Length Parameters */
		case 192:			/* Multi-Section Parameter */
			len = buf[1] + 2;
			break;
		case 194:			/* Transmitter System Diagnostic Code / Occurrence Count Table */
			len = buf[1] + 2;
		
			/* Check size */
			if(len >= buf_size){
				break;
			}
		
			/* Loop for get fault code */
			for(idx = 2; idx < len; ){
				/* Fake Mid */
				fc_j1587.mid = f_mid;
				
				/* Get PID OR SID */
				fc_j1587.spid = buf[idx];
				idx = idx + 1;
				
				/* Get FMI */
				fc_j1587.fmi = buf[idx] & 0x0F;
				
				/* Get active or inactive */
				fc_j1587.iden = (buf[idx] >> 4) & 0x01;		
				
				/* Get active or inactive */
				fc_j1587.status_fault = (buf[idx] >> 6) & 0x01;						
				
				/* Get oc */
				fc_j1587.occ_count = (buf[idx] >> 7) & 0x01;	
				if(fc_j1587.occ_count == 1){
					idx = idx + 1;
					fc_j1587.Count = buf[idx];
				}else{
					fc_j1587.Count = 0;
				}
				
				/* Store fault code active */
				if(fc_j1587.status_fault == 0){
					StoreFaultCodeJ1587(fc_j1587);
				}
				
				idx = idx + 1;		
			}
			break;
		case 195:			/* Transmitter Data Request / Clear Count */
			len = buf[1] + 2;	
			break;
		case 196:			/* Diagnostic Data/count clear response */
		case 233:			/* Unit Number (Power Unit) */
		case 234:			/* Software Identification */
		case 235:			/* Total Idle Hours */
		case 236:			/* Total Idle Fuel Used */
		case 237:			/* Vehicle Identification Number (VIN) */
		case 243:			/* Device Identification */
		case 244:			/* Trip Miles */
			len = buf[1] + 2;
			break;
		case 245:			/* Total Miles */
			len = buf[1] + 2;
			if(buf[1] != 4){
				break;
			}
			
			canbus.s_km = (buf[5] << 24) | (buf[4] << 16) | (buf[3] << 8) | buf[2];	/* Mi */
			dal = 1.60934 * (double)canbus.s_km * 0.1;							
			canbus.s_km = (uint32_t)dal;	
			break;
		case 247:			/* Total Engine Hours */
		case 248:			/* Total VSG Hours */
		case 249:			/* Total Engine Revolutions */
			len = buf[1] + 2;
			break;
		case 250:			/* Total Fuel Used */
			len = buf[1] + 2;
			if(buf[1] != 4){
				break;
			}
			canbus.s_fuel = (buf[5] << 24) | (buf[4] << 16) | (buf[3] << 8) | buf[2];				
			dal = canbus.s_fuel  * 0.125 * 3.78541;			
			canbus.s_fuel = (uint32_t)dal;	
			break;
		case 251:			/* Clock */
		case 252:			/* Date */
			len = buf[1] + 2;
			break;
		default:
			break;
	}
	
	return len;
}

static void J1587DataHandler(uint8_t* buf, uint8_t buf_size){
	uint8_t idx, len;
	/* Check max size message */
	if(buf_size > 21){
		return;
	}
	
	/* For parser PID*/
	for(idx = 1; idx < buf_size - 1; ){
		/* fake MID for fault code */
		f_mid = buf[idx - 1];
		
		/* Paser data */
		len = J1587ParserPID(buf + idx, buf_size - idx);
		
		/* Stop no pid */
		if(len == 0){
			break;
		}
		
		/* Next pid */
		idx += len;
	}
}

uint32_t StepResq = 0;
void vJ1587RequestTask(const void* args){
	for(;;){
		osDelay(1000);
		if(ACC_STATUS == 0){
			SetECUClearData();
			if(flg_init_j1587_uart == 1){
				flg_init_j1587_uart = 0;
				NVIC_DisableIRQ(USART1_IRQn);
				usart_deinit(USART1);
			}
				osDelay(3000);
			osThreadYield(); 
			continue;
		}
		if(flg_init_j1587_uart == 0){
			//init uart j1587
			USART1Init(mcu_config.baud_2_value);		
		}
		
		StepResq++;
		if(StepResq > 45){
			StepResq = 0;
		}
		
		/* Send request */
		J1587HandlerRequest(StepResq);
		osThreadYield(); 
	}
}

void vJ1587HandlerTask(const void* args){
	osEvent evt;
	J1587_Mail_Typedef* msg;
	uint16_t sum_val = 0;
	for(;;){
		evt = osMailGet(J1587PkgMailBox, 3000);
		
		if(evt.status == osEventMail){
			/* Get message*/
			msg = (J1587_Mail_Typedef*)evt.value.p;
			
			sum_val = cal_sum16_buffer(msg->data, msg->size);
			
			sum_val = sum_val - msg->data[msg->size - 1];
			sum_val = sum_val & 0xFF;
			sum_val = 256 - sum_val;
			
			if(sum_val == msg->data[msg->size - 1]){
				J1587DataHandler(msg->data, msg->size);	
			}		
	
			/* Free data block mail box */
			osMailFree(J1587PkgMailBox, msg);
		}
		osThreadYield(); 
	}
}


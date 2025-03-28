#include "Serial.h"
#include "osObjects.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "utils.h"
#include "fw.h"

void vSerialReceiver(const void* args);

void vSerialTask(const void* args);

static void SerialPkgBufferPush(uint8_t* data, uint16_t size);

/* MsgBox to store data received from Serial port */
osMessageQDef(SerialRxMsgBox, SerialRxBuffer_SIZE, uint8_t);
osMessageQId  SerialRxMsgBox;

osMailQDef(SerialPkgMailBox, 3, Serial_Mail_Typedef);
osMailQId SerialPkgMailBox;

uint8_t SerialRxBuffer[SerialRxBuffer_SIZE];
uint16_t SerialRxBufferIdx = 0;
int server_SOS_Status=0;

osThreadId serial_receiver_thread;
osThreadDef(vSerialReceiver, osPriorityNormal, 1, 1024);

osThreadId serial_thread;
osThreadDef(vSerialTask, osPriorityNormal, 1, 2048);

osSemaphoreId sSerialFree;
osSemaphoreDef(sSerialFree);

void Serial_Init(void){
	/* Create TX Serial semaphore */
	sSerialFree = osSemaphoreCreate(osSemaphore(sSerialFree), 1);
	
	SerialRxMsgBox = osMessageCreate(osMessageQ(SerialRxMsgBox), NULL);
	
	/* Create Serial mail queue */
	SerialPkgMailBox = osMailCreate(osMailQ(SerialPkgMailBox), NULL);
	if(SerialPkgMailBox == NULL){
		SerialPushStr("Error! Creating SerialPkgMailBox", END_LF);
	}
	
	serial_receiver_thread = osThreadCreate(osThread(vSerialReceiver), NULL);
  if(!serial_receiver_thread) return;
	
	serial_thread = osThreadCreate(osThread(vSerialTask), NULL);
  if(!serial_thread) return;
}

static void SerialrxBufferPush(uint8_t data){
	SerialRxBuffer[SerialRxBufferIdx++] = data;
	if(SerialRxBufferIdx >= SerialRxBuffer_SIZE)
		SerialRxBufferIdx = 0;
}

uint8_t res_msg[160];
static void Serial_Parse(uint8_t* msg, uint8_t Len){
	if(!strncmp((char*)msg, "GD_GOTO_BOOT_MODE", 17) || !strncmp((char*)msg, "VERSION?", 8)){
		res_msg[0] = '>';
		res_msg[1] = '>';
		res_msg[2] = 'G';
		res_msg[3] = 'D';
		res_msg[4] = 'B';
		res_msg[5] = 'O';
		res_msg[6] = 'O';
		res_msg[7] = 'T';
		SerialPushBufferWithCS(res_msg, 8);
	}else if(msg[0] == 0xFF){//> 0xFF NPKG  CRC
		int res = 0;
		res = FirmwareInfoParse(msg + 1);
		if((res == 0) || (res == -2)){
			SerialPushBufferWithCS((uint8_t*)">>OK", 4);
		}else{
			SerialPushBufferWithCS((uint8_t*)">>ERROR", 7);
		}
	}else if(msg[0] == 0xFD){//> 0xFD ID LEN CRC DATA
		FirmwarePackageParse(msg + 1);
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
				
				// Push mailbox Data				
				SerialPkgBufferPush(SerialRxBuffer, SerialRxBufferIdx);

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

void vSerialTask(const void* args){
	osEvent evt;
	Serial_Mail_Typedef* msg;
	uint8_t cs = 0;
		
	/* Init usart */
	SerialUARTInit();
	
	SerialPushStr("GD MCU BOOTLOADER V1.1", END_LF);
	
	for(;;){
		/* Wait GSM message */
		evt = osMailGet(SerialPkgMailBox, 1000);//wait data
		if(evt.status == osEventMail){
			/* Get message*/
			msg = (Serial_Mail_Typedef*)evt.value.p;
			
			//process data
			if(msg->data[0] == '>'){
				cs = CRC_8BitsCompute(msg->data, msg->size - 1);
				if(cs == msg->data[msg->size - 1]){
					Serial_Parse((msg->data + 1), (msg->size - 2));
				}
			}
			
			/* Free data block mail box */
			osMailFree(SerialPkgMailBox, msg);
		}
		
		osThreadYield();
	}
}

static void SerialPkgBufferPush(uint8_t* data, uint16_t size){
	/* Copy data */
	Serial_Mail_Typedef *msg;
	
	msg = (Serial_Mail_Typedef*)osMailAlloc(SerialPkgMailBox, 1000);
	
	if(msg == NULL){
		SerialPushStr("Error: Full SerialPkgMailBox", END_LF);
		return;
	}
	if(size > SerialPkg_SIZE) size = SerialPkg_SIZE;
	
	msg->size = size;
	memcpy(msg->data, data, size);
	
	/* Put mail */
	osMailPut(SerialPkgMailBox, msg);
}

void SerialPushByte(uint8_t data){
	UART_SendByte(USART_COM, data);
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
	osSemaphoreRelease(sSerialFree);
}

void SerialPushBufferWithCS(uint8_t* data, uint16_t size){
	uint8_t cs = 0;
	cs = CRC_8BitsCompute(data, size);
	SerialPushBuffer(data, size);
	SerialPushByte(cs);
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

static char umsg[30];
void SerialPushInt(uint64_t val, ENDLINE type){
	sprintf(umsg, "%llu", val);
	SerialPushStr(umsg, type);
}

void UART_SendByte(uint32_t USARTx, uint8_t data){
	int32_t tick, delayPeriod;
	tick = osKernelSysTick(); // get start value of the Kernel system tick
	delayPeriod = osKernelSysTickMicroSec(50 * 1000);//50ms timeout
	
	/* Wait for USART Tx buffer empty or timeout*/
	while((usart_flag_get(USARTx, USART_FLAG_TBE) == RESET) && (osKernelSysTick() - tick) < delayPeriod);
	usart_data_transmit(USARTx,(uint8_t)data);
}

void PrintError(char* err){
	uint16_t len = strlen(err);
	uint16_t idx;
	for(idx = 0; idx < len; idx++)
		UART_SendByte(USART_COM, err[idx]);
}

void SerialUARTInit(void){
	rcu_periph_clock_enable(USART_COM_GPIO_CLK_TX);
	rcu_periph_clock_enable(USART_COM_GPIO_CLK_RX);
	rcu_periph_clock_enable(USART_COM_CLK);
	
	//connect port to USARTx Tx
	gpio_init(USART_COM_GPIO_TX, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,USART_COM_TxPin);
	
	//connect port to USARTx Rx
	gpio_init(USART_COM_GPIO_RX,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,USART_COM_RxPin);
	
	//USART configure
	usart_deinit(USART_COM);
	usart_baudrate_set(USART_COM, USART_BAUDRATE);
	usart_parity_config(USART_COM,USART_PM_NONE);
	usart_word_length_set(USART_COM, USART_WL_8BIT);
	usart_stop_bit_set(USART_COM, USART_STB_1BIT);
	usart_receive_config(USART_COM, USART_RECEIVE_ENABLE);
	usart_transmit_config(USART_COM, USART_TRANSMIT_ENABLE);
	usart_enable(USART_COM);
	
	nvic_irq_enable(USART_COM_IRQn, 0, 0);
	usart_interrupt_enable(USART_COM, USART_INT_RBNE);
}

void USART_COM_IRQHandler(void){
	static uint8_t rxd = 0;
	/* Check DR register in not empty */
	if(usart_interrupt_flag_get(USART0,USART_INT_FLAG_RBNE) != RESET){
		/* Receive the character */
		rxd =  usart_data_receive(USART0);
		if(SerialRxMsgBox != NULL)
			osMessagePut(SerialRxMsgBox, (uint32_t)rxd, 0);
	}
}


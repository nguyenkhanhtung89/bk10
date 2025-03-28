#include "ds18b20.h"

int16_t ds18b20_temp = -2000;

void threadDS18B20(const void * args);

osThreadId threadIDSENSOR;
osThreadDef(threadDS18B20, osPriorityAboveNormal, 1, 1024);

//--------------------------------------------------

//__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
//	micros *= (SystemCoreClock / 1000000) / 9;
//	/* Wait till done */
//	while (micros--) ;
//}

//--------------------------------------------------
/**
 * Initialises communication with the DS18B20 device.
 *
 * @param 	None.
 * @return 	1 if succeed, 0 if failed.
 */

uint32_t DWT_Delay_Init(void){
    /* Disable TRC */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
    /* Enable TRC */
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;
    /* Disable clock cycle counter */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
    /* Enable  clock cycle counter */
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;
    /* Reset the clock cycle counter value */
    DWT->CYCCNT = 0;
    /* 3 NO OPERATION instructions */
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    /* Check if clock cycle counter has started */
    if(DWT->CYCCNT){
       return 0; /*clock cycle counter started*/
    }else{
      return 1; /*clock cycle counter not started*/
    }
}
__STATIC_INLINE void DelayMicro(__IO uint32_t micros)
{
  uint32_t au32_initial_ticks = DWT->CYCCNT;
  uint32_t au32_ticks = (SystemCoreClock / 1000000);
  micros *= au32_ticks;
  while ((DWT->CYCCNT - au32_initial_ticks) < micros-au32_ticks);
}

uint8_t ds18b20_init_seq(void){
	GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_RESET);
	DelayMicro(600);
	GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_SET);
	DelayMicro(100);
	if (GPIO_ReadInputDataBit(TXRX_PORT, TXRX_PIN) == Bit_RESET) {
		DelayMicro(500);
		return 1;
	} else {
		DelayMicro(500);
		return 0;
	}
}

/**
 * Sends the read rom command.
 *
 * @param 	None.
 * @return 	The lasered rom code.
 */
uint64_t ds18b20_read_rom_cmd(void){
	uint8_t crc, family_code;
	uint64_t lasered_rom_code, serial_num;

	ds18b20_write_byte(READ_ROM_CMD_BYTE);

	family_code = ds18b20_read_byte();
	serial_num = ds18b20_read_byte();
	serial_num |= (ds18b20_read_byte() << 8);
	serial_num |= (ds18b20_read_byte() << 16);
	crc = ds18b20_read_byte();

	lasered_rom_code = (crc << 24) | (serial_num << 8) |  family_code;

	return lasered_rom_code;
}

/**
 * Sends the function command.
 *
 * @param 	cmd: Number of the function command
 * @return  None.
 */
void ds18b20_send_function_cmd(uint8_t cmd){
	ds18b20_write_byte(cmd);
	if (cmd == CONVERT_T_CMD) {
		while(GPIO_ReadInputDataBit(TXRX_PORT, TXRX_PIN) == Bit_RESET) {
		/* wait for end of conversion */
		}
	}
}

/**
 * Sends the rom command.
 *
 * @param 	cmd: Number of the rom command
 * @return  None.
 */
void ds18b20_send_rom_cmd(uint8_t cmd){
	ds18b20_write_byte(cmd);
}

static uint8_t check_buf_empty(u8 *in, u8 len){
	u8 i;
	for(i = 0; i < len; i++){
		if((in[i] != 0) && (in[i] != 0xFF)){
			return 1;
		}
	}
	return 0;
}

/**
 * Read the temperature.
 *
 * @param 	None.
 * @return  Float value of the last measured temperature.
 */

char test[64];
int ds18b20_read_temp(float *out){
	int32_t tick, delayPeriod;
	uint8_t i;
	uint16_t u16_temp;
	float temperature = 0.0;
	uint8_t data[9];
	uint8_t crc = 0;
	
	tick = osKernelSysTick(); // get start value of the Kernel system tick
	delayPeriod = osKernelSysTickMicroSec(1000 * 1000);//1s timeout
	
	while((!ds18b20_init_seq()) && ((osKernelSysTick() - tick) < delayPeriod));
	//step1 = ds18b20_init_seq();
	ds18b20_send_rom_cmd(SKIP_ROM_CMD_BYTE);
	ds18b20_send_function_cmd(CONVERT_T_CMD);
	
	DelayMicro(100);
	
	//step2 = ds18b20_init_seq();
	while((!ds18b20_init_seq()) && ((osKernelSysTick() - tick) < delayPeriod));
	
	ds18b20_send_rom_cmd(SKIP_ROM_CMD_BYTE);
	ds18b20_send_function_cmd(READ_SCRATCHPAD_CMD);
	
	DelayMicro(100);
	
	for(i = 0; i < 9; i++){
		data[i] = ds18b20_read_byte();
	}
	
	if(check_buf_empty(data, 9) == 0){
		//SerialPushStr("DS18B20 -> READ TIME OUT", END_LF);
		return -2;
	}
	
	crc = dsCRC8(data,8);
	if((crc != data[8])){
		//SerialPushStr("DS18B20 -> READ ERROR", END_LF);
		return -1;
	}
	
	u16_temp = ((data[1] << 8) | data[0]);

	if((u16_temp & 0xF800) == 0xF800){//check nhiet do am: 5 bit sign = 1 
    u16_temp = ~u16_temp;
    u16_temp = u16_temp +1 ;
    temperature = u16_temp *(-0.0625);
  }else{
    temperature = u16_temp*0.0625;  
  }
	
	if((temperature == (float)85.000) || (temperature == (float)127.000)){
		//SerialPushStr("DS18B20 -> ERROR STARTUP 85 DEG", END_LF);
		return -3;
	}
//	sprintf(test, "READ OK TEMP = %.2f", temperature); 
//	SerialPushStr(test, END_LF);
	*out = temperature;
	
	return 0;
}

/**
 * Write byte to DS18B20.
 *
 * @param 	data: Data to be written.
 * @return  None.
 */
void ds18b20_write_byte(uint8_t data){
	uint8_t i;
  uint8_t 	mask = 0x01;
	uint8_t data_bit = data & mask;
	for (i = 0; i < 8; i++) {
		if (data_bit) {
			GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_RESET);
			DelayMicro(3);
			GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_SET);
			DelayMicro(90);
		} else {
			GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_RESET);
			DelayMicro(90);
			GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_SET);
			DelayMicro(3);
		}
		mask = mask << 1;
		data_bit = data & mask;
	}
}

/**
 * Read one bit from DS18B20.
 *
 * @param 	None.
 * @return  1 if succeed, 0 if failed.
 */
uint8_t ds18b20_read_bit(void){
	GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_RESET);
	DelayMicro(2);
	GPIO_WriteBit(TXRX_PORT, TXRX_PIN, Bit_SET);
	DelayMicro(10);
	if(GPIO_ReadInputDataBit(TXRX_PORT, TXRX_PIN) == Bit_RESET) {
		DelayMicro(58);
		return 0;
	} else {
		DelayMicro(58);
		return 1;
	}
}

/**
 * Read one byte from DS18B20.
 *
 * @param 	None.
 * @return  One byte of data read from DS18B20.
 */
uint8_t ds18b20_read_byte(void){
	uint8_t i, data = 0;
	for (i = 0; i < 8; i++)
		data |= (ds18b20_read_bit() << i);
	return data;
}

uint8_t dsCRC8(const uint8_t *addr, uint8_t len){//begins from LS-bit of LS-byte
  uint8_t crc = 0;
	uint8_t i;
  while (len--){
    uint8_t inbyte = *addr++;
    for ( i = 8; i ; i--){
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) {
				crc ^= 0x8C;
			}
      inbyte >>= 1;
    }
  }
  return crc;
}


void threadDS18B20(const void * args){
	static u16 temp_timeout = 0;
  float temp = -200;
	int res;
	DWT_Delay_Init();
  for(;;){
		res = ds18b20_read_temp(&temp);
		if(!res){
			ds18b20_temp = (temp * 10);
			temp_timeout = 0;
		}else if(res == -2){
			temp_timeout++;
			if(temp_timeout > 60){
				ds18b20_temp = -2000;
			}
		}
    osDelay(3000);
		osThreadYield();
  }
}

void DS18B20_Init(){
	config_GPIO();
	
  threadIDSENSOR = osThreadCreate(osThread(threadDS18B20), NULL);
  if (!threadIDSENSOR) return;
}

void config_GPIO(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	
	GPIO_WriteBit(GPIOB,GPIO_Pin_7,Bit_RESET);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


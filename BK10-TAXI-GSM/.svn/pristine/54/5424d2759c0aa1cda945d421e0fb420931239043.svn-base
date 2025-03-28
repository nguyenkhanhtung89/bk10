#include "ds18b20.h"

int16_t ds18b20_temp = -2000;

void threadDS18B20(const void * args);

osThreadId threadIDSENSOR;
osThreadDef(threadDS18B20, osPriorityAboveNormal, 1, 1024);

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
  uint32_t auint32_t_initial_ticks = DWT->CYCCNT;
  uint32_t auint32_t_ticks = (SystemCoreClock / 1000000);
  micros *= auint32_t_ticks;
  while ((DWT->CYCCNT - auint32_t_initial_ticks) < (micros - auint32_t_ticks));
}

uint8_t ds18b20_init_seq(void){
	gpio_bit_write(TXRX_PORT, TXRX_PIN, RESET);
	DelayMicro(600);
	gpio_bit_write(TXRX_PORT, TXRX_PIN, SET);
	DelayMicro(100);
	if (gpio_input_bit_get(TXRX_PORT, TXRX_PIN) == RESET) {
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
		while(gpio_input_bit_get(TXRX_PORT, TXRX_PIN) == RESET) {
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

static uint8_t check_buf_empty(uint8_t *in, uint8_t len){
	uint8_t i;
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
	uint16_t uint16_t_temp;
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
	
	uint16_t_temp = ((data[1] << 8) | data[0]);

	if((uint16_t_temp & 0xF800) == 0xF800){//check nhiet do am: 5 bit sign = 1 
    uint16_t_temp = ~uint16_t_temp;
    uint16_t_temp = uint16_t_temp +1 ;
    temperature = uint16_t_temp *(-0.0625);
  }else{
    temperature = uint16_t_temp*0.0625;  
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
			gpio_bit_write(TXRX_PORT, TXRX_PIN, RESET);
			DelayMicro(3);
			gpio_bit_write(TXRX_PORT, TXRX_PIN, SET);
			DelayMicro(90);
		} else {
			gpio_bit_write(TXRX_PORT, TXRX_PIN, RESET);
			DelayMicro(90);
			gpio_bit_write(TXRX_PORT, TXRX_PIN, SET);
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
	gpio_bit_write(TXRX_PORT, TXRX_PIN, RESET);
	DelayMicro(2);
	gpio_bit_write(TXRX_PORT, TXRX_PIN, SET);
	DelayMicro(10);
	if(gpio_input_bit_get(TXRX_PORT, TXRX_PIN) == RESET) {
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
	static uint16_t temp_timeout = 0;
  float temp = -200;
	int res;
	char *buf;
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
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
	gpio_bit_set(GPIOB, GPIO_PIN_7);
}


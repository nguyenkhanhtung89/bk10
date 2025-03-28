#include "power.h"
#include "utils.h"
#include "flash.h"

ADC_def ADCData;
power_def PowerData;
static __IO PulseDef PulseData;

static void PW_Control_init(void);
static void get_adc_values(void);
static void ADC_Initialize(void);
static void FuelCyclesConfig(void);
static void PulseTimerInit(void);
void vPowerTask(const void* args);

osThreadId power_thread;
osThreadDef(vPowerTask, osPriorityNormal, 1, 1024);

void Power_Init(void){
	power_thread = osThreadCreate(osThread(vPowerTask), NULL);
  if(!power_thread) return;
	
	/* init adc function */
	ADC_Initialize();
	
	/* init power services */
	PW_Control_init();
	
	if((mcu_config.TypePulseADC == PULSE_MODE) || (mcu_config.TypePulseADC == CYCLE_MODE)){
		PulseTimerInit();
	
		FuelCyclesConfig();
	}
}

uint8_t EC21_PW_reboot_time = 0;
uint8_t Device_BAT_off_time = 0;
void vPowerTask(const void* args){
	if (FLASH_ReadData(FLG_UPDATE_DONE_ADDR) == 1) {
    FLASH_WriteData(FLG_UPDATE_DONE_ADDR, (u32)0x00, 0);
  }
	
	for(;;){
		/* get values of adc dma */
		get_adc_values();
		/* Read power mode */
		if(ADCData.main_power_val > (LOW_EXT_POWER_LEVEL + 50)){
			PowerData.ext_power_l = 0;
			if(PowerData.power_mode == POWER_SAVE){
				if(PowerData.ext_power_h ++ >= 5){
					PowerData.power_mode = POWER_BALANCE;
					PowerData.ext_power_l = 0;
					PowerData.ext_power_h = 0;
					SerialPushStr("CO NGUON NGOAI", END_LF);
				}
			}
			GPIO_SetBits(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
		}else if(ADCData.main_power_val < LOW_EXT_POWER_LEVEL){
			PowerData.ext_power_h = 0;
			if(PowerData.power_mode == POWER_BALANCE){
				if(PowerData.ext_power_l++ >= 3){
					PowerData.power_mode = POWER_SAVE;
					PowerData.ext_power_l = 0;
					PowerData.ext_power_h = 0;
					/* lcd, buzzer and led off */
					SerialPushStr("MAT NGUON NGOAI", END_LF);
				}
			}else if(ADCData.battery_val < LOW_BATTERY_LEVEL){
				/* device power off */
				if(PowerData.low_bat_cnt++ > 10){
					SerialPushStr("NGUON PIN DANG YEU -> TAT THIET BI", END_LF);
					osDelay(3000);
					GPIO_ResetBits(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
				}
			}
			GPIO_ResetBits(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
		}
		
		if(EC21_PW_reboot_time != 0){
			EC21_PW_reboot_time --;
			if(EC21_PW_reboot_time == 0){
				SerialPushStr("<<<<<<- EC21 PW RESET ->>>>>>", END_LF);
        osDelay(1000);
				osDelay(1000);
				osDelay(1000);
				EC21_PW_Reset();
				osDelay(1000);
				osDelay(1000);
				osDelay(1000);
				NVIC_SystemReset();
			}
		}
		
		if(Device_BAT_off_time != 0){
			Device_BAT_off_time --;
			if(Device_BAT_off_time == 0){
				SerialPushStr("<<<<<<- BATTERY OFF ->>>>>>", END_LF);
        osDelay(1000);
				osDelay(1000);
				osDelay(1000);
				GPIO_ResetBits(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
			}
		}
		osDelay(1000);
    osThreadYield();
	}
}

void EC21_PW_Reset(void) {
	int i = 0;
	//power off EC21
  GPIO_SetBits(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
	//wait 10 seconds
  for(i = 0; i < 10; i++){
		osDelay(1000);
	}
	//power on EC21
  GPIO_ResetBits(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
}

/************************************************ADC**************************************************************/
static void PW_Control_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* RCC init */
	RCC_AHBPeriphClockCmd(PWR_EC21_GPIO_CLK | PWR_BATTERY_GPIO_CLK | PWR_BOST_GPIO_CLK | PWR_MAIN_GPIO_GPIO_CLK | PWR_5VOUT_GPIO_CLK | PWR_RFID_GPIO_GPIO_CLK, ENABLE);
	
	/* CFG output mode */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	/* Battery controls */
	GPIO_InitStructure.GPIO_Pin   = PWR_BATTERY_GPIO_PIN;
	GPIO_Init(PWR_BATTERY_GPIO_PORT, &GPIO_InitStructure);
	/* power on battery */
	GPIO_SetBits(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
	
	/* Power EC21 controls */
	GPIO_InitStructure.GPIO_Pin   = PWR_EC21_GPIO_PIN;
	GPIO_Init(PWR_EC21_GPIO_PORT, &GPIO_InitStructure);
	/* power on EC21 */
	GPIO_ResetBits(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
	
	/* Power BOST controls */
	GPIO_InitStructure.GPIO_Pin   = PWR_BOST_GPIO_PIN;
	GPIO_Init(PWR_BOST_GPIO_PORT, &GPIO_InitStructure);
	/* power on bost */
	GPIO_ResetBits(PWR_BOST_GPIO_PORT, PWR_BOST_GPIO_PIN);
	
	/* Power MAIN controls */
	GPIO_InitStructure.GPIO_Pin   = PWR_MAIN_GPIO_PIN;
	GPIO_Init(PWR_MAIN_GPIO_GPIO_PORT, &GPIO_InitStructure);
	/* power on 12vol */
	GPIO_ResetBits(PWR_MAIN_GPIO_GPIO_PORT, PWR_MAIN_GPIO_PIN);
	
	/* Power 5V-OUT controls */
	GPIO_InitStructure.GPIO_Pin   = PWR_5VOUT_GPIO_PIN;
	GPIO_Init(PWR_5VOUT_GPIO_PORT, &GPIO_InitStructure);
	/* power off 5V-OUT = PW ON CAMERA*/
	GPIO_ResetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
	
	/* Power RFID controls */
	GPIO_InitStructure.GPIO_Pin   = PWR_RFID_GPIO_PIN;
	GPIO_Init(PWR_RFID_GPIO_GPIO_PORT, &GPIO_InitStructure);
	/* power on RFID */
	GPIO_SetBits(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
}

static void ADC_Initialize(void){
	DMA_InitTypeDef  DMA_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	/* Configure the ADC clock */
  RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2);
	
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
  /* Enable ADC1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	
  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOC, ENABLE);
	
	/* ADC GPIO config */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* DMA1 channel1 configuration */
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCData.adc_array;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_DMA_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	/* ADC1 configuration*/
	ADC_StructInit(&ADC_InitStructure);
	
  /* Calibration procedure */  
  ADC_VoltageRegulatorCmd(ADC1, ENABLE);
	osDelay(10);
	
  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Differential);
  ADC_StartCalibration(ADC1);
	
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1) != RESET);
	
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Interleave;
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 10;
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
	
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;  
  ADC_InitStructure.ADC_NbrOfRegChannel = ADC_DMA_SIZE;
  ADC_Init(ADC1, &ADC_InitStructure);
	
  /* ADC1 regular channels configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_7Cycles5);  //PC0 chanel 6    -> V12-IN
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 2, ADC_SampleTime_7Cycles5);  //PC1 chanel 7    -> V5.6-IN
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_7Cycles5);  //PC3 chanel 9    -> VBATTERY-IN
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 4, ADC_SampleTime_7Cycles5);  //PA0 chanel 1    -> ADC in
	
	/* Configures the ADC DMA */
  ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);
  
  /* Enable DMA Stream Transfer Complete interrupt */
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
	
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
	
  /* wait for ADRDY */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	
  /* Enable DMA1 Channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
	
  /* Start ADC1 Software Conversion */ 
  ADC_StartConversion(ADC1);
}

void DMA1_Channel1_IRQHandler(void){
	/* DMA Transfer Complete with interrupt */
  if(DMA_GetITStatus(DMA1_IT_TC1)){
    /* Clear DMA Transfer Complete interrupt pending bit */
    DMA_ClearITPendingBit(DMA1_IT_TC1);
  }
}

static uint16_t adc_values_[70];
static uint16_t ADC_ReadFuelValue(uint8_t N , uint8_t X, uint8_t POS){
	uint32_t avg_value = 0;
	uint8_t index = 0;
	
	memset(adc_values_, 0, 64);
	for(index = 0; index < N; index++) {
		/* Wait end of conversion */
		while(DMA_GetITStatus(DMA1_IT_TC1) == RESET);
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		/* Store ADC samples */
		adc_values_[index] = ADCData.adc_array[POS];
	}
	
	/* Sort the N-X ADC samples */
	ArraySort(adc_values_,N);
	
	/* Add the N ADC samples */
	for(index = X/2; index < N-X/2; index++) {
		avg_value += adc_values_[index];
	}
	
	/* Compute the average of N-X ADC sample */
	avg_value /= N-X;
	
	/* Return average value */
	return avg_value;
}

static uint16_t BAT_FilterArr[32];
static uint8_t BAT_FilterIdx = 0;
static void BAT_fillter_values(int32_t input, uint16_t * val) {
  uint32_t avg_val = 0;
  uint8_t idx = 0;
  uint8_t cnt = 0;
	
  BAT_FilterArr[BAT_FilterIdx] = input;
  BAT_FilterIdx++;
  if (BAT_FilterIdx >= 16) {
    BAT_FilterIdx = 0;
  }
	
  /* Sort the N-X array values */
  ArraySort(BAT_FilterArr, 16);
  for (idx = 6; idx < 10; idx++) {
    if (BAT_FilterArr[idx] > 0) {
      cnt++;
      avg_val += BAT_FilterArr[idx];
    }
  }
	/* calculator result */
  if ((avg_val / cnt) > 0) {
    *val = avg_val / cnt;
  }else{
		*val = input;
	}
}

static uint16_t ADC_FilterArr[32];
static uint8_t ADC_FilterIdx = 0;
static void ADC_fillter_values(int32_t input, uint16_t * val) {
	uint32_t avg_val = 0;
	uint8_t idx = 0;
	uint8_t cnt = 0;
	
	ADC_FilterArr[ADC_FilterIdx] = input;
	ADC_FilterIdx++;
	if (ADC_FilterIdx >= 16) {
		ADC_FilterIdx = 0;
	}
	
	/* Sort the N-X array values */
	ArraySort(ADC_FilterArr, 16);
	for (idx = 6; idx < 10; idx++) {
		if (ADC_FilterArr[idx] > 0) {
			cnt++;
			avg_val += ADC_FilterArr[idx];
		}
	}
	/* calculator result */
	if ((avg_val / cnt) > 0) {
		*val = avg_val / cnt;
	}else{
		*val = input;
	}
}

char test_val[128];
u8 pw_cnt = 0;
static void get_adc_values(void){
	uint16_t dig_val = 0;
	uint16_t f_dig_val = 0;
	uint16_t raw_voltage_val = 0;
	
	/* get main power milivoltage value */
	dig_val = ADC_ReadFuelValue(64, 20, 0);
	ADCData.main_power_val = ((((dig_val * 3300) / 0xFFF) * (V12_R1 + V12_R2)) / V12_R2);
	ADCData.main_power_val += (ADCData.main_power_val * 14 / 100);
	ADCData.main_power_val += 700;
	
	/* get main power milivoltage value */
	ADCData.v_5v6_val = ((((ADCData.adc_array[1] * 3300) / 0xFFF) * (V5V6_R1 + V5V6_R2)) / V5V6_R2);
	
	/* get battery milivoltage value */
	dig_val = ADC_ReadFuelValue(64, 20, 2);
	BAT_fillter_values(dig_val, &f_dig_val);
	raw_voltage_val = (f_dig_val * 3300) / 0xFFF;
	ADCData.battery_val = ((raw_voltage_val * (VBAT_R1 + VBAT_R2)) / VBAT_R2);
	ADCData.battery_val += ((ADCData.battery_val * 3) / 100);
	
	/* get (ADC-5V) milivoltage value */
	if(mcu_config.TypePulseADC == ADC_MODE){
		dig_val = ADC_ReadFuelValue(64, 20, 3);
		ADC_fillter_values(dig_val, &f_dig_val);
		raw_voltage_val = (f_dig_val * 3300) / 0xFFF;
		ADCData.adc_pulse_val = ((raw_voltage_val * (VADC_R1 + VADC_R2)) / VADC_R2);
	}else if(mcu_config.TypePulseADC == PULSE_MODE){
		ADCData.adc_pulse_val = PulseData.FuelPulseValues;
	}else if(mcu_config.TypePulseADC == CYCLE_MODE){
		ADCData.adc_pulse_val = PulseData.FuelCyclesValues;
	}
	
//	if(pw_cnt++ % 5 == 0){
//		sprintf(test_val, "12Vin = %d, bat = %d, adc = %d, 5v6 = %d, ACC: %d - DOOR: %d - AIR: %d", ADCData.main_power_val,  ADCData.battery_val, ADCData.adc_pulse_val, ADCData.v_5v6_val, ACC_STATUS, DOOR_STATUS, AIR_STATUS);
//		SerialPushStr(test_val, END_LF);
//	}
}

static void FuelCyclesConfig(void){
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	
  /* Enable GPIOC clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* Configure  pin as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
		/* Connect EXTI0 Line to PA.0 pin */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, GPIO_PinSource0);
	
  /* Configure EXTI8 line */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0 ;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	
	if(mcu_config.TypePulseADC == CYCLE_MODE){
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	}else{
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	}
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
  /* Enable and set EXTI0 Interrupt*/
	NVIC_EnableIRQ(EXTI0_IRQn);
}

static void PulseTimerInit(void){
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* Enable TIM3 */ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	/* Enable Timer 3 interupt */
	NVIC_EnableIRQ(TIM3_IRQn);
	/* --------------------------------------------------------------------------
	* TIMER3 configuration
	* VL clocks at 72 MHz, change to 24 - 1 for a 24 MHz system
	* --------------------------------------------------------------------------*/
	TIM_TimeBaseStructure.TIM_Period 					= 36050;
	TIM_TimeBaseStructure.TIM_Prescaler 			= 1;
	TIM_TimeBaseStructure.TIM_ClockDivision 	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);	
	TIM_UpdateRequestConfig(TIM3, TIM_UpdateSource_Global);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	/* Enable TIMER Update interrupt */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	/* TIMER enable counter */
	TIM_Cmd(TIM3, ENABLE);
}

/* Pulse cycles interrupt */
void EXTI0_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line0) != RESET){
		PulseData.FuelPulseTimeOut = 0;
		if(mcu_config.TypePulseADC == PULSE_MODE){
			PulseData.Freq_pulse ++;
		}else if (mcu_config.TypePulseADC == CYCLE_MODE){
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){
				PulseData.FallingTime = PulseData.FuelCyclesTime;
				PulseData.FuelCyclesTime = 0;
			}else{
				PulseData.RisingTime = PulseData.FuelCyclesTime;
				PulseData.FuelCyclesTime = 0;
				PulseData.FuelCyclesValues = ((PulseData.RisingTime * 1100) / (PulseData.RisingTime + PulseData.FallingTime)) - 25;
			}
			if(PulseData.FuelCyclesValues < 0)PulseData.FuelCyclesValues = 0;
		}
		
		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}

/* Timer for fuel cycles and pulse */
void TIM3_IRQHandler(void){
  if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
		PulseData.FuelCyclesTime++;
		if(PulseData.FuelCyclesTime > 60000){
			PulseData.FuelCyclesTime = 0;
			PulseData.FuelCyclesValues = 0;
		}
		
		PulseData.FuelPulseTimeOut++;
		if(PulseData.FuelPulseTimeOut > 15000){
			PulseData.FuelPulseTimeOut = 0;
			PulseData.FuelCyclesValues = 0;
			PulseData.FuelPulseValues = 0;
		}
		
		PulseData.TM3_Counter++;
		if(PulseData.TM3_Counter >= 999){
			PulseData.FuelPulseValues = PulseData.Freq_pulse;
			PulseData.Freq_pulse = 0;
			PulseData.TM3_Counter = 0;
		}
  }
}

void ec21_power_rst(uint8_t time){
	EC21_PW_reboot_time = time;
}

void device_battery_off(uint8_t time){
	Device_BAT_off_time = time;
}

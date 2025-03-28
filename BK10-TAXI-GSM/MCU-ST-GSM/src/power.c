#include "power.h"
#include "utils.h"
#include "flash.h"

ADC_def ADCData;
power_def PowerData;
static __IO PulseDef PulseData;

SensorIRDef sensorIR;

static void PW_Control_init(void);
static void get_adc_values(void);
static void ADC_Initialize(void);
static void FuelCyclesConfig(void);
static void PulseTimerInit(void);
static void InitVS1838B(void);
void vPowerTask(const void* args);

osThreadId power_thread;
osThreadDef(vPowerTask, osPriorityNormal, 1, 1024);

void vIRLedTask(const void* args);

osThreadId IRLed_thread;
osThreadDef(vIRLedTask, osPriorityNormal, 1, 512);

void Power_Init(void){
	power_thread = osThreadCreate(osThread(vPowerTask), NULL);
  if(!power_thread) return;
	
	IRLed_thread = osThreadCreate(osThread(vIRLedTask), NULL);
  if(!IRLed_thread) return;	
//	
	/* init adc function */
	ADC_Initialize();
	
	/* init power services */
	PW_Control_init();
	if(mcu_config.TaxiMode == 0){
		if((mcu_config.TypePulseADC == PULSE_MODE) || (mcu_config.TypePulseADC == CYCLE_MODE)){
			PulseTimerInit();
		
			FuelCyclesConfig();
		}
	}else{
		InitVS1838B();
	}
}

static void BlinkIRLed(void){
	TIM_Cmd(TIM2, ENABLE); osDelay(9);
	TIM_Cmd(TIM2, DISABLE); osDelay(4);
	
	TIM_Cmd(TIM2, ENABLE);	osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(1);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE); osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE); osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);

	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(2);
	TIM_Cmd(TIM2, ENABLE);osDelay(1);
	TIM_Cmd(TIM2, DISABLE);osDelay(1);
	
	osDelay(30);
}

uint16_t counter_ir_1 = 0;
uint16_t counter_ir_2 = 0;
uint8_t time_check_sensor = 30;
void vIRLedTask(const void* args){
	uint8_t index = 0;
	uint8_t counterCheck1, counterCheck2, counterCheck3, counterCheck4;
	for(;;){
		
		if(canbus.isOBD_VF8 == 1){
			if((mcu_config.LedMode == 0) || (time_check_sensor != 0)){//0 auto, 1 alway hirring, 2 alway no hirring
				if(sensorIR.status_seat == 1){//co khach
					if((ec21_data.disable_release_button == 0) && (time_check_sensor <= 1)){//enable slience mode -> change to disable release button
						mcu_config.LedMode = 1;
					}
					mcu_config.LedStatus = 1;	
					Mode_disable_OBD = 0;
				}else{//khong khach
					mcu_config.LedStatus = 0;
				}
			}else if(mcu_config.LedMode == 1){//luon bao co khach
				mcu_config.LedStatus = 1;
			}		
			
			osDelay(1000);
			continue;
		}
		
		sensorIR.countFallingIR1 = 0;
		index = 3;
		while(index -- > 0){
			BlinkIRLed();
		}
		
		if((sensorIR.countFallingIR1 >= 90) && (sensorIR.countFallingIR1 <= 110)){
			counterCheck2 = 0;
			counterCheck1 ++;
			if(counterCheck1 >= 2){
				counterCheck1 = 0;
				sensorIR.statusIR1 = 0;
			}
		}else if(sensorIR.countFallingIR1 < 30){
			counterCheck1 = 0;
			counterCheck2++;
			if(counterCheck2 >= 2){
				counterCheck2 = 0;
				sensorIR.statusIR1 = 1;
			}
		}
		counter_ir_1 = sensorIR.countFallingIR1;
		
		osDelay(200);
		sensorIR.countFallingIR2 = 0;
		index = 4;
		while(index -- > 0){
			BlinkIRLed();
		}

		if((sensorIR.countFallingIR2 >= 120) && (sensorIR.countFallingIR2 <= 145)){
			counterCheck4 = 0;
			counterCheck3++;
			if(counterCheck3 >= 2){
				counterCheck3 = 0;
				sensorIR.statusIR2 = 0;
			}
		}else if(sensorIR.countFallingIR2 < 30){
			counterCheck3 = 0;
			counterCheck4++;
			if(counterCheck4 >= 2){
				counterCheck4 = 0;
				sensorIR.statusIR2 = 1;
			}
		}
		counter_ir_2 = sensorIR.countFallingIR2;
		
		osDelay(200);
		
		if((mcu_config.TypeOBD == VFE34_TYPE) || (mcu_config.TypeOBD == VF8_TYPE)){//VF8
			if((mcu_config.LedMode == 0) || (time_check_sensor != 0)){//0 auto, 1 alway hiring, 2 alway no hiring
				if((sensorIR.statusIR1 == 1) || (sensorIR.statusIR2 == 1)){//co khach
					
					if((ec21_data.disable_release_button == 0) && (time_check_sensor <= 1)){//enable slience mode -> change to disable release button
						if(mcu_config.use_release_button == 1){
							mcu_config.LedMode = 1;
						}
					}

					mcu_config.LedStatus = 1;	
					Mode_disable_OBD = 0;
				}else{//khong khach
					mcu_config.LedStatus = 0;
				}
			}else if(mcu_config.LedMode == 1){//luon bao co khach
				mcu_config.LedStatus = 1;
			}
		}
		if(Ir_test_mode == 1){
			if(sensorIR.statusIR1 == 0){
				Buzz_On();
			}else{
				Buzz_Off();
			}
		}
	}
}

static void InitPWM(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	/*Enable or disable the AHB peripheral clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	/*Configure GPIO pin */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	/* Connect TIM4 pins to AF2 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_1);
	// Enable the TIM2 peripherie
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	// Setup the timing and configure the TIM1 timer
	
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	TIM_TimeBaseStructure.TIM_Period = 472;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	// Initialise the timer channels
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 236; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCNPolarity_Low; 
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Set;
	// Setup four channels
	// Channel 1, pin PA0?
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	// Starup the timer
	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}

void InitVS1838B(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RCC_AHBPeriphClockCmd(PWR_5VOUT_GPIO_CLK | TRAN_SENSOR_GPIO_CLK, ENABLE);
	
	/* CFG output mode */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin   = PWR_5VOUT_GPIO_PIN;
	GPIO_Init(PWR_5VOUT_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
	
	GPIO_InitStructure.GPIO_Pin   = TRAN_SENSOR_GPIO_PIN;
	GPIO_Init(TRAN_SENSOR_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(TRAN_SENSOR_GPIO_PORT, TRAN_SENSOR_GPIO_PIN);
	
  /* Enable GPIOC clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  
  /* Configure  pin as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, GPIO_PinSource13);
	
  EXTI_InitStructure.EXTI_Line = EXTI_Line13 ;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	
	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	
	 /* Enable GPIOC clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  
  /* Configure  pin as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, GPIO_PinSource14);
	
  EXTI_InitStructure.EXTI_Line = EXTI_Line14 ;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	
	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	InitPWM();
}

uint8_t EC21_PW_reboot_time = 0;
uint8_t Device_BAT_off_time = 0;
u8 flg_pw_in = 0;
u8 flg_pw_h = 0;
u8 flg_pw_l = 0;
u8 rst_condition = 0;
void vPowerTask(const void* args){
	if (FLASH_ReadData(FLG_UPDATE_DONE_ADDR) == 1) {
    FLASH_WriteData(FLG_UPDATE_DONE_ADDR, (u32)0x00, 0);
  }
	
	for(;;){
		/* get values of adc dma */
		get_adc_values();

		rst_condition++;
		if(rst_condition > 20){
			rst_condition = 0;
			flg_pw_in = 0;
			flg_pw_h = 0;
			flg_pw_l = 0;
		}
		
		/* Read power mode */
		if(ADCData.main_power_val > (LOW_EXT_POWER_LEVEL + 50)){
			flg_pw_in = 1;
			if(flg_pw_l == 1){
				flg_pw_h = 1;
			}	
			
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
			rst_condition = 0;
			if(flg_pw_in == 1){
				flg_pw_l = 1;
			}
			
			if((flg_pw_in == 1) && (flg_pw_h == 1) && (flg_pw_l == 1)){
				//power off battery
				GPIO_ResetBits(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
			}
			
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
	/* power off RFID */
	GPIO_ResetBits(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
	//wait 10 seconds
  for(i = 0; i < 10; i++){
		osDelay(1000);
	}
	//power on EC21
  GPIO_ResetBits(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
	/* power on RFID */
	GPIO_SetBits(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
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
  
	if(mcu_config.TaxiMode == 0){
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	
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
	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 4, ADC_SampleTime_7Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 5, ADC_SampleTime_7Cycles5);  //PA0 chanel 1    -> ADC in
	
	ADC_TempSensorCmd(ADC1, ENABLE);
	
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
	
	dig_val = ADC_ReadFuelValue(64, 20, 3);
	ADCData.temperature = (1.43 - dig_val*3.3/4096) * 1000 / 4.3 + 25;
	
	if(mcu_config.TaxiMode == 0){
		/* get (ADC-5V) milivoltage value */
		if(mcu_config.TypePulseADC == ADC_MODE){
			dig_val = ADC_ReadFuelValue(64, 20, 4);
			ADC_fillter_values(dig_val, &f_dig_val);
			raw_voltage_val = (f_dig_val * 3300) / 0xFFF;
			ADCData.adc_pulse_val = ((raw_voltage_val * (VADC_R1 + VADC_R2)) / VADC_R2);
		}else if(mcu_config.TypePulseADC == PULSE_MODE){
			ADCData.adc_pulse_val = PulseData.FuelPulseValues;
		}else if(mcu_config.TypePulseADC == CYCLE_MODE){
			ADCData.adc_pulse_val = PulseData.FuelCyclesValues;
		}
	}
	
//	if(pw_cnt++ % 1 == 0){
//		sprintf(test_val, "12Vin = %d, bat = %d, adc = %d, 5v6 = %d, Temp = %d", ADCData.main_power_val,  ADCData.battery_val, ADCData.adc_pulse_val, ADCData.v_5v6_val, ADCData.temperature);
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
	TIM_TimeBaseStructure.TIM_Period 					= 35999;
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

void EXTI15_10_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line13) != RESET){
		sensorIR.countFallingIR1++;
		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
	
	if(EXTI_GetITStatus(EXTI_Line14) != RESET){
		sensorIR.countFallingIR2++;
		/* Clear the EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
}

uint8_t ctltran = 0 ;
uint16_t index1 = 0;
uint8_t index2 = 0;
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

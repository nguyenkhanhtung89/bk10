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

uint16_t adc_channel_sample(uint8_t channel){
	/* ADC regular channel config */
	adc_regular_channel_config(ADC0, 0U, channel, ADC_SAMPLETIME_7POINT5);
	/* ADC software trigger enable */
	adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);

	/* wait the end of conversion flag */
	while(!adc_flag_get(ADC0, ADC_FLAG_EOC));
	/* clear the end of conversion flag */
	adc_flag_clear(ADC0, ADC_FLAG_EOC);
	/* return regular channel sample value */
	return (adc_regular_data_read(ADC0)); 
}
	
void vPowerTask(const void* args);

osThreadId power_thread;
osThreadDef(vPowerTask, osPriorityNormal, 1, 1024);

void vIRLedTask(const void* args);

osThreadId IRLed_thread;
osThreadDef(vIRLedTask, osPriorityNormal, 1, 256);

void Power_Init(void){
	power_thread = osThreadCreate(osThread(vPowerTask), NULL);
  if(!power_thread) return;	
	
	IRLed_thread = osThreadCreate(osThread(vIRLedTask), NULL);
  if(!IRLed_thread) return;	
	
	/* init adc function */
	ADC_Initialize();
	
	/* init power services */
	PW_Control_init();

	InitVS1838B();
}

static void BlinkIRLed(void){
	timer_enable(TIMER1); osDelay(9);
	timer_disable(TIMER1); osDelay(4);
	
	timer_enable(TIMER1);	osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(2);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(2);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(2);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(2);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(1);
	timer_enable(TIMER1); osDelay(1);
	timer_disable(TIMER1); osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);

	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(2);
	timer_enable(TIMER1);osDelay(1);
	timer_disable(TIMER1);osDelay(1);
	
	osDelay(30);
}
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
		}else if(sensorIR.countFallingIR1 < 15){
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
		}else if(sensorIR.countFallingIR2 < 15){
			counterCheck3 = 0;
			counterCheck4++;
			if(counterCheck4 >= 2){
				counterCheck4 = 0;
				sensorIR.statusIR2 = 1;
			}
		}
		counter_ir_2 = sensorIR.countFallingIR2;
		
		osDelay(200);
		
		if((mcu_config.LedMode == 0) || (time_check_sensor != 0)){//0 auto, 1 alway hiring, 2 alway no hiring
			if((sensorIR.statusIR1 == 1) || (sensorIR.statusIR2 == 1)){//co khach
				
				if((ec21_data.disable_release_button == 0) && (time_check_sensor <= 1)){//enable slience mode -> change to disable release button
					if(mcu_config.use_release_button == 1){
						mcu_config.LedMode = 1;
					}
				}

				mcu_config.LedStatus = 1;	
			}else{//khong khach
				mcu_config.LedStatus = 0;
			}
		}else if(mcu_config.LedMode == 1){//luon bao co khach
			mcu_config.LedStatus = 1;
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
	rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);

    /*Configure PA0 PA1 PA2(TIMER1 CH0 CH1 CH2) as alternate function*/
    gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_0);
		
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);

    timer_deinit(TIMER1);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1578;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER1,TIMER_CH_0,&timer_ocintpara);
//    timer_channel_output_config(TIMER1,TIMER_CH_1,&timer_ocintpara);
//    timer_channel_output_config(TIMER1,TIMER_CH_2,&timer_ocintpara);

    /* CH0 configuration in PWM mode0,duty cycle 25% */
    timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,789);
    timer_channel_output_mode_config(TIMER1,TIMER_CH_0,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1,TIMER_CH_0,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);
    /* auto-reload preload enable */
}

void InitVS1838B(void){
	rcu_periph_clock_enable(PWR_5VOUT_GPIO_CLK);
	rcu_periph_clock_enable(TRAN_SENSOR_GPIO_CLK);
	
	/* Power 5V-OUT controls */
	gpio_init(PWR_5VOUT_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_5VOUT_GPIO_PIN);
	/* power off 5V-OUT */
	gpio_bit_reset(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
	
	gpio_init(TRAN_SENSOR_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,TRAN_SENSOR_GPIO_PIN);
	gpio_bit_reset(TRAN_SENSOR_GPIO_PORT, TRAN_SENSOR_GPIO_PIN);
	
	/* Enable GPIOC clock */
	rcu_periph_clock_enable(RCU_GPIOC);
	
	rcu_periph_clock_enable(RCU_AF);
  
  /* Configure  pin as input floating */
	gpio_init(GPIOC,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_13);
	
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
	
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_13);
	
	exti_init(EXTI_13, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	
	exti_interrupt_flag_clear(EXTI_13);
	
	/* Enable GPIOC clock */
	rcu_periph_clock_enable(RCU_GPIOC);
	
	rcu_periph_clock_enable(RCU_AF);
  
  /* Configure  pin as input floating */
	gpio_init(GPIOC,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_14);
	
	nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
	
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_14);
	
	exti_init(EXTI_14, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	
	exti_interrupt_flag_clear(EXTI_14);
	
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
    FLASH_WriteData(FLG_UPDATE_DONE_ADDR, (uint32_t)0x00, 0);
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
			gpio_bit_set(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
		}else if(ADCData.main_power_val < LOW_EXT_POWER_LEVEL){
			rst_condition = 0;
			if(flg_pw_in == 1){
				flg_pw_l = 1;
			}
			
			if((flg_pw_in == 1) && (flg_pw_h == 1) && (flg_pw_l == 1)){
				//power off battery
				gpio_bit_reset(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
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
					gpio_bit_reset(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
				}
			}
			gpio_bit_reset(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
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
				gpio_bit_reset(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
			}
		}
		osDelay(1000);
    osThreadYield();
	}
}

void EC21_PW_Reset(void) {
	int i = 0;
	//power off EC21
  gpio_bit_set(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
	/* power off RFID */
	gpio_bit_reset(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
	//wait 10 seconds
  for(i = 0; i < 10; i++){
		osDelay(1000);
	}
	//power on EC21
  gpio_bit_reset(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
	/* power on RFID */
	gpio_bit_set(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
}

/************************************************ADC**************************************************************/
static void PW_Control_init(void){
	/* RCC init */
	rcu_periph_clock_enable(PWR_EC21_GPIO_CLK);
	rcu_periph_clock_enable(PWR_BATTERY_GPIO_CLK);
	rcu_periph_clock_enable(PWR_BOST_GPIO_CLK);
	rcu_periph_clock_enable(PWR_MAIN_GPIO_GPIO_CLK);
	rcu_periph_clock_enable(PWR_5VOUT_GPIO_CLK);
	rcu_periph_clock_enable(PWR_RFID_GPIO_GPIO_CLK);
	
	/* Battery controls */
	gpio_init(PWR_BATTERY_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_BATTERY_GPIO_PIN);
	/* power on battery */
	gpio_bit_set(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_GPIO_PIN);
	
	/* Power EC21 controls */
	gpio_init(PWR_EC21_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_EC21_GPIO_PIN);
	/* power on EC21 */
	gpio_bit_reset(PWR_EC21_GPIO_PORT, PWR_EC21_GPIO_PIN);
	
	/* Power BOST controls */
	gpio_init(PWR_BOST_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_BOST_GPIO_PIN);
	/* power on bost */
	gpio_bit_reset(PWR_BOST_GPIO_PORT, PWR_BOST_GPIO_PIN);
	
	/* Power MAIN controls */
	gpio_init(PWR_MAIN_GPIO_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_MAIN_GPIO_PIN);
	/* power on 12vol */
	gpio_bit_reset(PWR_MAIN_GPIO_GPIO_PORT, PWR_MAIN_GPIO_PIN);
	
	/* Power 5V-OUT controls */
	gpio_init(PWR_5VOUT_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_5VOUT_GPIO_PIN);
	/* power off 5V-OUT */
	gpio_bit_reset(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
	
	/* Power RFID controls */
	gpio_init(PWR_RFID_GPIO_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_RFID_GPIO_PIN);
	/* power on RFID */
	gpio_bit_set(PWR_RFID_GPIO_GPIO_PORT, PWR_RFID_GPIO_PIN);
}

static void ADC_Initialize(void){
	/* enable GPIOA clock */
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);

	/* enable ADC clock */
	rcu_periph_clock_enable(RCU_ADC0); 
	/* config ADC clock */
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);//RCU_CKADC_CKAPB2_DIV6

	/* config the GPIO as analog mode */
	gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_10MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3);

	/* ADC mode config */
	adc_mode_config(ADC_MODE_FREE);
	/* ADC data alignment config */
	adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
	/* ADC channel length config */
	adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1U);

	/* ADC trigger config */
	adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
	/* ADC external trigger config */
	adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

	/* enable ADC interface */
	adc_enable(ADC0);
	osDelay(10);
	/* ADC calibration and reset calibration */
	adc_calibration_enable(ADC0);
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
//uint8_t pw_cnt = 0;
static void get_adc_values(void){
	uint16_t dig_val = 0;
	uint16_t f_dig_val = 0;
	uint16_t raw_voltage_val = 0;
	
	/* get main power milivoltage value */
	dig_val = adc_channel_sample(ADC_CHANNEL_10);
	ADCData.main_power_val = ((((dig_val * 3300) / 0xFFF) * (V12_R1 + V12_R2)) / V12_R2);
	//ADCData.main_power_val += (ADCData.main_power_val * 14 / 100);
	ADCData.main_power_val += 700;
	
	/* get main power milivoltage value */
	dig_val = adc_channel_sample(ADC_CHANNEL_11);
	ADCData.v_5v6_val = ((((dig_val * 3300) / 0xFFF) * (V5V6_R1 + V5V6_R2)) / V5V6_R2);
	
	/* get battery milivoltage value */
	dig_val = adc_channel_sample(ADC_CHANNEL_13);
	BAT_fillter_values(dig_val, &f_dig_val);
	raw_voltage_val = (f_dig_val * 3300) / 0xFFF;
	ADCData.battery_val = ((raw_voltage_val * (VBAT_R1 + VBAT_R2)) / VBAT_R2);
	//ADCData.battery_val += ((ADCData.battery_val * 3) / 100);
	
//	if(pw_cnt++ % 5 == 0){
//		sprintf(test_val, "12Vin = %d, bat = %d, adc = %d, 5v6 = %d, ACC: %d - DOOR: %d - AIR: %d",
//		ADCData.main_power_val,  ADCData.battery_val, ADCData.adc_pulse_val, ADCData.v_5v6_val, ACC_STATUS, DOOR_STATUS, AIR_STATUS);
//		SerialPushStr(test_val, END_LF); 
//	}
}

static void FuelCyclesConfig(void){
  /* Enable GPIOA clock */
	rcu_periph_clock_enable(RCU_GPIOA);
	
	rcu_periph_clock_enable(RCU_AF);
  
  /* Configure  pin as input floating */
	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_0);
	
	nvic_irq_enable(EXTI0_IRQn, 2U, 0U);
	
	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_0);
	
	exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
	
	exti_interrupt_flag_clear(EXTI_0);
}

/* Pulse cycles interrupt */
void EXTI0_IRQHandler(void){

}

void EXTI10_15_IRQHandler(void){
	if(exti_interrupt_flag_get(EXTI_13) != RESET){
		
		sensorIR.countFallingIR1++;
		/* Clear the EXTI line 13 pending bit */
		exti_interrupt_flag_clear(EXTI_13);
	}
	
	if(exti_interrupt_flag_get(EXTI_14) != RESET){
		
		sensorIR.countFallingIR2++;
		/* Clear the EXTI line 14 pending bit */
		exti_interrupt_flag_clear(EXTI_14);
	}
}


static void PulseTimerInit(void){
	//Timer6 configuration
	timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_TIMER6);

	//timer_break_parameter_struct timer_breakpara;
	timer_deinit(TIMER6);
	
	timer_initpara.prescaler = 1;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = 59999;
	
	timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER6, &timer_initpara);
	
	/* Enable Timer Interupt */
	nvic_irq_enable(TIMER6_IRQn,1,1);
	
	/* Enable TIMER Update interrupt */
	timer_interrupt_enable(TIMER6, TIMER_INT_UP);
	
	/* TIMER enable counter */
  timer_enable(TIMER6);
}

uint8_t ctltran = 0 ;
uint16_t index1 = 0;
uint8_t index2 = 0;

uint16_t counter_ir_1 = 0;
uint16_t counter_ir_2 = 0;
/* Timer for fuel cycles and pulse */
void TIMER6_IRQHandler(void){
  if(timer_interrupt_flag_get(TIMER6, TIMER_INT_FLAG_UP)){
		timer_interrupt_flag_clear(TIMER6, TIMER_INT_UP);
		
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

#include "interface.h"
#include "Serial.h"
#include "power.h"

void vInterfaceTask(const void* args);

static void UserTimerHanler (void const *arg);

/* argument for the timer call back function */
static osTimerDef (UserTimer, UserTimerHanler);

static void ButtonTimerHanler (void const *arg);

/* argument for the timer call back function */
static osTimerDef (ButtonTimer, ButtonTimerHanler);
uint8_t Ir_test_mode = 0;
uint8_t Mode_disable_OBD = 0;
static __IO LedBuzzer_Def LedBuzzerState;
static __IO uint16_t LedTime;
static __IO uint16_t BuzzTime;
static __IO uint16_t BuzzerTime;
static uint8_t buzz_state = 0xFF;

static void GPIO_Initialize(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(LED_TCP_GPIO_CLK | LED_GPS_GPIO_CLK | LED_DRIVER_GPIO_CLK | BUZZER_GPIO_CLK | ACC_GPIO_CLK | DOOR_GPIO_CLK | AIR_GPIO_CLK, ENABLE);
	
	/* CFG output mode */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin   = LED_TCP_GPIO_PIN;
	GPIO_Init(LED_TCP_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(LED_TCP_GPIO_PORT, LED_TCP_GPIO_PIN);
	
	GPIO_InitStructure.GPIO_Pin   = LED_GPS_GPIO_PIN;
	GPIO_Init(LED_GPS_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(LED_GPS_GPIO_PORT, LED_GPS_GPIO_PIN);
	
	GPIO_InitStructure.GPIO_Pin   = LED_DRIVER_GPIO_PIN;
	GPIO_Init(LED_DRIVER_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(LED_DRIVER_GPIO_PORT, LED_DRIVER_GPIO_PIN);
	
	GPIO_InitStructure.GPIO_Pin   = BUZZER_GPIO_PIN;
	GPIO_Init(BUZZER_GPIO_PORT, &GPIO_InitStructure);

	/* CFG input mode */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
  GPIO_InitStructure.GPIO_Pin = ACC_GPIO_PIN;
  GPIO_Init(ACC_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = DOOR_GPIO_PIN;
  GPIO_Init(DOOR_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = AIR_GPIO_PIN;
  GPIO_Init(AIR_GPIO_PORT, &GPIO_InitStructure);
}

void Set_Led_Status(Led_TypeDef LedType, uint8_t state){
	switch(LedType){
		case LED_TCP:
			break;
		case LED_GPS:
			break;
		case LED_DRIVER:
			break;
		case BUZZER:
			BuzzTime = 0;
			LedBuzzerState.BuzzerStatus = (Buzzer_StatusDef)state;
			break;
	}
}

Buzzer_StatusDef get_buzer_state(void){
	return LedBuzzerState.BuzzerStatus;
}

static void Led_On(Led_TypeDef LedType){
	switch(LedType){
		case LED_TCP:
			LED_TCP_GPIO_PORT->BRR = LED_TCP_GPIO_PIN;
			break;
		case LED_GPS:
			LED_GPS_GPIO_PORT->BRR = LED_GPS_GPIO_PIN;
			break;
		case LED_DRIVER:
			LED_DRIVER_GPIO_PORT->BRR = LED_DRIVER_GPIO_PIN;
			break;
		default: break;
	}
}

static void Led_Off(Led_TypeDef LedType){
	switch(LedType){
		case LED_TCP:
			LED_TCP_GPIO_PORT->BSRR = LED_TCP_GPIO_PIN;
			break;
		case LED_GPS:
			LED_GPS_GPIO_PORT->BSRR = LED_GPS_GPIO_PIN;
			break;
		case LED_DRIVER:
			LED_DRIVER_GPIO_PORT->BSRR = LED_DRIVER_GPIO_PIN;
			break;	
		default:break;		
	}
}

void reset_buzz_state(void){
	buzz_state = 0xFF;
}

static uint8_t get_buzzer_disable(void){
	buzz_state = 0;//FlashReadUint32(ADD_BUZZER_DISABLE);
	if(buzz_state > 2) return 0;
	return buzz_state;
}

uint8_t is_buzz_disable(void){
	if(buzz_state == 0xFF) return get_buzzer_disable();
	return buzz_state;
}

void Buzz_On(void){
	if(is_buzz_disable() == 1) return;
	/* Enable Timer Interupt */
	NVIC_EnableIRQ(TIM4_IRQn);
	
	/* Enable TIMER Update interrupt */
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	/* TIMER enable counter */
  TIM_Cmd(TIM4, ENABLE);
}

void Buzz_Off(void){
	if(is_buzz_disable() == 1) return;
	/* Disable Timer Interupt */
	NVIC_DisableIRQ(TIM4_IRQn);
	
	/* Disable TIMER Update interrupt */
	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);	
	/* TIMER Disable counter */
  TIM_Cmd(TIM4, DISABLE);
	
	/* Set buzzer pin -> Low level */
	GPIO_ResetBits(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}

u8 status_of_led;
static void LedAndBuzzerControl(void){
	if(mcu_config.LedStatus == 0){//khong khach
		GPIO_ResetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
		sensorIR.statusHiring = 0;
	}else if(mcu_config.LedStatus == 1){//co khach
		sensorIR.statusHiring = 1;
		GPIO_SetBits(PWR_5VOUT_GPIO_PORT, PWR_5VOUT_GPIO_PIN);
	}
	
	switch(ec21_data.tcp_status){
		case 0:{
			if(ec21_data.sim_status == E_QL_SIM_STAT_NOT_INSERTED){
				switch(BuzzerTime){
					case 0:Led_On  (LED_TCP);break;
					case 2:Led_On  (LED_TCP);break;
					default:Led_Off(LED_TCP);break;
				}
			}else{
				if(ec21_data.internet_status == 1){
					Led_On(LED_TCP);
				}else{
					switch(BuzzerTime){
						case 0:Led_On  (LED_TCP);break;
						case 2:Led_On  (LED_TCP);break;
						case 4:Led_On  (LED_TCP);break;
						default:Led_Off(LED_TCP);break;
					}
				}
			}
			break;
		}
		case 1:
			switch(LedTime){
				case 0:Led_On (LED_TCP);break;
				case 1:Led_Off(LED_TCP);break;
				case 2:Led_On (LED_TCP);break;
				case 3:Led_Off(LED_TCP);break;
				case 4:Led_On (LED_TCP);break;
				case 5:Led_Off(LED_TCP);break;
				case 6:Led_On (LED_TCP);break;
				case 7:Led_Off(LED_TCP);break;
			}
			break;
	}
	
	if(PowerData.power_mode == POWER_SAVE){
		if(ec21_data.gps_status == 1){
			if(LedTime == 1){
				Led_On (LED_GPS);
			}else{
				Led_Off(LED_GPS);
			}
		}else{
			Led_Off(LED_GPS);
		}
	}else{
		switch(ec21_data.gps_status){
			case 1:
				switch(LedTime){
					case 0:Led_On (LED_GPS);break;
					case 1:Led_Off(LED_GPS);break;
					case 2:Led_On (LED_GPS);break;
					case 3:Led_Off(LED_GPS);break;
					case 4:Led_On (LED_GPS);break;
					case 5:Led_Off(LED_GPS);break;
					case 6:Led_On (LED_GPS);break;
					case 7:Led_Off(LED_GPS);break;
				}
				break;
			case 0:Led_On(LED_GPS);break;
		}
	}
	
	switch(ec21_data.login_status){
		case 0:
			Led_Off(LED_DRIVER);
			break;
		case 1:
			switch(LedTime){
				case 0:Led_On (LED_DRIVER);break;
				case 1:Led_Off(LED_DRIVER);break;
				case 2:Led_On (LED_DRIVER);break;
				case 3:Led_Off(LED_DRIVER);break;
				case 4:Led_On (LED_DRIVER);break;
				case 5:Led_Off(LED_DRIVER);break;
				case 6:Led_On (LED_DRIVER);break;
				case 7:Led_Off(LED_DRIVER);break;
			}
			break;
	}
	if(mcu_config.buzz_level != 0){
		switch(LedBuzzerState.BuzzerStatus){
			case None:{
				if(Ir_test_mode == 0){
					Buzz_Off();
				}
				break;
			}
			case DeviceStarting:
				switch(BuzzTime){
					case 0:Buzz_On();break;
					case 1:Buzz_On();break;
					case 2:Buzz_On();break;
					case 3:Buzz_Off();
						LedBuzzerState.BuzzerStatus = None;
						break;
					default:break;
				}
				break;
			case DeviceStarted:
				switch(BuzzTime){
					case 0:Buzz_On();break;
					case 1:Buzz_Off();break;
					case 2:Buzz_On();break;
					case 3:Buzz_Off();
						LedBuzzerState.BuzzerStatus = None;
						break;
					default:break;
				}
				break;
			case OverSpeed:
				if(BuzzerTime % 7 == 0x00)
					Buzz_On();
				else
					Buzz_Off();
				break;
			case DriveContinuousOverTime:
				switch(BuzzerTime){
					case 0:Buzz_On();break;
					case 2:Buzz_On();break;
					case 4:Buzz_On();break;
					case 6:Buzz_On();break;
					default:Buzz_Off();break;
				}
				break;
			case bDriverLogin:
				switch(BuzzTime){
					case 0:Buzz_On();break;
					case 1:Buzz_On();break;
					case 2:Buzz_On();break;
					case 3:
						Buzz_Off();
						LedBuzzerState.BuzzerStatus = None;
						break;
					default:break;
				}
				break;
			case bDriverLogout:
				switch(BuzzTime){
					case 0:Buzz_On();break;
					case 1:Buzz_Off();break;
					case 2:Buzz_On();break;
					case 3:Buzz_Off();break;
					case 4:Buzz_On();break;
					case 5:Buzz_Off();
						LedBuzzerState.BuzzerStatus = None;
						break;
					default:break;
				}
				break;
			default:break;
		}
	}
	LedTime++;
	BuzzTime++;
	BuzzerTime++;
	if(BuzzerTime > 70)BuzzerTime = 0;
	if(LedTime > 7)LedTime = 0;
	if(BuzzTime > 7)BuzzTime = 0;
}

static void ButtonTimer_Init(void){
  static osTimerId bTimId;

  bTimId = osTimerCreate(osTimer(ButtonTimer), osTimerPeriodic, (void * ) 1);
  if (bTimId != NULL) {
    osStatus status;
    // start timer with delay 50ms
    status = osTimerStart(bTimId, 50);
    if (status != osOK) {
      SerialPushStr("Error! Start Button Timer", END_LF);
    }
  }
}

static void UserTimer_Init(void){
	static osTimerId uTimId;  
	uTimId = osTimerCreate (osTimer(UserTimer), osTimerPeriodic, (void*)0);
	
	if (uTimId != NULL){
		osStatus status;
    // start timer with delay 150ms
    status = osTimerStart(uTimId, 150);
    if (status != osOK) {
      return;
    }
  }
}

static uint16_t get_buzzer_period(void){
	switch(mcu_config.buzz_level){
		case 1:
			return BUZZER_LEVEL1;
		case 2:
			return BUZZER_LEVEL2;
		case 3:
			return BUZZER_LEVEL3;
		case 4:
			return BUZZER_LEVEL4;
		case 5:
			return BUZZER_LEVEL5;
		default:
			return BUZZER_LEVEL5;
	}
}

static void irqTimer_Init(void){
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	/* Enable TIM2 Clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	/* TIMER2 configuration */
	TIM_TimeBaseStructure.TIM_Period 					= get_buzzer_period();
	TIM_TimeBaseStructure.TIM_Prescaler 			= 13;
	TIM_TimeBaseStructure.TIM_ClockDivision 	= TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);	
	TIM_UpdateRequestConfig(TIM4, TIM_UpdateSource_Global);	
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
}

void Interface_Init(void){
	/* Init GPIO port */
	GPIO_Initialize();
	
	if(mcu_config.slience_mode == 1){
		return;
	}
	
	/* Init led buzzer control Timer */
	UserTimer_Init();
	/* init buzzer Timer */
	irqTimer_Init();
	
	/* Init led buzzer control Timer */
	UserTimer_Init();
	
	ButtonTimer_Init();
	
	/* Set buzzer Device started  */
	Set_Led_Status(BUZZER, DeviceStarted);
}

void TIM4_IRQHandler(void){
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		BUZZER_GPIO_PORT->ODR ^= BUZZER_GPIO_PIN;
	}
}

static void UserTimerHanler (void const *arg){
	LedAndBuzzerControl();
}

uint8_t GetStatusAir(void){
	if(mcu_config.TaxiMode == 0){
		return (!GPIO_ReadInputDataBit(AIR_GPIO_PORT, AIR_GPIO_PIN));
	}else{
		return 0;
	}
}

uint8_t GetStatusDoor(void){
	if(mcu_config.TaxiMode == 0){
		return (!GPIO_ReadInputDataBit(DOOR_GPIO_PORT, DOOR_GPIO_PIN));
	}else{
		return 0;
	}
}

u8 button_timeout = 0;
u8 count_of_push = 0;
u8 active_buzz = 0;
static void ButtonTimerHanler(void const * arg) {
  static uint8_t tHold = 0;
	
	if(BUTTON_STATUS == 1){
		tHold++;
  }else{
		if((tHold >= 1) && (tHold <= 10)){
      button_timeout = 0;
			count_of_push++;
    }
    tHold = 0;
  }
	button_timeout++;
	if(button_timeout > 40){
		if(count_of_push == 1){//bam 1 lan => chuyen sang trang thai auto (release)
			mcu_config.LedMode = 0;
			time_check_sensor = 30;
			Ir_test_mode = 0;
			Mode_disable_OBD = 0;
			active_buzz = 5;
			sensorIR.statusButton = 1;
			mcu_config.tmp_use_release_button = 1;
		}else if(count_of_push == 4){
			//mode test Irled
			Ir_test_mode = 1;
			active_buzz = 15;
		}else if(count_of_push == 6){
			//mod off OBD in 30min
			Mode_disable_OBD = 1;
			active_buzz = 30;
		}
		button_timeout = 0;
		count_of_push = 0;
	}
	
	if(active_buzz != 0){
		active_buzz --;
		Buzz_On();
		if(active_buzz <= 1){
			active_buzz = 0;
			Buzz_Off();
		}
	}
}

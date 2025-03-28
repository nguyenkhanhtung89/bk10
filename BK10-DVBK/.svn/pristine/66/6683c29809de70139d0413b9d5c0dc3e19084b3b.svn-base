#include "interface.h"
#include "Serial.h"
//#include "display.h"

static void UserTimerHanler (void const *arg);

/* argument for the timer call back function */
static osTimerDef (UserTimer, UserTimerHanler);

static __IO LedBuzzer_Def LedBuzzerState;
static __IO uint16_t LedTime;
static __IO uint16_t BuzzTime;
static __IO uint16_t BuzzerTime;
static uint8_t buzz_state = 0xFF;

static void GPIO_Initialize(void){
	rcu_periph_clock_enable(LED_TCP_GPIO_CLK);
	rcu_periph_clock_enable(LED_GPS_GPIO_CLK);
	rcu_periph_clock_enable(LED_POWER_GPIO_CLK);
	rcu_periph_clock_enable(LED_DRIVER_GPIO_CLK);
	rcu_periph_clock_enable(BUZZER_GPIO_CLK);
	
	gpio_init(LED_TCP_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,LED_TCP_GPIO_PIN);
	gpio_bit_set(LED_TCP_GPIO_PORT,LED_TCP_GPIO_PIN);

	gpio_init(LED_GPS_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,LED_GPS_GPIO_PIN);
	gpio_bit_set(LED_GPS_GPIO_PORT,LED_GPS_GPIO_PIN);

	gpio_init(LED_POWER_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,LED_POWER_GPIO_PIN);
	gpio_bit_set(LED_POWER_GPIO_PORT,LED_POWER_GPIO_PIN);

	gpio_init(LED_DRIVER_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,LED_DRIVER_GPIO_PIN);
	gpio_bit_set(LED_DRIVER_GPIO_PORT,LED_DRIVER_GPIO_PIN);

	gpio_init(BUZZER_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BUZZER_PIN);
}

void Set_Led_Status(Led_TypeDef LedType, uint8_t state){
	switch(LedType){
		case LED_TCP:
			break;
		case LED_GPS:
			break;
		case LED_DRIVER:
			LedBuzzerState.DriverLedStatus = (Driver_StatusDef)state;
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
			gpio_bit_set(LED_TCP_GPIO_PORT,LED_TCP_GPIO_PIN);
			break;
		case LED_GPS:
			gpio_bit_set(LED_GPS_GPIO_PORT,LED_GPS_GPIO_PIN);
			break;
		case LED_DRIVER:
			gpio_bit_set(LED_DRIVER_GPIO_PORT,LED_DRIVER_GPIO_PIN);
			break;
		default: break;
	}
}

static void Led_Off(Led_TypeDef LedType){
	switch(LedType){
		case LED_TCP:
			gpio_bit_reset(LED_TCP_GPIO_PORT,LED_TCP_GPIO_PIN);
			break;
		case LED_GPS:
			gpio_bit_reset(LED_GPS_GPIO_PORT,LED_GPS_GPIO_PIN);
			break;
		case LED_DRIVER:
			gpio_bit_reset(LED_DRIVER_GPIO_PORT,LED_DRIVER_GPIO_PIN);
			break;	
		default:break;		
	}
}

void reset_buzz_state(void){
	buzz_state = 0xFF;
}

//(0 on)   // (1 off)   // (2 on - off logout ~ run)
static uint8_t get_buzzer_disable(void){
	buzz_state = 0;//FlashReadUint32(ADD_BUZZER_DISABLE);
	if(buzz_state > 2) return 0;
	return buzz_state;
}

uint8_t is_buzz_disable(void){
	if(buzz_state == 0xFF) return get_buzzer_disable();
	return buzz_state;
}
static void Buzz_On(void){
	if(is_buzz_disable() == 1) return;

	/* Enable Timer Interupt */
	nvic_irq_enable(TIMER5_IRQn,1,1);
	
	/* Enable TIMER Update interrupt */
	timer_interrupt_enable(TIMER5, TIMER_INT_UP);
	
	/* TIMER enable counter */
  timer_enable(TIMER5);
	
}

static void Buzz_Off(void){
	if(is_buzz_disable() == 1) return;
	/* Disable Timer Interupt */
	nvic_irq_disable(TIMER5_IRQn);
	
	/* Disable TIMER Update interrupt */
	timer_interrupt_disable(TIMER5, TIMER_INT_UP);
	/* TIMER Disable counter */
  timer_disable(TIMER5);
	
	/* Set buzzer pin -> Low level */
	gpio_bit_reset(BUZZER_GPIO_PORT, BUZZER_PIN);
}

static void LedAndBuzzerControl(void){
	switch(ec21_data.tcp_status){
		case 0:{
			if(ec21_data.sim_status == 0){
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
	switch(LedBuzzerState.DriverLedStatus){
		case PutinTag:
			Led_Off(LED_DRIVER);
			break;
		case PutOutTag:
			Led_On(LED_DRIVER);
			break;
		case DriverLogin:
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
			case None:Buzz_Off();break;
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
	//Timer5 configuration
	timer_parameter_struct timer_initpara;
	
	rcu_periph_clock_enable(RCU_TIMER5);

	//timer_break_parameter_struct timer_breakpara;
	timer_deinit(TIMER5);
	
	timer_initpara.prescaler = 22;
	timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection = TIMER_COUNTER_UP;
	timer_initpara.period = get_buzzer_period();
	timer_initpara.clockdivision = TIMER_CKDIV_DIV2;
	timer_initpara.repetitioncounter = 0;
	timer_init(TIMER5, &timer_initpara);
}

void Interface_Init(void){//LED, BUZZER, LCD, BUTTON	
	/* Init GPIO port */
	GPIO_Initialize();
	
	if(mcu_config.slience_mode == 1){
		return;
	}
	/* Init led buzzer control Timer */
	UserTimer_Init();
	/* init buzzer Timer */
	irqTimer_Init();
	/* Set buzzer Device started  */
	Set_Led_Status(BUZZER, DeviceStarted);
}
uint8_t stateLogicBuz = 0;
void TIMER5_IRQHandler(void){
	if(timer_interrupt_flag_get(TIMER5, TIMER_INT_FLAG_UP))
	{
		timer_interrupt_flag_clear(TIMER5, TIMER_INT_UP);
		if(stateLogicBuz == 0)
		{
			gpio_bit_reset(BUZZER_GPIO_PORT,BUZZER_PIN);
			stateLogicBuz = 1;
		}
		else
		{
			gpio_bit_set(BUZZER_GPIO_PORT,BUZZER_PIN);
			stateLogicBuz = 0;
		}
	}
		
}

static void UserTimerHanler (void const *arg){
	LedAndBuzzerControl();
}

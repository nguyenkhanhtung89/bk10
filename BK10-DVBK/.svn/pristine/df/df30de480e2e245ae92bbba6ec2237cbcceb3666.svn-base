/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic
#include "osObjects.h"

#define PWR_BATTERY_PIN								GPIO_Pin_11
#define PWR_BATTERY_GPIO_PORT 				GPIOB
#define PWR_BATTERY_GPIO_CLK					RCC_AHBPeriph_GPIOB

#define PWR_EC21_PIN									GPIO_Pin_12
#define PWR_EC21_GPIO_PORT 						GPIOC
#define PWR_EC21_GPIO_CLK					 	  RCC_AHBPeriph_GPIOC

#define PWR_RFID_PIN									GPIO_Pin_9
#define PWR_RFID_GPIO_PORT 						GPIOB
#define PWR_RFID_GPIO_CLK					 	  RCC_AHBPeriph_GPIOB

static void pw_init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
  RCC_AHBPeriphClockCmd(PWR_BATTERY_GPIO_CLK | PWR_EC21_GPIO_CLK | PWR_RFID_GPIO_CLK, ENABLE);
	/* CFG output mode */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	/* Pin on/off power EC21 */
	GPIO_InitStructure.GPIO_Pin   = PWR_EC21_PIN;
  GPIO_Init(PWR_EC21_GPIO_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(PWR_EC21_GPIO_PORT, PWR_EC21_PIN);
	
	/* pin on/off battery */
	GPIO_InitStructure.GPIO_Pin   = PWR_BATTERY_PIN;
  GPIO_Init(PWR_BATTERY_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(PWR_BATTERY_GPIO_PORT, PWR_BATTERY_PIN);
	
	/* pin on/off RFID */
	GPIO_InitStructure.GPIO_Pin   = PWR_RFID_PIN;
  GPIO_Init(PWR_RFID_GPIO_PORT, &GPIO_InitStructure);
	GPIO_SetBits(PWR_RFID_GPIO_PORT, PWR_RFID_PIN);
}

void goto_application(uint32_t addr){
	pFunction Jump_To_Application;
	uint32_t JumpAddress;
	
	/* Jump to application */
	JumpAddress = *(__IO uint32_t*) (addr + 4);
	Jump_To_Application = (pFunction) JumpAddress;
	
	/* Initialize user application's Stack Pointer */
	__set_MSP(*(__IO uint32_t*) addr);
	Jump_To_Application();
}

void IWDG_Start(void){
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	/* Set proper clock depending on timeout user select */
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	
	/* Reload IWDG counter */
	IWDG_SetReload(1024);
	
	/* Reload IWDG counter */
	IWDG_ReloadCounter();
	
	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

int main(void){
	RCC_ClocksTypeDef RCC_ClockFreq;
	RCC_GetClocksFreq(&RCC_ClockFreq);
	/* initialize CMSIS-RTOS */
  osKernelInitialize();
	
	/* Watchdog timer init */
	IWDG_Start();
	
	/* Power on full */
	pw_init();
	
  /* create 'thread' functions that start executing */
	Init_Thread();
	
	/* start thread execution */
  osKernelStart();
	
	for(;;){
		osDelay(1000);
		/* Watchdog reset */
		IWDG_ReloadCounter();
		
		osThreadYield();
	}
}

void assert_failed(uint8_t* file, uint32_t line){
	for(;;);   // hang here (behavior is our your choice)
}

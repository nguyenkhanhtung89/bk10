/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#define PWR_BATTERY_PIN GPIO_PIN_11
#define PWR_BATTERY_GPIO_PORT GPIOB
#define PWR_BATTERY_GPIO_CLK RCU_GPIOB

#define PWR_EC21_PIN GPIO_PIN_12
#define PWR_EC21_GPIO_PORT GPIOC
#define PWR_EC21_GPIO_CLK RCU_GPIOC

#define PWR_RFID_PIN GPIO_PIN_9
#define PWR_RFID_GPIO_PORT GPIOB
#define PWR_RFID_GPIO_CLK RCU_GPIOB

static void pw_init(void)
{
	rcu_periph_clock_enable(PWR_BATTERY_GPIO_CLK);
	rcu_periph_clock_enable(PWR_EC21_GPIO_CLK);
	rcu_periph_clock_enable(PWR_RFID_GPIO_CLK);
	
	gpio_init(PWR_BATTERY_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_BATTERY_PIN);
	gpio_bit_set(PWR_BATTERY_GPIO_PORT,PWR_BATTERY_PIN);
	
	gpio_init(PWR_EC21_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_EC21_PIN);
	gpio_bit_reset(PWR_EC21_GPIO_PORT,PWR_EC21_PIN);
	
	gpio_init(PWR_RFID_GPIO_PORT,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,PWR_RFID_PIN);
	gpio_bit_set(PWR_RFID_GPIO_PORT,PWR_RFID_PIN);
}

void goto_application(uint32_t addr)
{
	pFunction Jump_To_Application;
	uint32_t JumpAddress;
//	nvic_vector_table_set(FLASH_BASE,0xA000);
	//Jump to application
	JumpAddress = *(__IO uint32_t *)(addr + 4);
	Jump_To_Application = (pFunction) JumpAddress;
	
	//Initialize user application's Stack Pointer
	__set_MSP(*(__IO uint32_t*)addr);
	Jump_To_Application();
}

void IWDG_Start(void){
	rcu_osci_on(RCU_IRC40K);
	/* wait till IRC40K is ready */
	while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K)){
	}
  /* confiure FWDGT counter clock: 40KHz(IRC40K) / 256 = 0.15625 KHz */
	fwdgt_config(1024,FWDGT_PSC_DIV256);
	
	/* After 6.55 seconds to generate a reset */
	fwdgt_counter_reload();
	fwdgt_enable();
}
/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
  // initialize peripherals here
	//watchdog timer init
	IWDG_Start();
	
	//power on full
	pw_init();
	
	//create 'thread' functions that start executing
	Init_Thread();

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);

  osKernelStart ();                         // start thread execution 
	
	for(;;)
	{
		osDelay(1000);
		//Watchdog reset
		fwdgt_counter_reload();
		
		osThreadYield();
	}
}

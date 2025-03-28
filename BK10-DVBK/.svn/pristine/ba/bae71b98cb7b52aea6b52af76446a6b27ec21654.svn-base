/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

static void IWDG_Start(void){
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
	uint32_t fre = 0;
	fre = rcu_clock_freq_get(CK_SYS);
  osKernelInitialize ();                    // initialize CMSIS-RTOS

  // initialize peripherals here
	/* Watchdog timer init */
	IWDG_Start();
	
	/* create 'thread' functions that start executing */
	Init_Thread();
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);

  osKernelStart ();                         // start thread execution 
	for(;;){
		fwdgt_counter_reload();
		osDelay(100);
	}
}

uint32_t flash_read_uint32_t(uint32_t address) {
  return ( * (volatile uint32_t * ) address);
}

uint32_t FLASH_ReadData(uint32_t ADDR) { //set flg = 0 in app
  uint32_t Result = 0x00;

  Result = flash_read_uint32_t(ADDR);
	
  /* Return Application Program */
  return Result;
}

volatile uint8_t oneTh = 0;
void FLASH_WriteData(uint32_t ADDR, uint32_t Data, uint8_t reset) {
	  oneTh++;
	  if(oneTh == 1){
		NVIC_DisableIRQ(USART0_IRQn);
    /* unlock the flash program/erase controller */
    fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
		
		if(fmc_page_erase(ADDR) != FMC_READY)
		{
			fmc_lock();
			NVIC_EnableIRQ(USART0_IRQn);
			return;
		}

		
		fmc_word_program(ADDR, Data);

		
		fmc_lock();
		
		NVIC_EnableIRQ(USART0_IRQn);
		
		if (reset) {
			osDelay(1000);
			NVIC_SystemReset();
		}
	}
		oneTh--;
}

void assert_failed(uint8_t* file, uint32_t line){
  char text[256];
	
	sprintf(text, "Assert fail at File %s Line %d", file, (int)line);
	
	for(;;);   // hang here (behavior is our your choice)
}


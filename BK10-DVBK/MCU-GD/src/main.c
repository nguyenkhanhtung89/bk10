/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic
#include "osObjects.h"

static void IWDG_Start(void){
	rcu_osci_on(RCU_IRC40K);
	
	/* wait till IRC40K is ready */
	while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K)){
	}
	
  /* confiure FWDGT counter clock: 40KHz(IRC40K) / 256 = 0.15625 KHz */
	fwdgt_config(1024, FWDGT_PSC_DIV256);
	
	/* After 6.55 seconds to generate a reset */
	fwdgt_counter_reload();
	fwdgt_enable();
}


int main (void) {
	uint32_t fre = 0;
	uint16_t cnt_warning_cmr = 0;
	fre = rcu_clock_freq_get(CK_SYS);
	
	// initialize CMSIS-RTOS
  osKernelInitialize ();
	
	osDelay(1000);
	
	IWDG_Start();
  
	/* create 'thread' functions that start executing */
	Init_Thread();
	
	// start thread execution 
  osKernelStart ();                        
	for(;;){
		osDelay(1000);
		
		/* Watchdog reset */
		fwdgt_counter_reload();
		
		/* check warning camera */
		if(ec21_data.camera_warning == 1){
			if((cnt_warning_cmr % 600) == 0){
				Set_Led_Status(BUZZER, CamDisconnected);
				cnt_warning_cmr++;
			}
		}
		
		osThreadYield();
	}
}

void assert_failed(uint8_t* file, uint32_t line){
//  char text[256];
//	
//	sprintf(text, "Assert fail at File %s Line %d", file, (int)line);
	
	for(;;);   // hang here (behavior is our your choice)
}


/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic
#include "osObjects.h"
 
int main(void){
//	RCC_ClocksTypeDef RCC_ClockFreq;
//	RCC_GetClocksFreq(&RCC_ClockFreq);
	uint16_t cnt_warning_cmr = 0;
	/* initialize CMSIS-RTOS */
  osKernelInitialize();
	
  /* create 'thread' functions that start executing */
	Init_Thread();
	
	/* start thread execution */
  osKernelStart();
	
	for(;;){
		osDelay(1000);		
		/* Watchdog reset */
		IWDG_ReloadCounter();		
		
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
  //char text[256];
	
	//sprintf(text, "Assert fail at File %s Line %d", file, (int)line);
	
	for(;;);   // hang here (behavior is our your choice)
}

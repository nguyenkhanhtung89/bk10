/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic
#include "osObjects.h"
 
int main(void){
//	RCC_ClocksTypeDef RCC_ClockFreq;
//	RCC_GetClocksFreq(&RCC_ClockFreq);
	/* initialize CMSIS-RTOS */
  osKernelInitialize();
	
  /* create 'thread' functions that start executing */
	Init_Thread();
	
	/* start thread execution */
  osKernelStart();
	
	for(;;){
		osDelay(1000);
		canbus.timeout++;
		if(canbus.timeout > 120){
			canbus.timeout = 120;
			canbus.connected = 0;
		}
		
		if(time_check_sensor != 0){
			time_check_sensor --;
		}

		if((mcu_config.use_release_button == 0) && (mcu_config.tmp_use_release_button == 1)){
			mcu_config.use_release_button  = 1;
			FLASH_WriteData(USE_RELEASE_BUTTON, (uint32_t)mcu_config.use_release_button, 0);
		}

		/* Watchdog reset */
		IWDG_ReloadCounter();		
		osThreadYield();
	}
}

void assert_failed(uint8_t* file, uint32_t line){
  //char text[256];
	
	//sprintf(text, "Assert fail at File %s Line %d", file, (int)line);
	
	for(;;);   // hang here (behavior is our your choice)
}

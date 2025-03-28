/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic
#include "osObjects.h"
#include "flash.h"
#include "utils.h"

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

char test_f[64];
u8 tmp_buff[32];
int main (void) {
	uint32_t fre = 0;
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
		canbus.timeout++;
		if(canbus.timeout > 30){
			canbus.timeout = 30;
			canbus.connected = 0;
		}
		
		if(time_check_sensor != 0){
			time_check_sensor --;
		}

		if((mcu_config.use_release_button == 0) && (mcu_config.tmp_use_release_button == 1)){
			mcu_config.use_release_button  = 1;
			FLASH_WriteData(USE_RELEASE_BUTTON, (uint32_t)mcu_config.use_release_button, 0);
		}
		
		if(flg_check_update_vin == 1){
			flg_check_update_vin = 0;
			memset(tmp_buff, 0x0, 32);
			FLASH_ReadBufData(ADD_VIN_NUMBER, tmp_buff, 17);
			
			if((buf_cmp(tmp_buff, VIN_NUMBER, 17) != 0) && (strlen((char*)VIN_NUMBER) == 17)){
				FLASH_WriteBufData(ADD_VIN_NUMBER, VIN_NUMBER, strlen((char*)VIN_NUMBER));
			}
		}
		
		/* Watchdog reset */
		fwdgt_counter_reload();
		
		osThreadYield();
	}
}

void assert_failed(uint8_t* file, uint32_t line){
//  char text[256];
//	
//	sprintf(text, "Assert fail at File %s Line %d", file, (int)line);
	
	for(;;);   // hang here (behavior is our your choice)
}


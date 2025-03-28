#include "thread.h"
	
void Init_Thread(void){
	/* load mcu config */
	load_mcu_config();
	/* Init Serial Port */
	Serial_Init();
	/* uart  */
	digital_init();
	/* Init power control */
	Power_Init();
	/* led and buzzer init */
	Interface_Init();
	/* init OBD service */
	CAN_service_init();
	/* temp sensor init */
	DS18B20_Init();
}

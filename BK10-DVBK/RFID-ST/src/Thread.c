#include "thread.h"
	
void Init_Thread(void){
	/* Init Serial Port */
	Serial_Init();
	
	/* Init RFID */
	RFID_Init();
	
	Interface_Init();
}


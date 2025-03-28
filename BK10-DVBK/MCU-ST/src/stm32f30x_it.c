#include "stm32f30x_it.h"
#include "Serial.h"

void NMI_Handler(void){
	PrintError("NMI_Handler");
	while(1){};
}

void HardFault_Handler(void){
	PrintError("HardFault_Handler");
  NVIC_SystemReset();
	while(1){};
}

void MemManage_Handler(void){
	PrintError("MemManage_Handler");
	while(1){};
}

void BusFault_Handler(void){
	PrintError("BusFault_Handler");
	while(1){};
}

void UsageFault_Handler(void){
	PrintError("UsageFault_Handler");
	while(1){};
}

void DebugMon_Handler(void){
	PrintError("DebugMon_Handler");
	while(1){};
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic
#include "osObjects.h"
 
static void IWDG_Start(void){
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
//	RCC_ClocksTypeDef RCC_ClockFreq;
//	RCC_GetClocksFreq(&RCC_ClockFreq);
	
	/* initialize CMSIS-RTOS */
  osKernelInitialize();
	
	/* Watchdog timer init */
	IWDG_Start();
	
  /* create 'thread' functions that start executing */
	Init_Thread();
	
	/* start thread execution */
  osKernelStart();
}

u32 flash_read_u32(u32 address) {
  return ( * (volatile u32 * ) address);
}

u32 FLASH_ReadData(u32 ADDR) { //set flg = 0 in app
  u32 Result = 0x00;

  Result = flash_read_u32(ADDR);
	
  /* Return Application Program */
  return Result;
}

void FLASH_WriteData(u32 ADDR, u32 Data, uint8_t reset) {
  /* Wait for last operation to be completed */
  FLASH_Status status = FLASH_COMPLETE;
  status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
  if (status != FLASH_COMPLETE) {
    return;
  }

  NVIC_DisableIRQ(USART1_IRQn);

  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  FLASH_Unlock();

  /* Erase page */
  if (FLASH_ErasePage(ADDR) != FLASH_COMPLETE) {
    FLASH_Lock();
    NVIC_EnableIRQ(USART1_IRQn);
    return;
  }

  /* Write data */
  FLASH_ProgramWord(ADDR, Data);

  FLASH_Lock();
  NVIC_EnableIRQ(USART1_IRQn);
  if (reset) {
    osDelay(1000);
    NVIC_SystemReset();
  }
}

void assert_failed(uint8_t* file, uint32_t line){
  char text[256];
	
	sprintf(text, "Assert fail at File %s Line %d", file, (int)line);
	
	for(;;);   // hang here (behavior is our your choice)
}

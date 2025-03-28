#include "flash.h"
#include "utils.h"

void disable_irqn(void){
	NVIC_DisableIRQ(USART1_IRQn);
	USART_Cmd(USART1, DISABLE);
	
	NVIC_DisableIRQ(USART2_IRQn);
	USART_Cmd(USART2, DISABLE);
	
	NVIC_DisableIRQ(UART4_IRQn);
	USART_Cmd(UART4, DISABLE);
}

void enable_irqn(void){
	USART_Cmd(USART1, ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);
	
	USART_Cmd(USART2, ENABLE);
	NVIC_EnableIRQ(USART2_IRQn);
	
	USART_Cmd(UART4, ENABLE);
	NVIC_EnableIRQ(UART4_IRQn);
}

u32 FLASH_ReadData(u32 address) {
  return ( * (volatile u32 * ) address);
}

void FLASH_WriteData(u32 ADDR, u32 Data, uint8_t reset){
  /* Wait for last operation to be completed */
  FLASH_Status status = FLASH_COMPLETE;
  status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
  if (status != FLASH_COMPLETE) {
    return;
  }
	
	disable_irqn();
	FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  
  /* Erase page */
  if (FLASH_ErasePage(ADDR) != FLASH_COMPLETE) {
    FLASH_Lock();
    enable_irqn();
    return;
  }
	
  /* Write data */
  FLASH_ProgramWord(ADDR, Data);
	
  FLASH_Lock();
  enable_irqn();
  if (reset) {
    osDelay(1000);
    NVIC_SystemReset();
  }
}

void FLASH_ReadBufData(uint32_t address, uint8_t *data, uint16_t len){
	uint16_t index;
	uint32_t temp = 0;
  for(index = 0; index < len; index++){
		temp = ( * (volatile uint32_t * ) (address+index*4));
		if(temp <= 255){
			data[index] = temp;
		}else{
			data[index] = 0;
			return;
		}
	}
}

FLASH_Status FLASH_WriteBufData(u32 addr, u8* buf, u16 num){
	uint32_t idx  = 0;
	FLASH_Status FLASHStatus = FLASH_COMPLETE;	
	
	/* Wait for last operation to be completed */
	if(FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT) != FLASH_COMPLETE){
		return FLASH_BUSY;
	}
	disable_irqn();
	FLASH_Unlock();	
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
	
	/* Erase Page */	
	FLASHStatus = FLASH_ErasePage(addr + 0xFF);
	if(FLASHStatus != FLASH_COMPLETE){
		FLASH_Lock();
		enable_irqn();
		return FLASHStatus;
	}
	
	/* Write data */
	for(idx = 0; idx < num; idx++){
		/* write to flash */
		FLASHStatus = FLASH_ProgramWord(addr + idx*4, buf[idx]);
		
		if(FLASHStatus != FLASH_COMPLETE){
			FLASH_Lock();
			enable_irqn();
			return FLASHStatus;
		}
	}
	
	/* Re-lock flash */
	FLASH_Lock();
	enable_irqn();
	return FLASHStatus;
}

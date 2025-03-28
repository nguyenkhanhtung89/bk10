#include "flash.h"
#include "utils.h"

void disable_irqn(void){	
	NVIC_DisableIRQ(USART0_IRQn);
	
	NVIC_DisableIRQ(USART1_IRQn);
	
	NVIC_DisableIRQ(UART3_IRQn);
}

void enable_irqn(void){
	NVIC_EnableIRQ(USART0_IRQn);
	
	NVIC_EnableIRQ(USART1_IRQn);
	
	NVIC_EnableIRQ(UART3_IRQn);
}

uint32_t FLASH_ReadData(uint32_t address) {
  return ( * (volatile uint32_t * ) address);
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

void FLASH_WriteData(uint32_t ADDR, uint32_t Data, uint8_t reset){
	disable_irqn();
	fmc_unlock();

	/* clear all pending flags */
	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
	
  /* Erase page */
  if (fmc_page_erase(ADDR) != FMC_READY) {
    fmc_lock();
    enable_irqn();
    return;
  }
	
  /* Write data */
  fmc_word_program(ADDR, Data);
	
  fmc_lock();
  enable_irqn();
  if (reset) {
    osDelay(1000);
    NVIC_SystemReset();
  }
}

void FLASH_WriteBufData(uint32_t ADDR, uint8_t *data, uint16_t len){
	uint16_t index = 0;
	
	disable_irqn();
	fmc_unlock();

	/* clear all pending flags */
	fmc_flag_clear(FMC_FLAG_BANK0_END);
	fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
	fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
	
  /* Erase page */
  if (fmc_page_erase(ADDR) != FMC_READY) {
    fmc_lock();
    enable_irqn();
    return;
  }
	
	/* Write data */
	for(index = 0; index < len; index++){
		fmc_word_program(ADDR + index*4, data[index]);
	}
	
	fmc_lock();
  enable_irqn();
}

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_INTERRUPT_H
#define __DRV_INTERRUPT_H

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"

/* Exported types ------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* flags -------------------------------------------------------------------- */

/** 
 * @brief  SPI Interface pins 
 */ 
#define RFTRANS_95HF_SPI			     						SPI2
#define RFTRANS_95HF_SPI_CLK		    					RCC_APB1Periph_SPI2
#define RFTRANS_95HF_SPI_AF                   GPIO_AF_5

#define POWER_95HF_PIN												GPIO_Pin_2
#define POWER_95HF_GPIO_PORT 									GPIOD
#define POWER_95HF_GPIO_CLK					  				RCC_AHBPeriph_GPIOD

#define RFTRANS_95HF_SPI_SCK_PIN        			GPIO_Pin_13
#define RFTRANS_95HF_SPI_SCK_PinSource      	GPIO_PinSource13
#define RFTRANS_95HF_SPI_SCK_GPIO_PORT   			GPIOB
#define RFTRANS_95HF_SPI_SCK_GPIO_CLK    			RCC_AHBPeriph_GPIOB
	 
#define RFTRANS_95HF_SPI_MISO_PIN        			GPIO_Pin_14
#define RFTRANS_95HF_SPI_MISO_PinSource      	GPIO_PinSource14
#define RFTRANS_95HF_SPI_MISO_GPIO_PORT  			GPIOB
#define RFTRANS_95HF_SPI_MISO_GPIO_CLK   			RCC_AHBPeriph_GPIOB
	 
#define RFTRANS_95HF_SPI_MOSI_PIN        			GPIO_Pin_15
#define RFTRANS_95HF_SPI_MOSI_PinSource      	GPIO_PinSource15
#define RFTRANS_95HF_SPI_MOSI_GPIO_PORT  			GPIOB
#define RFTRANS_95HF_SPI_MOSI_GPIO_CLK   			RCC_AHBPeriph_GPIOB

#define RFTRANS_95HF_SPI_SELECT_PIN           GPIO_Pin_12
#define RFTRANS_95HF_SPI_SELECT_GPIO_PORT     GPIOB
#define RFTRANS_95HF_SPI_SELECT_GPIO_CLK      RCC_AHBPeriph_GPIOB

#define IRQOUT_RFTRANS_95HF_PIN        				GPIO_Pin_15
#define IRQOUT_RFTRANS_95HF_GPIO_PORT    			GPIOA
#define IRQOUT_RFTRANS_95HF_GPIO_CLOCK				RCC_AHBPeriph_GPIOA

// set state on SPI_NSS pin
#define RFTRANS_95HF_SELECT_LOW() 						GPIO_ResetBits(RFTRANS_95HF_SPI_SELECT_GPIO_PORT, RFTRANS_95HF_SPI_SELECT_PIN)
#define RFTRANS_95HF_SELECT_HIGH()  					GPIO_SetBits  (RFTRANS_95HF_SPI_SELECT_GPIO_PORT, RFTRANS_95HF_SPI_SELECT_PIN)
// set state on IRQ_In pin
#define RFTRANS_95HF_IRQIN_LOW() 							GPIO_ResetBits(IRQOUT_RFTRANS_95HF_GPIO_PORT, IRQOUT_RFTRANS_95HF_PIN)
#define RFTRANS_95HF_IRQIN_HIGH()  						GPIO_SetBits  (IRQOUT_RFTRANS_95HF_GPIO_PORT, IRQOUT_RFTRANS_95HF_PIN)

#endif /* __DRV_INTERRUPT_H */

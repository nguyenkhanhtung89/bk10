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
#define RFTRANS_95HF_SPI			     						SPI1
#define RFTRANS_95HF_SPI_CLK		    					RCU_SPI1

#define POWER_95HF_PIN												GPIO_PIN_2
#define POWER_95HF_GPIO_PORT 									GPIOD
#define POWER_95HF_GPIO_CLK					  				RCU_GPIOD

#define RFTRANS_95HF_SPI_SCK_PIN        			GPIO_PIN_13
#define RFTRANS_95HF_SPI_SCK_GPIO_PORT   			GPIOB
	 
#define RFTRANS_95HF_SPI_MISO_PIN        			GPIO_PIN_14
#define RFTRANS_95HF_SPI_GPIO_PORT  					GPIOB
#define RFTRANS_95HF_SPI_GPIO_CLK   					RCU_GPIOB
	 
#define RFTRANS_95HF_SPI_MOSI_PIN        			GPIO_PIN_15
#define RFTRANS_95HF_SPI_MOSI_GPIO_PORT  			GPIOB

#define RFTRANS_95HF_SPI_SELECT_PIN           GPIO_PIN_12
#define RFTRANS_95HF_SPI_SELECT_GPIO_PORT     GPIOB

#define IRQOUT_RFTRANS_95HF_PIN        				GPIO_PIN_15
#define IRQOUT_RFTRANS_95HF_GPIO_PORT    			GPIOA
#define IRQOUT_RFTRANS_95HF_GPIO_CLOCK				RCU_GPIOA

// set state on SPI_NSS pin
#define RFTRANS_95HF_SELECT_LOW() 						gpio_bit_reset(RFTRANS_95HF_SPI_SELECT_GPIO_PORT, RFTRANS_95HF_SPI_SELECT_PIN)
#define RFTRANS_95HF_SELECT_HIGH()  					gpio_bit_set  (RFTRANS_95HF_SPI_SELECT_GPIO_PORT, RFTRANS_95HF_SPI_SELECT_PIN)
// set state on IRQ_In pin
#define RFTRANS_95HF_IRQIN_LOW() 							gpio_bit_reset(IRQOUT_RFTRANS_95HF_GPIO_PORT, IRQOUT_RFTRANS_95HF_PIN)
#define RFTRANS_95HF_IRQIN_HIGH()  						gpio_bit_set(IRQOUT_RFTRANS_95HF_GPIO_PORT, IRQOUT_RFTRANS_95HF_PIN)

#endif /* __DRV_INTERRUPT_H */

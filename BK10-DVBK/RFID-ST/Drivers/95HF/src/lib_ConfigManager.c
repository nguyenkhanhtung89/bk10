#include "lib_ConfigManager.h"

static void ConfigManager_Init( void);
static int8_t ConfigManager_PORsequence( void );

/**
 *	@brief  This function initialize the PICC
 *  @param  None 
 *  @retval None
 */
static void ConfigManager_Init( void){
	/* configure the Serial interface to communicate with the RF transceiver */
	drv95HF_InitializeSPI();
}

/**
 *	@brief  This function sends POR sequence. It is used to initialize chip after a POR.
 *  @param  none
 *  @retval MANAGER_ERRORCODE_PORERROR : the POR sequence doesn't succeded
 *  @retval MANAGER_SUCCESSCODE : chip is ready
 */
static uint8_t u95HFBuffer[RFTRANS_95HF_MAX_BUFFER_SIZE + 3];
static int8_t ConfigManager_PORsequence( void ){
	uint16_t NthAttempt = 5;
	uc8 command[]= {ECHO};
	//uint8_t u95HFBuffer[RFTRANS_95HF_MAX_BUFFER_SIZE + 3];
	memset(u95HFBuffer, 0, RFTRANS_95HF_MAX_BUFFER_SIZE+3);
	/* Power up sequence: Pulse on IRQ_IN to select UART or SPI mode */
	drv95HF_SendIRQINPulse();
	
	drv95HF_ResetSPI();		
	
	do{
		/* send an ECHO command and checks response */
		drv95HF_SendReceive(command, u95HFBuffer);

		if(u95HFBuffer[0]==ECHORESPONSE){
			
			return MANAGER_SUCCESSCODE;	
		}
		drv95HF_ResetSPI();
	}while(u95HFBuffer[0]!=ECHORESPONSE && NthAttempt-- != 0x00);

	return MANAGER_ERRORCODE_PORERROR;
}

uint8_t ST95HF_Init (void){
	/* Initialize HW according to protocol to use */
	ConfigManager_Init();
	
	/* initilialize the RF transceiver */
	if(ConfigManager_PORsequence() != MANAGER_SUCCESSCODE){
		return 0x00;
	}else{
		return 0x01;
	}
}


bool TagScan(uint8_t* TagUID){
	PCD_FieldOff();
	osDelay(10);
	if(ISO15693_GetUID (TagUID) == RESULTOK)	
		return true;
	PCD_FieldOff();
	return false;
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/

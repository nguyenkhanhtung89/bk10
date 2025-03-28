#include "lib_PCD.h"

static uint8_t IsAnAvailableProtocol 		(uint8_t Protocol);

static uint8_t	u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];

/**
 *	@brief  this functions returns PCD_SUCCESSCODE if the protocol is available, otherwise PCD_ERRORCODE_PARAMETER
 *  @param  Protocol : RF protocol (ISO 14443 A or 14443 B or 15 693 or Fellica)
 *  @return PCD_SUCCESSCODE	: the protocol is available
 *  @return PCD_ERRORCODE_PARAMETER : the protocol isn't available
 */
static uint8_t IsAnAvailableProtocol (uint8_t Protocol){
	switch(Protocol){
		case PCD_PROTOCOL_FIELDOFF:
			return PCD_SUCCESSCODE;
		case PCD_PROTOCOL_ISO15693:
			return PCD_SUCCESSCODE;
		case PCD_PROTOCOL_ISO14443A:
			return PCD_SUCCESSCODE;
		case PCD_PROTOCOL_ISO14443B:
			return PCD_SUCCESSCODE;
		case PCD_PROTOCOL_FELICA:
			return PCD_SUCCESSCODE;
		default: return PCD_ERRORCODE_PARAMETER;
	}	
}

/**
 *	@brief  this function send an EOF pulse to a contacless tag
 *  @param  pResponse : pointer on the PCD device reply
 *  @retval PCD_SUCCESSCODE : the function is succesful 
 */
int8_t PCD_SendEOF(uint8_t *pResponse){
	uint8_t DataToSend[] = {SEND_RECEIVE	,0x00};

	drv95HF_SendReceive(DataToSend, pResponse);
	return PCD_SUCCESSCODE;
}

/**
 *	@brief  this functions turns the field off
 *  @param  none
 *  @retval PCD_SUCCESSCODE : the function is succesful 
 */
void PCD_FieldOff( void ){
	uint8_t ParametersByte=0x00;
	uint8_t pResponse[10];

	PCD_ProtocolSelect(0x02, PCD_PROTOCOL_FIELDOFF, &ParametersByte,pResponse);
}

/**
 * @brief  this functions turns the field on
 * @param  none
 * @return none
 */
void PCD_FieldOn ( void ){
	uint8_t ParametersByte=0x00;
	uint8_t pResponse[10];

	PCD_ProtocolSelect(0x02, PCD_PROTOCOL_ISO15693, &ParametersByte,pResponse);

}

/**
 *	@brief  this function send Echo command to the PCD device 
 *  @param  pResponse : pointer on the PCD device reply
 *  @return PCD_SUCCESSCODE 
 */
int8_t PCD_Echo(uint8_t *pResponse){
	uint8_t command[]= {ECHO};
	
	drv95HF_SendReceive(command, pResponse);
	return PCD_SUCCESSCODE;
}

/**
 *	@brief  this function send a ProtocolSeclect command to the PCD device
 *  @param  Length  : number of byte of protocol select command parameters
 *  @param  Protocol : RF protocol (ISO 14443 A or B or 15 693 or Fellica)
 *  @param  Parameters: prococol parameters (see reader datasheet)
 *  @param  pResponse : pointer on the PCD device response
 *  @return PCD_SUCCESSCODE : the command was succedfully send
 *  @return PCD_ERRORCODE_PARAMETERLENGTH : the Length parameter is erroneous
 *  @return PCD_ERRORCODE_PARAMETER : a parameter is erroneous
 */
int8_t PCD_ProtocolSelect(uint8_t Length,uint8_t Protocol,uint8_t *Parameters,uint8_t *pResponse){
//	uint8_t	u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];
	uint8_t DataToSend[SELECT_BUFFER_SIZE];
	int8_t	status; 
	memset(u95HFBuffer, 0, RFTRANS_95HF_MAX_BUFFER_SIZE+3);
	
	/* check ready to receive Protocol select command */
	PCD_Echo(u95HFBuffer);
	
	if (u95HFBuffer[0] != ECHORESPONSE){
		/* reset the device */
		PCD_PORsequence( );
	}

		/* initialize the result code to 0xFF and length to in case of error 	*/
	*pResponse = PCD_ERRORCODE_DEFAULT;
	*(pResponse+1) = 0x00;
	
	/* check the function parameters */
	errchk(IsAnAvailableProtocol(Protocol));

	DataToSend[PCD_COMMAND_OFFSET ] = PROTOCOL_SELECT;
	DataToSend[PCD_LENGTH_OFFSET  ]	= Length;
	DataToSend[PCD_DATA_OFFSET    ]	= Protocol;

	/* DataToSend CodeCmd Length Data	*/
	/* Parameters[0] first byte to emmit	*/
	memcpy(&(DataToSend[PCD_DATA_OFFSET +1]),Parameters,Length-1);
	
	/* Send the command the Rf transceiver	*/
 	drv95HF_SendReceive(DataToSend, pResponse);

	return PCD_SUCCESSCODE;

Error:
	return PCD_ERRORCODE_PARAMETER;	
}


/**
 *	@brief  this function send a SendRecv command to the PCD device. the command to contacless device is embeded in Parameters.
 *  @param  Length 		: Number of bytes
 *  @param	Parameters 	: data depenps on protocl selected
 *  @param  pResponse : pointer on the PCD device response
 *  @return PCD_SUCCESSCODE : the command was succedfully send
 *  @return PCD_ERRORCODE_DEFAULT : the PCD device returned an error code
 *  @return PCD_ERRORCODE_PARAMETERLENGTH : Length parameter is erroneous
 */
int8_t PCD_SendRecv(uint8_t Length,uint8_t *Parameters,uint8_t *pResponse){
	uint8_t DataToSend[SENDRECV_BUFFER_SIZE];

	/*initialize the result code to 0xFF and length to 0 */
	*pResponse = PCD_ERRORCODE_DEFAULT;
	*(pResponse+1) = 0x00;
	
	/* check the function parameters	*/
	if (CHECKVAL (Length,1,255)==false)
		return PCD_ERRORCODE_PARAMETERLENGTH; 
	
	DataToSend[PCD_COMMAND_OFFSET ] = SEND_RECEIVE;
	DataToSend[PCD_LENGTH_OFFSET  ]	= Length;
	
	/* DataToSend CodeCmd Length Data*/
	/* Parameters[0] first byte to emmit	*/
	memcpy(&(DataToSend[PCD_DATA_OFFSET]),Parameters,Length);
	
	/* Send the command the Rf transceiver	*/
	drv95HF_SendReceive(DataToSend, pResponse);
	
	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE){
			if(*pResponse == PCD_ERRORCODE_NOTAGFOUND)
				return PCD_ERRORCODE_TIMEOUT;
			else
				return PCD_ERRORCODE_DEFAULT;
	}
	return PCD_SUCCESSCODE;
}

/**
 *	@brief  this function send a WriteRegister command to the PCD device
 *  @param  Length 		: Number of bytes of WrReg parameters
 *  @param	Address		: address of first register to write
 *  @param	pData 		: pointer data to be write
 *  @param	Flags		: whether to increment address after register read
 *  @param  pResponse : pointer on the PCD device response
 *  @return PCD_SUCCESSCODE : the command was succedfully send
 *  @return PCD_ERRORCODE_PARAMETERLENGTH : Length parameter is erroneous
 */
int8_t PCD_WriteRegister (uint8_t Length,uint8_t Address,uint8_t Flags,uint8_t *pData,uint8_t *pResponse){
	uint8_t DataToSend[WRREG_BUFFER_SIZE];

	DataToSend[PCD_COMMAND_OFFSET ] = WRITE_REGISTER;
	DataToSend[PCD_LENGTH_OFFSET  ]	= Length;
	DataToSend[PCD_DATA_OFFSET  ]	= Address;
	DataToSend[PCD_DATA_OFFSET +1 ]	= Flags;

	/* Parameters[0] first byte to emmit	*/
	memcpy(&(DataToSend[PCD_DATA_OFFSET + 2]),pData,Length - 2 );
	
	/* Send the command the Rf transceiver	*/
	drv95HF_SendReceive(DataToSend, pResponse);
	return PCD_SUCCESSCODE;
}


/**
 *	@brief  This function sends POR sequence. It can be use to initialize the PCD device after a POR.
 *  @param  none
 *  @return PCD_ERRORCODE_PORERROR : the POR sequence doesn't succeded
 *  @return PCD_SUCCESSCODE : the RF transceiver is ready
 */
int8_t PCD_PORsequence( void ){
//	uint8_t	u95HFBuffer [RFTRANS_95HF_MAX_BUFFER_SIZE+3];
	uint16_t NthAttempt=0;
	memset(u95HFBuffer, 0, RFTRANS_95HF_MAX_BUFFER_SIZE+3);
	drv95HF_ResetSPI();		
	
	do{
		/* send an ECHO command and checks the PCD device response */
		PCD_Echo(u95HFBuffer);
		if (u95HFBuffer[0]==ECHORESPONSE)
			return PCD_SUCCESSCODE;	

		drv95HF_ResetSPI();				
		
		osDelay(10);
		
	} while (u95HFBuffer[0]!=ECHORESPONSE && NthAttempt++ <5);

return PCD_ERRORCODE_PORERROR;
}

/**
*	 @brief  this function send a command to the PCD device device over SPI or UART bus and receive its response.
*	 @brief  the returned value is PCD_SUCCESSCODE 	
*  @param  *pCommand  : pointer on the buffer to send to the the PCD device ( Command | Length | Data)
*  @param  *pResponse : pointer on the the PCD device response ( Command | Length | Data)
*  @return PCD_SUCCESSCODE : the the PCD device returns an success code
*  @return PCD_ERRORCODE_DEFAULT : the the PCD device returns an error code
 */
int8_t PCD_CheckSendReceive(uint8_t *pCommand, uint8_t *pResponse) 
{
	
	drv95HF_SendReceive(pCommand, pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != PCD_SUCCESSCODE)
		return PCD_ERRORCODE_DEFAULT;

	return PCD_SUCCESSCODE;
}

/**  
* @brief  	this function returns PCD_SUCCESSCODE is the reader reply is a succesful code.
* @param  	CmdCode		:  	code command send to the reader
* @param  	ReaderReply	:  	pointer on the PCD device response	
* @retval  	PCD_SUCCESSCODE :  the PCD device returned a succesful code
* @retval  	PCD_ERRORCODE_DEFAULT  :  the PCD device didn't return a succesful code
* @retval  	PCD_NOREPLY_CODE : no the PCD device response
*/
uint8_t PCD_IsReaderResultCodeOk (uint8_t CmdCode,uint8_t *ReaderReply){
	if (ReaderReply[READERREPLY_STATUSOFFSET] == PCD_ERRORCODE_DEFAULT)
		return PCD_NOREPLY_CODE;

  switch (CmdCode){
		case ECHO: 
			if (ReaderReply[PSEUDOREPLY_OFFSET] == ECHO)
				return PCD_SUCCESSCODE;
			else 
				return PCD_ERRORCODE_DEFAULT;
		case IDN: 
			if (ReaderReply[READERREPLY_STATUSOFFSET] == IDN_RESULTSCODE_OK)
				return PCD_SUCCESSCODE;
			else 
				return PCD_ERRORCODE_DEFAULT;
		case PROTOCOL_SELECT: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case IDN_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
				case PROTOCOLSELECT_ERRORCODE_CMDLENGTH :
					return PCD_ERRORCODE_DEFAULT;
				case PROTOCOLSELECT_ERRORCODE_INVALID :
					return PCD_ERRORCODE_DEFAULT;
				default : return PCD_ERRORCODE_DEFAULT;
			}
		case SEND_RECEIVE: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case SENDRECV_RESULTSCODE_OK :
					if (ReaderReply[READERREPLY_STATUSOFFSET+1] != 0)
						return PCD_SUCCESSCODE;
					else
						return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_RESULTSRESIDUAL :
					return PCD_SUCCESSCODE;
				case SENDRECV_ERRORCODE_COMERROR :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_FRAMEWAIT :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_SOF :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_OVERFLOW :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_FRAMING :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_EGT :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_LENGTH :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_CRC :
					return PCD_ERRORCODE_DEFAULT;
				case SENDRECV_ERRORCODE_RECEPTIONLOST :
					return PCD_ERRORCODE_DEFAULT;
				default :
					return PCD_ERRORCODE_DEFAULT;
			}			
		case IDLE: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case IDLE_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
				case IDLE_ERRORCODE_LENGTH :
					return PCD_ERRORCODE_DEFAULT;
				default : return PCD_ERRORCODE_DEFAULT;
			}
		case READ_REGISTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case READREG_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
				case READREG_ERRORCODE_LENGTH :
					return PCD_ERRORCODE_DEFAULT;
				default : return PCD_ERRORCODE_DEFAULT;
			}
		case WRITE_REGISTER: 
			switch (ReaderReply[READERREPLY_STATUSOFFSET])
			{
				case WRITEREG_RESULTSCODE_OK :
					return PCD_SUCCESSCODE;
				default : return PCD_ERRORCODE_DEFAULT;
			}
		case BAUD_RATE: 
			return PCD_ERRORCODE_DEFAULT;
		default: 
			return ERRORCODE_GENERIC;
	}
}

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/


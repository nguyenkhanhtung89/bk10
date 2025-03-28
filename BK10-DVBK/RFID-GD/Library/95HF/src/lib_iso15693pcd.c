#include "cmsis_os.h"
#include "lib_iso15693pcd.h"

/* Variables for the different modes */

/* Get functions --- */
static int8_t ISO15693_GetSelectOrAFIFlag (uint8_t FlagsByte);
static int8_t ISO15693_GetAddressOrNbSlotsFlag (uint8_t FlagsByte);
static int8_t ISO15693_GetOptionFlag (uint8_t FlagsByte);
static int8_t ISO15693_GetProtocolExtensionFlag (uint8_t FlagsByte);
/* Invotory functions --- */
static int8_t ISO15693_Inventory ( uint8_t Flags , uint8_t AFI, uint8_t MaskLength, uint8_t *MaskValue, uint8_t *pResponse);
/* Command functions --- */
static int8_t ISO15693_CreateRequestFlag (uint8_t SubCarrierFlag,uint8_t DataRateFlag,uint8_t InventoryFlag,uint8_t ProtExtFlag,uint8_t SelectOrAFIFlag,uint8_t AddrOrNbSlotFlag,uint8_t OptionFlag,uint8_t RFUFlag);
static int8_t ISO15693_ReadSingleBlock ( uint8_t Flags, uint8_t *UID, uint16_t BlockNumber,uint8_t *pResponse );
static int8_t ISO15693_WriteSingleBlock ( uint8_t Flags, uint8_t *UIDin, uint16_t BlockNumber,uint8_t *DataToWrite,uint8_t *pResponse );
static int8_t ISO15693_ReadMultipleBlock (uint8_t Flags, uint8_t *UIDin, uint16_t BlockNumber, uint8_t NbBlock, uint8_t *pResponse );
static int8_t ISO15693_SendEOF ( uint8_t *pResponse );
/* Is functions --- */
static int8_t ISO15693_IsInventoryFlag (uint8_t FlagsByte);

/* Tag functions --- */
static uint8_t ISO15693_ReadMultipleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read);
static uint8_t ISO15693_ReadSingleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read);
static uint8_t ISO15693_TagSave(uint8_t Tag_Density, uint16_t NbByte_To_Write, uint16_t FirstByte_To_Write, uint8_t *Data_To_Save, uint8_t *Length_Low_Limit, uint8_t *Length_High_Limit);
static uint8_t ISO15693_WriteTagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBlock_To_Write, uint16_t FirstBlock_To_Write);

/**  
* @brief  	this function returns Select Or AFI flag  (depending on inventory flag)
* @param  	FlagsByte	: Request flags on one byte	
* @retval 	Select Or AFI
*/
static int8_t ISO15693_GetSelectOrAFIFlag (uint8_t FlagsByte){
	if ((FlagsByte & ISO15693_MASK_SELECTORAFIFLAG) != 0x00)
		return true ;
	else
		return false ;
}

/**  
* @brief  	this function returns address Or Number of slots flag  (depending on inventory flag)
* @param  	FlagsByte	: Request flags on one byte	
* @retval 	address Or Number of slots
*/
static int8_t ISO15693_GetAddressOrNbSlotsFlag (uint8_t FlagsByte){
	if ((FlagsByte & ISO15693_MASK_ADDRORNBSLOTSFLAG) != 0x00)
		return true ;
	else
		return false ;
}

/**  
* @brief  this function returns Option flag  (depending on inventory flag)
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Option flag
*/
static int8_t ISO15693_GetOptionFlag (uint8_t FlagsByte){
	if ((FlagsByte & ISO15693_MASK_OPTIONFLAG) != 0x00)
		return true ;
	else
		return false ;
}

/**  
* @brief  this function returns Option flag  (depending on inventory flag)
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Option flag
*/
static int8_t ISO15693_GetProtocolExtensionFlag (uint8_t FlagsByte){
	if ((FlagsByte & ISO15693_MASK_PROTEXTFLAG) != 0x00)
		return true ;
	else
		return false ;
}

/**  
* @brief  this function return a Byte, which is concatenation of iventory flags 
* @param  SubCarrierFlag	:  
* @param	DataRateFlag
* @param	InventoryFlag
* @param	ProtExtFlag
* @param	SelectOrAFIFlag
* @param	AddrOrNbSlotFlag
* @param	OptionFlag
* @param	RFUFlag
* @retval 	Flags byte
*/
static int8_t ISO15693_CreateRequestFlag (uint8_t SubCarrierFlag,uint8_t DataRateFlag,uint8_t InventoryFlag,uint8_t ProtExtFlag,uint8_t SelectOrAFIFlag,uint8_t AddrOrNbSlotFlag,uint8_t OptionFlag,uint8_t RFUFlag){
	int32_t FlagsByteBuf=0;
	FlagsByteBuf = 	(SubCarrierFlag 	& 0x01)					|
									((DataRateFlag  	& 0x01)	<< 1)		|
									((InventoryFlag 	& 0x01) << 2)		|
									((ProtExtFlag		& 0x01)	<< 3)			|
									((SelectOrAFIFlag   & 0x01)	<< 4)	|
									((AddrOrNbSlotFlag  & 0x01)	<< 5)	|
									((OptionFlag  		& 0x01) << 6)		|
									((RFUFlag  			& 0x01) << 7);

	return (int8_t) FlagsByteBuf; 
}

/**  
* @brief  this function send an inventory command to a contacless tag.  
* @param  Flags		:  	Request flags
* @param	AFI			:	AFI byte (optional)
* @param	MaskLength 	: 	Number of bits of mask value
* @param	MaskValue	:  	mask value which is compare to Contacless tag UID 
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_PARAMETERLENGTH	: 	MaskLength value is erroneous
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_Inventory( uint8_t Flags , uint8_t AFI, uint8_t MaskLength, uint8_t *MaskValue, uint8_t *pResponse ){
	uint8_t NthByte = 0, InventoryBuf [ISO15693_MAXLENGTH_INVENTORY],	NbMaskBytes = 0, NbSignificantBits=0;
	int8_t 	FirstByteMask, NthMaskByte = 0, status;
	
	// initialize the result code to 0xFF and length to 0  in case of error 
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;

	if(MaskLength>ISO15693_NBBITS_MASKPARAMETER)
		return ISO15693_ERRORCODE_PARAMETERLENGTH;

	errchk(ISO15693_IsInventoryFlag (Flags));
	
	InventoryBuf[NthByte++] = Flags;
	InventoryBuf[NthByte++] = ISO15693_CMDCODE_INVENTORY;
	
	if (ISO15693_GetSelectOrAFIFlag (Flags) == true)
		InventoryBuf[NthByte++] = AFI;
	
	InventoryBuf[NthByte++] = MaskLength;

	if(MaskLength !=0){
		// compute the number of bytes of mask value(2 border exeptions)
	   	if (MaskLength == 64)
			NbMaskBytes = 8;
		else
	   		NbMaskBytes = MaskLength / 8 + 1;
	
		NbSignificantBits = MaskLength - (NbMaskBytes-1) * 8;
		if (NbSignificantBits !=0)
	   		FirstByteMask = (0x01 <<NbSignificantBits)-1;
		else 
			FirstByteMask = 0xFF;
	
	   	// copy the mask value 
		if (NbMaskBytes >1){
			for (NthMaskByte = 0; NthMaskByte < NbMaskBytes - 1; NthMaskByte ++ )
				InventoryBuf[NthByte++] = MaskValue[NthMaskByte];
			osDelay(1);
		}
	
		if (NbSignificantBits !=0)
			InventoryBuf[NthByte++] = MaskValue[NthMaskByte] & FirstByteMask;
	}

 	errchk(PCD_SendRecv(NthByte,InventoryBuf,pResponse));

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != ISO15693_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
Error:
	return ISO15693_ERRORCODE_DEFAULT;
}

/**  
* @brief  	this function send an ReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param		UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param		BlockNumber	:  	index of block to read
* @param		pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ReadSingleBlock (uint8_t Flags, uint8_t *UIDin, uint16_t BlockNumber,uint8_t *pResponse){
	uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK], NthByte=0;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_READSINGLEBLOCK;

	if(ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true){	
		memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	if(ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else{
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;	
	}

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if(PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}

/**  
* @brief  	this function send an WriteSingleblock command and returns ISO15693_SUCCESSCODE if the command 
* @brief  	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  	Flags		:  	Request flags
* @param		UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param		BlockNumber	:  	index of block to write
* @param		BlockLength :	Nb of byte of block length
* @param		DataToWrite :	Data to write into contacless tag memory
* @param		pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_WriteSingleBlock(uint8_t Flags, uint8_t *UIDin, uint16_t BlockNumber,uint8_t *DataToWrite,uint8_t *pResponse ){
	uint8_t DataToSend[MAX_BUFFER_SIZE], NthByte=0, BlockLength = ISO15693_NBBYTE_BLOCKLENGTH;

	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_WRITESINGLEBLOCK;

	if(ISO15693_GetAddressOrNbSlotsFlag (Flags) == true){
		memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	if(ISO15693_GetProtocolExtensionFlag (Flags) 	== false)
		DataToSend[NthByte++] = BlockNumber;
	else{
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
	}
	
	memcpy(&(DataToSend[NthByte]),DataToWrite,BlockLength);
	NthByte +=BlockLength;

	if(ISO15693_GetOptionFlag (Flags) == false)
		PCD_SendRecv(NthByte,DataToSend,pResponse);
	else{
		PCD_SendRecv(NthByte,DataToSend,pResponse);
	 	osDelay(20);
		ISO15693_SendEOF (pResponse);
	}	

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}

/**  
* @brief  this function send an ReadSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_ReadMultipleBlock (uint8_t Flags, uint8_t *UIDin, uint16_t BlockNumber, uint8_t NbBlock, uint8_t *pResponse ){
	uint8_t DataToSend[ISO15693_MAXLENGTH_READSINGLEBLOCK];
	uint8_t		NthByte=0;
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_READMULBLOCKS;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true){
		memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}
	BlockNumber=BlockNumber<<5; // *32
	if (ISO15693_GetProtocolExtensionFlag (Flags) == false){
		DataToSend[NthByte++] = BlockNumber;
	}
	else{
		DataToSend[NthByte++] = BlockNumber & 0x00FF;
		DataToSend[NthByte++] = (BlockNumber & 0xFF00 ) >> 8;
	}
	
	DataToSend[NthByte++] = NbBlock;

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == PCD_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}

/**  
* @brief  	this function send an EOF pulse to contactless tag.
* @param	pResponse	: 	pointer on PCD  response
* @retval 	ISO15693_SUCCESSCODE	: 	PCD  returns a succesful code
* @retval 	ISO15693_ERRORCODE_DEFAULT	: 	 PCD  returns an error code
*/
static int8_t ISO15693_SendEOF(uint8_t *pResponse ){
	PCD_SendEOF(pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) != ISO15693_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;
		
	return ISO15693_SUCCESSCODE;

}

/**  
* @brief  	this function returns ISO15693_SUCCESSCODE is Inventorye flag is set
* @param  	FlagsByte	: the byts cantaining the eight flags  	
* @retval 	Inventory flag
*/
static int8_t ISO15693_IsInventoryFlag (uint8_t FlagsByte){
	if ((FlagsByte & ISO15693_MASK_INVENTORYFLAG) != 0x00)
		return ISO15693_SUCCESSCODE ;
	else
		return ISO15693_ERRORCODE_DEFAULT ;
}


/**
* @brief  Save the data from a TAG block for the uncomplete block to write
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  NbByte_To_Write : Numbre of Byte to write in the TAG
* @param  FirstByte_To_Write : First Byte to write in the TAG
* @param  *Data_To_Save : Data store before write the complete block of the TAG
* @param  *Length_Low_Limit : Number of Byte save for the first block to write (0 =< Length_Low_Limit =< 4)
* @param  *Length_High_Limit : Number of Byte save for the last block to write (0 =< Length_High_Limit =< 4)	
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_TagSave (uint8_t Tag_Density, uint16_t NbByte_To_Write, uint16_t FirstByte_To_Write, uint8_t *Data_To_Save, uint8_t *Length_Low_Limit, uint8_t *Length_High_Limit){
	const uint8_t NbBytePerBlock = 0x04; 
	uint8_t RepBuffer[32],
	ReadSingleBuffer [4]={0x02, 0x20, 0x00, 0x00};

	uint8_t Nb_Byte_To_Save_Low;
	uint8_t Nb_Byte_To_Save_High;
	uint16_t Num_Block_Low;
	uint16_t Num_Block_High;
			
	/**********Find Limit Block*****************/ 
	Num_Block_Low = FirstByte_To_Write / NbBytePerBlock;
	Num_Block_High = (FirstByte_To_Write + NbByte_To_Write) / NbBytePerBlock;
		
	Nb_Byte_To_Save_Low	= (FirstByte_To_Write) % NbBytePerBlock;
	Nb_Byte_To_Save_High	= 4-((FirstByte_To_Write + NbByte_To_Write/*+ Nb_Byte_To_Save_Low*/) % NbBytePerBlock); 

	/**********Read Low Limit******************/
	if(Tag_Density == ISO15693_LOW_DENSITY){
		ReadSingleBuffer[2] = Num_Block_Low;
		PCD_SendRecv (0x03,ReadSingleBuffer,RepBuffer);
	}
	
	else if(Tag_Density == ISO15693_HIGH_DENSITY){
		 ReadSingleBuffer[0] = 0x0A;
		 ReadSingleBuffer[2] =  Num_Block_Low & 0x00FF;
		 ReadSingleBuffer[3] = (Num_Block_Low & 0xFF00) >> 8;
		 PCD_SendRecv (0x04,ReadSingleBuffer,RepBuffer);
	}
	
	if(RepBuffer[0] != 0x80)
		return ISO15693_ERRORCODE_DEFAULT;
	
	memcpy(&Data_To_Save[0], &RepBuffer[3], Nb_Byte_To_Save_Low);
	*Length_Low_Limit = Nb_Byte_To_Save_Low;
	
	/**********Read High Limit******************/		 		
	if(Tag_Density == ISO15693_LOW_DENSITY){	
		ReadSingleBuffer[2] = Num_Block_High;
		PCD_SendRecv (0x03,ReadSingleBuffer,RepBuffer);
	}
	
	else if(Tag_Density == ISO15693_HIGH_DENSITY){
		ReadSingleBuffer[0] = 0x0A;
		ReadSingleBuffer[2] =  Num_Block_High & 0x00FF;
		ReadSingleBuffer[3] = (Num_Block_High & 0xFF00) >> 8;
		PCD_SendRecv (0x04,ReadSingleBuffer,RepBuffer);	
	}
	
	if(RepBuffer[0] != 0x80)
		return ISO15693_ERRORCODE_DEFAULT;
	
	/*Data temp*/
	memcpy(&Data_To_Save[4], &RepBuffer[3+(4-Nb_Byte_To_Save_High)], (Nb_Byte_To_Save_High));
	*Length_High_Limit = Nb_Byte_To_Save_High;
	return ISO15693_SUCCESSCODE;
}

/**
* @brief  Read Multiple Block in the TAG (sector size : 0x20 = 32 Blocks)
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBlock_To_Read : Number of block to read in the TAG
* @param  FirstBlock_To_Read : First block to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/
static uint8_t ISO15693_ReadMultipleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read){
	uint8_t RepBuffer[140];
	uint8_t	Requestflags = 0x02;
	uint16_t NbSectorToRead = 0;
	uint16_t SectorStart = 0;
	uint16_t NthDataToRead;
	
	/*Convert the block number in sector number*/
  NbSectorToRead = NbBlock_To_Read / 32 + 1;	
	SectorStart = FirstBlock_To_Read / 32;
	
	if(Tag_Density == ISO15693_HIGH_DENSITY)
		Requestflags = 0x0A;	
	/*else
		return ISO15693_ERRORCODE_DEFAULT;*/
	
	for (NthDataToRead = 0; NthDataToRead < NbSectorToRead; NthDataToRead++){
		//NumSectorToRead += NthDataToRead;

		if (ISO15693_ReadMultipleBlock(Requestflags, 0x00, NthDataToRead+SectorStart, 0x1F, RepBuffer) != ISO15693_SUCCESSCODE)
					return ISO15693_ERRORCODE_DEFAULT;
		/*Data Temp*/
		memcpy(&Data_To_Read[NthDataToRead * 128], &RepBuffer[3], 128);
		osDelay(20);
	}
				
	return ISO15693_SUCCESSCODE;
}

/**
* @brief  Read Single Block in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBlock_To_Read : Number of block to read in the TAG
* @param  FirstBlock_To_Read : First block to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_ReadSingleTagData(uint8_t Tag_Density, uint8_t *Data_To_Read, uint16_t NbBlock_To_Read, uint16_t FirstBlock_To_Read){
	uint8_t RepBuffer[16];
	uint8_t Requestflags = 0x02;
	uint16_t NthDataToRead= 0x0000;
	uint16_t Num_DataToRead = FirstBlock_To_Read;

		// update the RequestFlags
		if (Tag_Density == ISO15693_LOW_DENSITY)
			Num_DataToRead &=  0x00FF;
		else if (Tag_Density == ISO15693_HIGH_DENSITY)
			Requestflags = 0x0A;	
		else
			return ISO15693_ERRORCODE_DEFAULT;

	for(NthDataToRead= 0; NthDataToRead < NbBlock_To_Read; NthDataToRead++){
		Num_DataToRead += NthDataToRead;
		
		if(ISO15693_ReadSingleBlock (Requestflags, 0x00, Num_DataToRead,RepBuffer) != ISO15693_SUCCESSCODE)
			return ISO15693_ERRORCODE_DEFAULT;
			
		/*Data Temp*/
		memcpy(&Data_To_Read[NthDataToRead * 4], &RepBuffer[3], ISO15693_NBBYTE_BLOCKLENGTH);
		osDelay(20);
	}
				
	return ISO15693_SUCCESSCODE;
}

/**
* @brief  Write data by blocks in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Write : Data to write in the TAG
* @param  NbBlock_To_Write : Number of block to write in the TAG
* @param  FirstBlock_To_Write : First block to write in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
static uint8_t ISO15693_WriteTagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBlock_To_Write, uint16_t FirstBlock_To_Write){
	uint8_t	 RepBuffer[32], Requestflags =0x02;
	uint16_t NthDataToWrite =0, IncBlock =0;

	// update the RequestFlags
	if (Tag_Density == ISO15693_LOW_DENSITY)
		FirstBlock_To_Write &=  0x00FF;
	else if (Tag_Density == ISO15693_HIGH_DENSITY)
		Requestflags = 0x0A;	
	else
		return ISO15693_ERRORCODE_DEFAULT;

	for ( NthDataToWrite=FirstBlock_To_Write; NthDataToWrite<(FirstBlock_To_Write+NbBlock_To_Write); NthDataToWrite++){
		if ( ISO15693_WriteSingleBlock (Requestflags, 0x00,NthDataToWrite,&Data_To_Write[NthDataToWrite<<2],RepBuffer ) !=ISO15693_SUCCESSCODE)
			return ISO15693_ERRORCODE_DEFAULT;
	 
	  IncBlock += 4;
		osDelay(20);
	}
					
	return ISO15693_SUCCESSCODE;				
}	

/**  
* @brief  this function selects 15693 protocol accoording to input parameters
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
int8_t ISO15693_Init( void ){
	uint8_t 	ParametersByte = 0, pResponse[PROTOCOLSELECT_LENGTH];
	int8_t		status;
	uint8_t DemoGainParameters[]  = {PCD_TYPEV_ARConfigA, PCD_TYPEV_ARConfigB}; 

	ParametersByte =  	((ISO15693_APPENDCRC << ISO15693_OFFSET_APPENDCRC ) 	&  ISO15693_MASK_APPENDCRC) |
											((ISO15693_SINGLE_SUBCARRIER << ISO15693_OFFSET_SUBCARRIER)	& ISO15693_MASK_SUBCARRIER)	|
											((ISO15693_MODULATION_100 << ISO15693_OFFSET_MODULATION) & ISO15693_MASK_MODULATION) |
											((ISO15693_WAIT_FOR_SOF <<  ISO15693_OFFSET_WAITORSOF ) & ISO15693_MASK_WAITORSOF) 	|
											((ISO15693_TRANSMISSION_26 <<   ISO15693_OFFSET_DATARATE  )	& ISO15693_MASK_DATARATE);
	
	errchk(PCD_ProtocolSelect(ISO15693_SELECTLENGTH,ISO15693_PROTOCOL,&(ParametersByte),pResponse));

	/* in order to adjust the demoduation gain of the PCD*/
  errchk(PCD_WriteRegister    ( 0x04,AFE_ANALOG_CONF_REG_SELECTION,0x01,DemoGainParameters,pResponse)); 

	if (PCD_IsReaderResultCodeOk (PROTOCOL_SELECT,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;
	
	return ISO15693_SUCCESSCODE;
Error:
	return ISO15693_ERRORCODE_DEFAULT;
}

/**  
* @brief  this function return a tag UID 
* @param  	UIDout: 	UID of a tag in the field
* @retval status function
*/
int8_t ISO15693_GetUID (uint8_t *UIDout){
	int8_t 	FlagsByteData, status;
	uint8_t	TagReply	[ISO15693_NBBYTE_UID+7];
	uint8_t Tag_error_check;
	
	/* select 15693 protocol */
	errchk(ISO15693_Init	(	));

	FlagsByteData = ISO15693_CreateRequestFlag 	(	ISO15693_REQFLAG_SINGLESUBCARRIER,
													ISO15693_REQFLAG_HIGHDATARATE,
													ISO15693_REQFLAG_INVENTORYFLAGSET,
													ISO15693_REQFLAG_NOPROTOCOLEXTENSION,
													ISO15693_REQFLAG_NOTAFI,
													ISO15693_REQFLAG_1SLOT,
													ISO15693_REQFLAG_OPTIONFLAGNOTSET,
													ISO15693_REQFLAG_RFUNOTSET);
	
	errchk( status = ISO15693_Inventory (	FlagsByteData,
						 	0x00, 
						 	0x00,
							0x00,
							TagReply ));

	Tag_error_check = TagReply[ISO15693_OFFSET_LENGTH]+1;
	if((TagReply[Tag_error_check] & ISO15693_CRC_MASK) != 0x00 )
		return ISO15693_ERRORCODE_DEFAULT;
	

	if (status == ISO15693_SUCCESSCODE)
		memcpy(UIDout,&(TagReply[ISO15693_OFFSET_UID]),ISO15693_NBBYTE_UID);
	return ISO15693_SUCCESSCODE; 
Error:
	return ISO15693_ERRORCODE_DEFAULT;	
}

/**  
* @brief  this function send an GetSystemInfo command and returns ISO15693_SUCCESSCODE if the command 
* @brief	was correctly emmiting, ISO15693_ERRORCODE_DEFAULT otherwise
* @param  Flags		:  	inventory flags
* @param	UID			:  	Tag UID
* @retval ISO15693_SUCCESSCODE : the function is successful
* @retval ISO15693_ERRORCODE_DEFAULT : an error occured
*/
int8_t ISO15693_GetSystemInfo ( uint8_t Flags, uint8_t *UIDin, uint8_t *pResponse){
	uint8_t DataToSend[ISO15693_MAXLENGTH_GETSYSTEMINFO], NthByte=0;
	int8_t	status;
			
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_GETSYSINFO;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== true){	
		memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	errchk(PCD_SendRecv(NthByte,DataToSend,pResponse));
	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
Error:
	*pResponse = SENDRECV_ERRORCODE_SOFT;
	*(pResponse+1) = 0x00;
	return ISO15693_ERRORCODE_DEFAULT;
}

/**
* @brief  Find the TAG ISO15693 present in the field
* @param  *Length_Memory_TAG : return the memory size of the TAG
* @param  *Tag_Density : return 0 if the TAG is low density (< 16kbits), return 1 if the TAG is high density
* @param  *IC_Ref_TAG : return the IC_Ref of the TAG ( the 2 LSB bits are mask for the TAG LRiS2K, LRi2K and LRi1K)
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/
int8_t ISO15693_GetTagIdentification (uint16_t *Length_Memory_TAG, uint8_t *Tag_Density, uint8_t *IC_Ref_TAG){
	uint8_t RepBuffer[32],
				RequestFlags = 0x0A;

	uint8_t IC_Ref;
		
		/*Use ISO15693 Protocol*/
		ISO15693_Init();
		
		/*Send Get_System_Info with Protocol Extention Flag Set*/
		if ( ISO15693_GetSystemInfo ( RequestFlags, 0x00, RepBuffer) == ISO15693_SUCCESSCODE)
			IC_Ref = RepBuffer[17];
		else{
				RequestFlags = 0x02;
				if ( ISO15693_GetSystemInfo ( RequestFlags, 0x00, RepBuffer) == ISO15693_SUCCESSCODE)
					IC_Ref = RepBuffer[16];
				else 
					return ISO15693_ERRORCODE_NOTAGFOUND; 	
		}
		
		switch (IC_Ref){
			case ISO15693_M24LR64R :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
			  *Tag_Density = ISO15693_HIGH_DENSITY;
				break;
				
			case ISO15693_M24LR64ER :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
				*Tag_Density = ISO15693_HIGH_DENSITY;
				break;
				
			case ISO15693_M24LR16ER :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
				*Tag_Density = ISO15693_HIGH_DENSITY;
				break;
				
			case ISO15693_M24LR04ER :
				*Length_Memory_TAG = (RepBuffer[14]+1);
				*Tag_Density = ISO15693_LOW_DENSITY;
				break;
				
			case ISO15693_LRiS64K :
				*Length_Memory_TAG = ((((RepBuffer[15] <<8)& 0xFF00) | RepBuffer[14])+1);
				*Tag_Density = ISO15693_HIGH_DENSITY;
				break;

			default :
				/*Flag IC_REF LSB For LRiXX*/
				IC_Ref &= 0xFC;
			
				switch (IC_Ref)
				{		
					case ISO15693_LRiS2K :
						*Length_Memory_TAG = (RepBuffer[14]+1);
						*Tag_Density = ISO15693_LOW_DENSITY;
					break;
				
					case ISO15693_LRi2K :
						*Length_Memory_TAG = (RepBuffer[14]+1);
						*Tag_Density = ISO15693_LOW_DENSITY;
					break;
				
					case ISO15693_LRi1K :
						*Length_Memory_TAG = (RepBuffer[14]+1);
						*Tag_Density = ISO15693_LOW_DENSITY;
					break;
			
					default :
						return ISO15693_ERRORCODE_DEFAULT;
				}
		}
		
		*IC_Ref_TAG = IC_Ref;
		return ISO15693_SUCCESSCODE;
}


/**
* @brief  Read data by Bytes in the TAG (with read multiple if it is possible)
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  IC_Ref_Tag : The IC_Ref is use to use read multiple or read single
* @param  *Data_To_Read : return the data read in the TAG
* @param  NbBytes_To_Read : Number of Bytes to read in the TAG
* @param  FirstBytes_To_Read : First Bytes to read in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	
uint8_t ISO15693_ReadBytesTagData(uint8_t Tag_Density, uint8_t IC_Ref_Tag, uint8_t *Data_To_Read, uint16_t NbBytes_To_Read, uint16_t FirstBytes_To_Read){
	uint8_t status = ISO15693_ERRORCODE_DEFAULT;
  /*0x80 => Page size read multiple 0x20 & 1 block = 4 bytes*/
	uint16_t NbBlock_To_Read = NbBytes_To_Read / 4;
	uint16_t FirstBlock_To_Read = FirstBytes_To_Read / 4;
	
	/*LRiS2K don't support read multiple*/
	if(IC_Ref_Tag  == ISO15693_LRiS2K){
		 NbBlock_To_Read = NbBytes_To_Read / 4;		
		 status = ISO15693_ReadSingleTagData(Tag_Density, Data_To_Read, NbBlock_To_Read, FirstBlock_To_Read);
	}
	else{
		status = ISO15693_ReadMultipleTagData(Tag_Density, Data_To_Read, NbBlock_To_Read, FirstBlock_To_Read);			
	}
	return status;
}

/**
* @brief  Write data by bytes in the TAG
* @param  Tag_Density : TAG is HIGH or LOW density
* @param  *Data_To_Write : Data to write in the TAG
* @param  NbBytes_To_Write : Number of Bytes to write in the TAG
* @param  FirstBytes_To_Write : First Bytes to write in the TAG
* @retval ISO15693_ERRORCODE_DEFAULT / ISO15693_SUCCESSCODE.
*/	 
uint8_t ISO15693_WriteBytes_TagData(uint8_t Tag_Density, uint8_t *Data_To_Write, uint16_t NbBytes_To_Write, uint16_t FirstBytes_To_Write){
	/*1 block = 4 bytes*/
	uint16_t NbBlock_To_Write;

	/*Convert in Blocks the number of bytes to write*/
	uint16_t FirstBlock_To_Write = FirstBytes_To_Write/4;

	uint8_t Length_Low_Limit,
				Length_High_Limit,
				Data_To_Save[8];
	
	NbBlock_To_Write = (NbBytes_To_Write/4)+1;
	
	/*Save the data of the uncomplete block to write*/
	ISO15693_TagSave(Tag_Density, NbBytes_To_Write, FirstBytes_To_Write, Data_To_Save, &Length_Low_Limit, &Length_High_Limit);
	
	
	memcpy(&Data_To_Write[-Length_Low_Limit],&Data_To_Save[0], Length_Low_Limit);
	memcpy(&Data_To_Write[NbBytes_To_Write],&Data_To_Save[4], Length_High_Limit);
	
	if (ISO15693_WriteTagData(Tag_Density, &Data_To_Write[-Length_Low_Limit], NbBlock_To_Write, FirstBlock_To_Write) != ISO15693_SUCCESSCODE)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;	
}

#ifdef ISO15693_ALLCOMMANDS 

/**  
* @brief  	this function send an LockSingleBlock command to contactless tag.
* @param  	Flags		:  	Request flags
* @param	UIDin		:  	pointer on contacless tag UID (optional) (depend on address flag of Request flags)
* @param	BlockNumber	:  	index of block to read
* @param	pResponse	: 	pointer on PCD  response
* @retval 	RESULTOK	: 	PCD  returns a succesful code
* @retval 	ERRORCODE_GENERIC	: 	 PCD  returns an error code
*/
int8_t ISO15693_LockSingleBlock ( uint8_t Flags, uint8_t *UIDin, uint8_t BlockNumber,uint8_t *pResponse){
	uint8_t DataToSend[ISO15693_MAXLENGTH_LOCKSINGLEBLOCK], NthByte=0;
	
	DataToSend[NthByte++] = Flags;
	DataToSend[NthByte++] = ISO15693_CMDCODE_LOCKBLOCK;

	if (ISO15693_GetAddressOrNbSlotsFlag (Flags) 	== TRUE){
		memcpy(&(DataToSend[NthByte]),UIDin,ISO15693_NBBYTE_UID);
		NthByte +=ISO15693_NBBYTE_UID;	
	}

	DataToSend[NthByte++] = BlockNumber;

	PCD_SendRecv(NthByte,DataToSend,pResponse);

	if (PCD_IsReaderResultCodeOk (SEND_RECEIVE,pResponse) == ISO15693_ERRORCODE_DEFAULT)
		return ISO15693_ERRORCODE_DEFAULT;

	return ISO15693_SUCCESSCODE;
}

#endif /*ISO15693_ALLCOMMANDS*/
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/



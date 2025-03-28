#include <ql_oe.h>
#include "lis3dsh.h"

#define I2C_DEV         "/dev/i2c-2"	//i2c-2 on EC20xx, i2c-4 on AG35

int fd_i2c;

static IMU_6AXES_StatusTypeDef LIS3DSH_IO_Write(u8* pBuffer, u8 slaveAddr, u8 writeAddr, u16 NumByteToWrite);

static IMU_6AXES_StatusTypeDef LIS3DSH_IO_Read(u8* pBuffer, u8 slaveAddr, u8 readAddr, u16 NumByteToRead);

static IMU_6AXES_StatusTypeDef    LIS3DSH_ACC_Set_ODR( float odr );

static IMU_6AXES_StatusTypeDef   LIS3DSH_ACC_Set_FS( float fullScale );

static IMU_6AXES_StatusTypeDef LIS3DSH_ACC_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ);

IMU_6AXES_StatusTypeDef LIS3DSH_init(IMU_6AXES_InitTypeDef *LIS3DSH_InitTypeDef){

	fd_i2c = Ql_I2C_Init(I2C_DEV);

	if(LIS3DSH_ACC_Set_ODR( LIS3DSH_InitTypeDef->ACC_OutputDataRate ) != IMU_6AXES_OK)
	{
		return IMU_6AXES_ERROR;
	}

	if(LIS3DSH_ACC_Set_FS( LIS3DSH_InitTypeDef->ACC_FullScale ) != IMU_6AXES_OK)
	{
		return IMU_6AXES_ERROR;
	}

	if(LIS3DSH_ACC_Set_Axes_Status(LIS3DSH_InitTypeDef->ACC_X_Axis, LIS3DSH_InitTypeDef->ACC_Y_Axis, LIS3DSH_InitTypeDef->ACC_Z_Axis) != IMU_6AXES_OK)
	{
		return IMU_6AXES_ERROR;
	}
	return IMU_6AXES_OK;
}

IMU_6AXES_StatusTypeDef    LIS3DSH_ACC_Set_ODR( float odr )
{
  uint8_t new_odr = 0x00;
  uint8_t tempReg = 0x00;

  new_odr = ( odr <= 0.0f   ) ? LIS3DSH_ACC_ODR_PD          /* Power Down */
            : ( odr <= 3.125f ) ? LIS3DSH_ACC_ODR_3_125HZ
            : ( odr <= 6.25f) ? LIS3DSH_ACC_ODR_6_25HZ
            : ( odr <= 12.5f ) ? LIS3DSH_ACC_ODR_12_5HZ
            : ( odr <= 25.0f ) ? LIS3DSH_ACC_ODR_25HZ
            : ( odr <= 50.0f ) ? LIS3DSH_ACC_ODR_50HZ
            : ( odr <= 100.0f ) ? LIS3DSH_ACC_ODR_100HZ
  	  	  	: ( odr <= 400.0f ) ? LIS3DSH_ACC_ODR_400HZ
  	  	  	: ( odr <= 800.0f ) ? LIS3DSH_ACC_ODR_800HZ
  	  	  	:					 LIS3DSH_ACC_ODR_1600HZ;

  if(LIS3DSH_IO_Read( &tempReg, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG4, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  tempReg &= ~(LIS3DSH_ACC_ODR_MASK);
  tempReg |= new_odr;

  if(LIS3DSH_IO_Write(&tempReg, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG4, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  return IMU_6AXES_OK;
}

static IMU_6AXES_StatusTypeDef   LIS3DSH_ACC_Set_FS( float fullScale )
{
  uint8_t new_fs = 0x00;
  uint8_t tempReg = 0x00;

  new_fs = ( fullScale <= 2.0f ) ? LIS3DSH_ACC_FS_2G
           	: ( fullScale <= 4.0f ) ? LIS3DSH_ACC_FS_4G
        	: ( fullScale <= 6.0f ) ? LIS3DSH_ACC_FS_6G
        	: ( fullScale <= 8.0f ) ? LIS3DSH_ACC_FS_8G
        	:                         LIS3DSH_ACC_FS_16G;

  if(LIS3DSH_IO_Read( &tempReg, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG5, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  tempReg &= ~(LIS3DSH_ACC_FS_MASK);
  tempReg |= new_fs;

  if(LIS3DSH_IO_Write(&tempReg, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG5, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  return IMU_6AXES_OK;
}

static IMU_6AXES_StatusTypeDef LIS3DSH_ACC_Set_Axes_Status(uint8_t enableX, uint8_t enableY, uint8_t enableZ)
{
  uint8_t tmp1 = 0x00;
  uint8_t eX = 0x00;
  uint8_t eY = 0x00;
  uint8_t eZ = 0x00;

  eX = ( enableX == 0 ) ? LIS3DSH_ACC_XEN_DISABLE : LIS3DSH_ACC_XEN_ENABLE;
  eY = ( enableY == 0 ) ? LIS3DSH_ACC_YEN_DISABLE : LIS3DSH_ACC_YEN_ENABLE;
  eZ = ( enableZ == 0 ) ? LIS3DSH_ACC_ZEN_DISABLE : LIS3DSH_ACC_ZEN_ENABLE;

  if(LIS3DSH_IO_Read(&tmp1, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG4, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  /* Enable X axis selection */
  tmp1 &= ~(LIS3DSH_ACC_XEN_MASK);
  tmp1 |= eX;

  /* Enable Y axis selection */
  tmp1 &= ~(LIS3DSH_ACC_YEN_MASK);
  tmp1 |= eY;

  /* Enable Z axis selection */
  tmp1 &= ~(LIS3DSH_ACC_ZEN_MASK);
  tmp1 |= eZ;

  if(LIS3DSH_IO_Write(&tmp1, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG4, 1) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  return IMU_6AXES_OK;
}

static IMU_6AXES_StatusTypeDef    LIS3DSH_ACC_GetSensitivity( float *pfData )
{
  /*Here we have to add the check if the parameters are valid*/
  uint8_t tempReg = 0x00;

  if(LIS3DSH_IO_Read( &tempReg, LIS3DSH_MEMS_ADDRESS, LIS3DSH_AD_CTRL_REG5, 1 ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  tempReg &= LIS3DSH_ACC_FS_MASK;

  switch( tempReg )
  {
    case LIS3DSH_ACC_FS_2G:
    	*pfData = 0.06f;
    	break;
    case LIS3DSH_ACC_FS_4G:
    	*pfData = 0.12f;
    	break;
    case LIS3DSH_ACC_FS_6G:
    	*pfData = 0.18f;
    	break;
    case LIS3DSH_ACC_FS_8G:
    	*pfData = 0.244f;
    	break;
    case LIS3DSH_ACC_FS_16G:
    	*pfData = 0.732f;
    	break;
    default:
    	break;
  }

  return IMU_6AXES_OK;
}

IMU_6AXES_StatusTypeDef LIS3DSH_ACC_GetAxesRaw(int16_t *pData)
{
  uint8_t tempReg[2] = {0, 0};

  if(LIS3DSH_IO_Read(&tempReg[0], LIS3DSH_MEMS_ADDRESS, (LIS3DSH_AD_OUT_X_L | LIS3DSH_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  pData[0] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);

  if(LIS3DSH_IO_Read(&tempReg[0], LIS3DSH_MEMS_ADDRESS, (LIS3DSH_AD_OUT_Y_L | LIS3DSH_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  pData[1] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);

  if(LIS3DSH_IO_Read(&tempReg[0], LIS3DSH_MEMS_ADDRESS, (LIS3DSH_AD_OUT_Z_L | LIS3DSH_I2C_MULTIPLEBYTE_CMD),
                     2) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  pData[2] = ((((int16_t)tempReg[1]) << 8) + (int16_t)tempReg[0]);

  return IMU_6AXES_OK;
}


/**
 * @brief  Read data from LSM6DS0 Accelerometer and calculate linear acceleration in mg
 * @param  pData the pointer where the accelerometer data are stored
 * @retval IMU_6AXES_OK in case of success, an error code otherwise
 */
IMU_6AXES_StatusTypeDef LSM6DS0_ACC_GetAxes(int32_t *pData)
{
  int16_t pDataRaw[3];
  float sensitivity = 0;

  if(LIS3DSH_ACC_GetAxesRaw(pDataRaw) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  if(LIS3DSH_ACC_GetSensitivity( &sensitivity ) != IMU_6AXES_OK)
  {
    return IMU_6AXES_ERROR;
  }

  pData[0] = (int32_t)(pDataRaw[0] * sensitivity);
  pData[1] = (int32_t)(pDataRaw[1] * sensitivity);
  pData[2] = (int32_t)(pDataRaw[2] * sensitivity);

  return IMU_6AXES_OK;
}

static IMU_6AXES_StatusTypeDef LIS3DSH_IO_Write(u8* pBuffer, u8 slaveAddr, u8 writeAddr, u16 NumByteToWrite)
{
	s32 ret;
	ret = Ql_I2C_Write(fd_i2c, slaveAddr, writeAddr, pBuffer, NumByteToWrite);
	if(ret < 0){
		return IMU_6AXES_ERROR;
	}
	return IMU_6AXES_OK;
}

static IMU_6AXES_StatusTypeDef LIS3DSH_IO_Read(u8* pBuffer, u8 slaveAddr, u8 readAddr, u16 NumByteToRead)
{
	s32 ret;
	ret = Ql_I2C_Read(fd_i2c, slaveAddr, readAddr, pBuffer, NumByteToRead);
	if(ret < 0)
	{
		return IMU_6AXES_ERROR;
	}
	return IMU_6AXES_OK;
}

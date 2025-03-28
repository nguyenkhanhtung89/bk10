/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LIS3DH_H
#define LIS3DH_H

#include <stdio.h>
#include <stdarg.h>

#define LIS3DSH_MEMS_ADDRESS				0x1D

#define  LIS3DSH_AD_OUT_T                   0x0C
#define  LIS3DSH_AD_INFO1                   0x0D
#define  LIS3DSH_AD_INFO2                   0x0E
#define  LIS3DSH_AD_WHO_AM_I                0x0F
#define  LIS3DSH_AD_OFF_X                   0x10
#define  LIS3DSH_AD_OFF_Y                   0x11
#define  LIS3DSH_AD_OFF_Z                   0x12
#define  LIS3DSH_AD_CS_X                    0x13
#define  LIS3DSH_AD_CS_Y                    0x14
#define  LIS3DSH_AD_CS_Z                    0x15
#define  LIS3DSH_AD_LC_L                    0x16
#define  LIS3DSH_AD_LC_H                    0x17
#define  LIS3DSH_AD_STAT                    0x18
#define  LIS3DSH_AD_PEAK1                   0x19
#define  LIS3DSH_AD_PEAK2                   0x1A
#define  LIS3DSH_AD_VFC_1                   0x1B
#define  LIS3DSH_AD_VFC_2                   0x1C
#define  LIS3DSH_AD_VFC_3                   0x1D
#define  LIS3DSH_AD_VFC_4                   0x1E
#define  LIS3DSH_AD_THRS3                   0x1F
#define  LIS3DSH_AD_CTRL_REG4               0x20
#define  LIS3DSH_AD_CTRL_REG1               0x21
#define  LIS3DSH_AD_CTRL_REG2               0x22
#define  LIS3DSH_AD_CTRL_REG3               0x23
#define  LIS3DSH_AD_CTRL_REG5               0x24
#define  LIS3DSH_AD_CTRL_REG6               0x25
#define  LIS3DSH_AD_STATUS                  0x27
#define  LIS3DSH_AD_OUT_X_L                 0x28
#define  LIS3DSH_AD_OUT_X_H                 0x29
#define  LIS3DSH_AD_OUT_Y_L                 0x2A
#define  LIS3DSH_AD_OUT_Y_H                 0x2B
#define  LIS3DSH_AD_OUT_Z_L                 0x2C
#define  LIS3DSH_AD_OUT_Z_H                 0x2D
#define  LIS3DSH_AD_FIFO_CTRL               0x2E
#define  LIS3DSH_AD_FIFO_SRC                0x2F
#define  LIS3DSH_AD_ST1_0                   0x40
#define  LIS3DSH_AD_ST1_1                   0x41
#define  LIS3DSH_AD_ST1_2                   0x42
#define  LIS3DSH_AD_ST1_3                   0x43
#define  LIS3DSH_AD_ST1_4                   0x44
#define  LIS3DSH_AD_ST1_5                   0x45
#define  LIS3DSH_AD_ST1_6                   0x46
#define  LIS3DSH_AD_ST1_7                   0x47
#define  LIS3DSH_AD_ST1_8                   0x48
#define  LIS3DSH_AD_ST1_9                   0x49
#define  LIS3DSH_AD_ST1_A                   0x4A
#define  LIS3DSH_AD_ST1_B                   0x4B
#define  LIS3DSH_AD_ST1_C                   0x4C
#define  LIS3DSH_AD_ST1_D                   0x4D
#define  LIS3DSH_AD_ST1_E                   0x4E
#define  LIS3DSH_AD_ST1_F                   0x4F
#define  LIS3DSH_AD_TIM4_1                  0x50
#define  LIS3DSH_AD_TIM3_1                  0x51
#define  LIS3DSH_AD_TIM2_1_L                0x52
#define  LIS3DSH_AD_TIM2_1_H                0x53
#define  LIS3DSH_AD_TIM1_1_L                0x54
#define  LIS3DSH_AD_TIM1_1_H                0x55
#define  LIS3DSH_AD_THRS2_1                 0x56
#define  LIS3DSH_AD_THRS1_1                 0x57
#define  LIS3DSH_AD_MASK1_B                 0x59
#define  LIS3DSH_AD_MASK1_A                 0x5A
#define  LIS3DSH_AD_SETT1                   0x5B
#define  LIS3DSH_AD_PR1                     0x5C
#define  LIS3DSH_AD_TC1_L                   0x5D
#define  LIS3DSH_AD_TC1_H                   0x5E
#define  LIS3DSH_AD_OUTS1                   0x5F
#define  LIS3DSH_AD_ST2_0                   0x60
#define  LIS3DSH_AD_ST2_1                   0x61
#define  LIS3DSH_AD_ST2_2                   0x62
#define  LIS3DSH_AD_ST2_3                   0x63
#define  LIS3DSH_AD_ST2_4                   0x64
#define  LIS3DSH_AD_ST2_5                   0x65
#define  LIS3DSH_AD_ST2_6                   0x66
#define  LIS3DSH_AD_ST2_7                   0x67
#define  LIS3DSH_AD_ST2_8                   0x68
#define  LIS3DSH_AD_ST2_9                   0x69
#define  LIS3DSH_AD_ST2_A                   0x6A
#define  LIS3DSH_AD_ST2_B                   0x6B
#define  LIS3DSH_AD_ST2_C                   0x6C
#define  LIS3DSH_AD_ST2_D                   0x6D
#define  LIS3DSH_AD_ST2_E                   0x6E
#define  LIS3DSH_AD_ST2_F                   0x6F
#define  LIS3DSH_AD_TIM4_2                  0x70
#define  LIS3DSH_AD_TIM3_2                  0x71
#define  LIS3DSH_AD_TIM2_2_L                0x72
#define  LIS3DSH_AD_TIM2_2_H                0x73
#define  LIS3DSH_AD_TIM1_2_L                0x74
#define  LIS3DSH_AD_TIM1_2_H                0x75
#define  LIS3DSH_AD_THRS2_2                 0x76
#define  LIS3DSH_AD_THRS1_2                 0x77
#define  LIS3DSH_AD_DES2                    0x78
#define  LIS3DSH_AD_MASK2_B                 0x79
#define  LIS3DSH_AD_MASK2_A                 0x7A
#define  LIS3DSH_AD_SETT2                   0x7B
#define  LIS3DSH_AD_PR2                     0x7C
#define  LIS3DSH_AD_TC2_L                   0x7D
#define  LIS3DSH_AD_TC2_H                   0x7E
#define  LIS3DSH_AD_OUTS2                   0x7F

#define LIS3DSH_I2C_MULTIPLEBYTE_CMD                      ((uint8_t)0x80)

#define LIS3DSH_ACC_ODR_PD  		0x00  /**< ODR 100 Hz.                        */
#define LIS3DSH_ACC_ODR_3_125HZ 	0x10  /**< ODR 3.125 Hz.                      */
#define LIS3DSH_ACC_ODR_6_25HZ 		0x20  /**< ODR 6.25 Hz.                       */
#define LIS3DSH_ACC_ODR_12_5HZ		0x30  /**< ODR 12.5 Hz.                       */
#define LIS3DSH_ACC_ODR_25HZ 		0x40  /**< ODR 25 Hz.                         */
#define LIS3DSH_ACC_ODR_50HZ 		0x50  /**< ODR 50 Hz.                         */
#define LIS3DSH_ACC_ODR_100HZ		0x60  /**< ODR 100 Hz.                        */
#define LIS3DSH_ACC_ODR_400HZ		0x70  /**< ODR 400 Hz.                        */
#define LIS3DSH_ACC_ODR_800HZ 		0x80  /**< ODR 800 Hz.                        */
#define LIS3DSH_ACC_ODR_1600HZ 		0x90  /**< ODR 1600 Hz.                       */

#define LIS3DSH_ACC_ODR_MASK                             ((uint8_t)0xE0)

#define LIS3DSH_ACC_FS_2G 			0x00         /**< Full scale ±2g.                  */
#define LIS3DSH_ACC_FS_4G 			0x08         /**< Full scale ±4g.                  */
#define LIS3DSH_ACC_FS_6G 			0x10         /**< Full scale ±6g.                  */
#define LIS3DSH_ACC_FS_8G 			0x18         /**< Full scale ±8g.                  */
#define LIS3DSH_ACC_FS_16G 			0x20         /**< Full scale ±16g.           		*/

#define LIS3DSH_ACC_FS_MASK                              ((uint8_t)0x18)

#define LIS3DSH_ACC_BW_800HZ 		0x00      /**< AA filter BW 800Hz.                */
#define LIS3DSH_ACC_BW_200HZ 		0x40      /**< AA filter BW 200Hz.                */
#define LIS3DSH_ACC_BW_400HZ 		0x80      /**< AA filter BW 400Hz.                */
#define LIS3DSH_ACC_BW_50HZ 		0xC0      /**< AA filter BW 50Hz.                 */

#define LIS3DSH_ACC_BDU_CONTINUOUS 	0x00	/**< Block data continuously updated.   */
#define LIS3DSH_ACC_BDU_BLOCKED 	0x80    /**< Block data updated after reading.  */

#define LIS3DSH_ACC_ZEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s Z-axis output enable: disable */
#define LIS3DSH_ACC_ZEN_ENABLE                           ((uint8_t)0x08) /*!< Accelerometer’s Z-axis output enable: enable */

#define LIS3DSH_ACC_ZEN_MASK                             ((uint8_t)0x08)

#define LIS3DSH_ACC_YEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s Y-axis output enable: disable */
#define LIS3DSH_ACC_YEN_ENABLE                           ((uint8_t)0x02) /*!< Accelerometer’s Y-axis output enable: enable */

#define LIS3DSH_ACC_YEN_MASK                             ((uint8_t)0x02)

#define LIS3DSH_ACC_XEN_DISABLE                          ((uint8_t)0x00) /*!< Accelerometer’s X-axis output enable: disable */
#define LIS3DSH_ACC_XEN_ENABLE                           ((uint8_t)0x01) /*!< Accelerometer’s X-axis output enable: enable */

#define LIS3DSH_ACC_XEN_MASK                             ((uint8_t)0x01)


/**
 * @name    LIS3DSH_CTRL_REG1 register bits definitions
 * @{
 */
#define LIS3DSH_CTRL_REG1_MASK              0xE9
#define LIS3DSH_CTRL_REG1_SM1_EN            (1 << 0)
#define LIS3DSH_CTRL_REG1_SM1_PIN           (1 << 3)
#define LIS3DSH_CTRL_REG1_HYST0_1           (1 << 5)
#define LIS3DSH_CTRL_REG1_HYST1_1           (1 << 6)
#define LIS3DSH_CTRL_REG1_HYST2_1           (1 << 7)
/** @} */

/**
 * @name    LIS3DSH_CTRL_REG2 register bits definitions
 * @{
 */
#define LIS3DSH_CTRL_REG2_MASK              0xE9
#define LIS3DSH_CTRL_REG2_SM2_EN            (1 << 0)
#define LIS3DSH_CTRL_REG2_SM2_PIN           (1 << 3)
#define LIS3DSH_CTRL_REG2_HYST0_2           (1 << 5)
#define LIS3DSH_CTRL_REG2_HYST1_2           (1 << 6)
#define LIS3DSH_CTRL_REG2_HYST2_2           (1 << 7)
/** @} */

/**
 * @name    LIS3DSH_CTRL_REG3 register bits definitions
 * @{
 */
#define LIS3DSH_CTRL_REG3_MASK              0xFF
#define LIS3DSH_CTRL_REG3_STRT              (1 << 0)
#define LIS3DSH_CTRL_REG3_VFILT             (1 << 2)
#define LIS3DSH_CTRL_REG3_INT1_EN           (1 << 3)
#define LIS3DSH_CTRL_REG3_INT2_EN           (1 << 4)
#define LIS3DSH_CTRL_REG3_IEL               (1 << 5)
#define LIS3DSH_CTRL_REG3_IEA               (1 << 6)
#define LIS3DSH_CTRL_REG3_DR_EN             (1 << 7)
/** @} */

/**
 * @name    LIS3DSH_CTRL_REG4 register bits definitions
 * @{
 */
#define LIS3DSH_CTRL_REG4_MASK              0xFF
#define LIS3DSH_CTRL_REG4_XEN               (1 << 0)
#define LIS3DSH_CTRL_REG4_YEN               (1 << 1)
#define LIS3DSH_CTRL_REG4_ZEN               (1 << 2)
#define LIS3DSH_CTRL_REG4_BDU               (1 << 3)
#define LIS3DSH_CTRL_REG4_ODR_0             (1 << 4)
#define LIS3DSH_CTRL_REG4_ODR_1             (1 << 5)
#define LIS3DSH_CTRL_REG4_ODR_2             (1 << 6)
#define LIS3DSH_CTRL_REG4_ODR_3             (1 << 7)
/** @} */

/**
 * @name    LIS3DSH_CTRL_REG5 register bits definitions
 * @{
 */
#define LIS3DSH_CTRL_REG5_MASK              0xFF
#define LIS3DSH_CTRL_REG5_SIM               (1 << 0)
#define LIS3DSH_CTRL_REG5_ST1               (1 << 1)
#define LIS3DSH_CTRL_REG5_ST2               (1 << 2)
#define LIS3DSH_CTRL_REG5_FS_MASK           0x38
#define LIS3DSH_CTRL_REG5_FS0               (1 << 3)
#define LIS3DSH_CTRL_REG5_FS1               (1 << 4)
#define LIS3DSH_CTRL_REG5_FS2               (1 << 5)
#define LIS3DSH_CTRL_REG5_BW1               (1 << 6)
#define LIS3DSH_CTRL_REG5_BW2               (1 << 7)
/** @} */

/**
 * @name    LIS3DSH_CTRL_REG6 register bits definitions
 * @{
 */
#define LIS3DSH_CTRL_REG6_MASK              0xFF
#define LIS3DSH_CTRL_REG6_P2_BOOT           (1 << 0)
#define LIS3DSH_CTRL_REG6_P1_OVRUN          (1 << 1)
#define LIS3DSH_CTRL_REG6_P1_WTM            (1 << 2)
#define LIS3DSH_CTRL_REG6_P1_EMPTY          (1 << 3)
#define LIS3DSH_CTRL_REG6_ADD_INC           (1 << 4)
#define LIS3DSH_CTRL_REG6_WTM_EN            (1 << 5)
#define LIS3DSH_CTRL_REG6_FIFO_EN           (1 << 6)
#define LIS3DSH_CTRL_REG6_BOOT              (1 << 7)
/** @} */

typedef struct
{
  float ACC_OutputDataRate;
  float ACC_FullScale;
  uint8_t ACC_X_Axis;
  uint8_t ACC_Y_Axis;
  uint8_t ACC_Z_Axis;
} IMU_6AXES_InitTypeDef;

/**
 * @brief  IMU_6AXES status enumerator definition
 */
typedef enum
{
  IMU_6AXES_OK = 0,
  IMU_6AXES_ERROR = 1,
  IMU_6AXES_TIMEOUT = 2,
  IMU_6AXES_NOT_IMPLEMENTED = 3
} IMU_6AXES_StatusTypeDef;

/**
 * @brief  IMU_6AXES component id enumerator definition
 */
typedef enum
{
  IMU_6AXES_NONE_COMPONENT = 0,
  IMU_6AXES_LSM6DS0_COMPONENT = 1,
  IMU_6AXES_LSM6DS3_DIL24_COMPONENT = 2
} IMU_6AXES_ComponentTypeDef;

IMU_6AXES_StatusTypeDef LIS3DSH_init(IMU_6AXES_InitTypeDef *LIS3DSH_InitTypeDef);

IMU_6AXES_StatusTypeDef LSM6DS0_ACC_GetAxes(int32_t *pData);

#endif

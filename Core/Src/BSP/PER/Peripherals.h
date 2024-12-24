/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : peripherals.h
  * @brief          : Header for peripherals.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  *
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_PER_H
#define __BSP_PER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
	eBSP_PER_TARGET_VOID		,		//	0
	eBSP_PER_TARGET_SHT40A		,		//	1
	eBSP_PER_TARGET_STTS22		,		//	2
	eBSP_PER_TARGET_LPS22D		,		//	3
	eBSP_PER_TARGET_LIS2MDL		,		//	4
	eBSP_PER_TARGET_LSM6DSV		,		//	5
	eBSP_PER_TARGET_LSM6DSO		,		//	6
	eBSP_PER_TARGET_LIS2DUX		,		//	7
	/***** DON'T CROSS THIS LINE *****/
	eBSP_PER_MAX_VALUE_TARGET			//
}tBSP_PER_Target;


typedef	enum
{
	eBSP_PER_FUNC_VOID			,		//	0
	eBSP_PER_FUNC_TEMP			,		//	1
	eBSP_PER_FUNC_RH			,		//	2
	eBSP_PER_FUNC_TEMP_RH		,		//	3
	eBSP_PER_FUNC_HEATER		,		//	4
	eBSP_PER_FUNC_GET_SN		,		//	5
	/***** DON'T CROSS THIS LINE *****/
	eBSP_PER_MAX_VALUE_FUNC				//
}tBSP_PER_Func;


typedef	enum
{
	eBSP_PER_PRCSN_VOID			,		//	0
	eBSP_PER_PRCSN_LOW			,		//	1
	eBSP_PER_PRCSN_MED			,		//	2
	eBSP_PER_PRCSN_HIGH			,		//	3
	/***** DON'T CROSS THIS LINE *****/
	eBSP_PER_MAX_VALUE_PRCSN			//
}tBSP_PER_Precision;

typedef struct
{
	tBSP_PER_Target		Target;
	tBSP_PER_Func		Function;
	tBSP_PER_Precision	Precision;
	uint16_t			Time_msec;
	uint16_t			Power_mW;
}tBSP_PER_DataCmd;

typedef struct
{
	float				Temperature;
	float				Humidity_f;
	uint8_t				Humidity_i;
	uint8_t				Address;
	uint32_t			SerialNumber;
}tBSP_PER_DataResp;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
float				BSP_Per_Convert( tBSP_PER_Target Target, tBSP_PER_Func Function, uint32_t Value);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_PER_H */

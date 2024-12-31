/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : I2C.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task_Sensors.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include "SHT40.h"
#include "STTS22.h"
#include "LPS22D.h"
#include "LIS2MDL.h"
#include "LSM6DSV.h"
#include "LSM6DSO.h"
#include "LIS2DUX.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct
{
	bool					isInit;
	I2C_HandleTypeDef*		hI2C;
} Main_Info	= {	.isInit	= false,
				.hI2C	= NULL};

struct
{
	void*					handle;
}Main_Targets[eBSP_PER_MAX_VALUE_TARGET];

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_Sensors_Init( I2C_HandleTypeDef *handle)
{
	Main_Info.isInit	= true;
	Main_Info.hI2C		= handle;

	memset(Main_Targets, 0, sizeof(Main_Targets));

	Main_Targets[eBSP_PER_TARGET_SHT40A].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_STTS22].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LPS22D].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LIS2MDL].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LSM6DSV].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LSM6DSO].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LIS2DUX].handle	= Main_Info.hI2C;
}

/**
  * @brief
  * @retval
  */
void 				task_Sensors( void *arguments)
{
	while(1)
	{
		osDelay(1);
		if( Main_Info.isInit == false)	continue;
	}
}

/* USER CODE END 4 */

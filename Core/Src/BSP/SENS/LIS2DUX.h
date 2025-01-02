/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lis2dux..h
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	enum
{
	CMD_LIS2DUX_GET_SN						=	0x0F,
}tCmd_LIS2DUX;

typedef	void(*tCb_GetData_LIS2DUX)(tBSP_PER_DataResp *Data);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_DEVICE_ADDRESS_LIS2DUX		(0x19 << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void				BSP_LIS2DUX_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc);
void 				task_LIS2DUX( void *arguments);
bool				BSP_LIS2DUX_Cmd( tBSP_PER_DataCmd* cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lsm6dso..h
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
	CMD_LSM6DSO_GET_SN						=	0x0F,
}tCmd_LSM6DSO;

typedef	void(*tCb_GetData_LSM6DSO)(tBSP_PER_DataResp *Data);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_DEVICE_ADDRESS_LSM6DSO		(0x6A << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void				BSP_LSM6DSO_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc);
void 				task_LSM6DSO( void *arguments);
bool				BSP_LSM6DSO_Cmd( tBSP_PER_DataCmd* cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


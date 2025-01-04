/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : task_Sensors.h
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
#include "..\PER\Peripherals.h"

/* Private typedef -----------------------------------------------------------*/
typedef	enum
{
	eBSP_SENS_ERR_VOID			,		//	0
	eBSP_SENS_ERR_Q_LVL			,		//	1
	/***** DON'T CROSS THIS LINE *****/
	eBSP_SENS_MAX_VALUE_ERR				//
}tBSP_SENS_ErrCode;

/* USER CODE BEGIN PTD */
typedef struct
{
	tBSP_PER_Target		target;
	tBSP_PER_Func		func;
	int32_t				arg1;
	int32_t				arg2;
	int32_t				arg3;
} tQ_Sensor_Cmd;
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
void				BSP_Sensors_Init( I2C_HandleTypeDef *handle);
void 				task_Sensors( void *arguments);
void				BSP_Sensors_Cmd( tBSP_PER_DataCmd *Cmd, bool FromISR);
void				BSP_Sensors_SetErr( tBSP_PER_Target Source, tBSP_SENS_ErrCode ErrCode, bool Set);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */


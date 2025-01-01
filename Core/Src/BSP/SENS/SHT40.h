/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : sht40.h
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
	CMD_SHT40_GET_TEMP_RH_PRECISION_HI		=	0xFD,
	CMD_SHT40_GET_TEMP_RH_PRECISION_MED		=	0xF6,
	CMD_SHT40_GET_TEMP_RH_PRECISION_LO		=	0xE0,
	CMD_SHT40_GET_SN						=	0x89,
	CMD_SHT40_RESET							=	0x94,
	CMD_SHT40_HEATER_200MW_1000MSEC			=	0x39,
	CMD_SHT40_HEATER_200MW_100MSEC			=	0x32,
	CMD_SHT40_HEATER_110MW_1000MSEC			=	0x2F,
	CMD_SHT40_HEATER_110MW_100MSEC			=	0x24,
	CMD_SHT40_HEATER_20MW_1000MSEC			=	0x1E,
	CMD_SHT40_HEATER_20MW_100MSEC			=	0x15,
}tCmd_SHT40;

typedef struct
{
	tCmd_SHT40			cmd;
} tQ_SHT40_Cmd;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_DEVICE_ADDRESS_SHT40		(0x44 << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void				BSP_SHT40_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc);
void 				task_SHT40( void *arguments);
bool				BSP_SHT40_Cmd( tBSP_PER_DataCmd	*cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : stts22.h
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
	CMD_STTS22_GET_SN						=	0x01,
	CMD_STTS22_TEMP_H_LIMIT					=	0x02,
	CMD_STTS22_TEMP_L_LIMIT					=	0x03,
	CMD_STTS22_CTRL							=	0x04,
	CMD_STTS22_STATUS						=	0x05,
	CMD_STTS22_TEMP_L						=	0x06,
	CMD_STTS22_TEMP_H						=	0x07,
}tCmd_STTS22;

typedef	void(*tCb_GetData_STTS22)(tBSP_PER_DataResp *Data);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_DEVICE_ADDRESS_STTS22		(0x38 << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
bool			BSP_STTS22_Init( I2C_HandleTypeDef *handle, tCb_GetData_STTS22	CbFunc);
void 			BSP_STTS22_MainLoop( void);
bool			BSP_STTS22_Cmd( tBSP_PER_DataCmd	*cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


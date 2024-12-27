/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lps22d.h
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
	CMD_LPS22D_GET_SN						=	0x0F,
//	CMD_LPS22D_TEMP_H_LIMIT					=	0x02,
//	CMD_LPS22D_TEMP_L_LIMIT					=	0x03,
//	CMD_LPS22D_CTRL							=	0x04,
//	CMD_LPS22D_STATUS						=	0x05,
//	CMD_LPS22D_TEMP_L						=	0x06,
//	CMD_LPS22D_TEMP_H						=	0x07,
}tCmd_LPS22D;

typedef	void(*tCb_GetData_LPS22D)(tBSP_PER_DataResp *Data);

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define I2C_DEVICE_ADDRESS_LPS22D		(0x5D << 1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
bool			BSP_LPS22D_Init( I2C_HandleTypeDef *handle, tCb_GetData_LPS22D	CbFunc);
void 			BSP_LPS22D_MainLoop( void);
bool			BSP_LPS22D_Cmd( tBSP_PER_DataCmd	*cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


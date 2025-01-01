/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : I2C.h
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
typedef	void	(*BSP_I2C_Cb_Done)(bool result);	//	false in case of failure

typedef	struct
{
	TaskHandle_t			taskHandle;
	I2C_HandleTypeDef*		i2cHandle;
	uint16_t 				Address;
	tBSP_PER_Target			Device;
	uint8_t*				TxBuf;
	uint16_t 				TxLen;
	uint8_t*				RxBuf;
	uint16_t 				RxLen;
	uint16_t				DelayAfterTx;
	uint16_t				DelayAfterRx;
	uint16_t				Timeout;
}tBSP_I2C_Session;
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
void				BSP_I2C_Init( I2C_HandleTypeDef *handle);
void 				task_I2C( void *arguments);
bool				BSP_I2C_Cmd( tBSP_I2C_Session Cmd);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


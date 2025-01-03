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
#include "cmsis_os.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include "I2C.h"
#include ".\SENS\SHT40.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes	= {	.name = "Q_I2C"};
static	uint16_t					Main_RxLen		= 0;
static	TaskHandle_t				Main_taskHandle	= NULL;

// Bus handling
//static	tBSP_PER_Target		Main_ActiveDevice	= eBSP_PER_TARGET_VOID;
//static	uint16_t			Main_TO_Value		= 0;
//static	uint32_t			Main_TO_Target		= 0;
//static	tBSP_I2C_Session	Main_BSP_I2C_Session	= {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	void		BSP_I2C_Session( tBSP_I2C_Session Cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_I2C_Init( I2C_HandleTypeDef *handle)
{
	Main_Q	= osMessageQueueNew(16, sizeof(tBSP_I2C_Session), &Q_attributes);

}

/**
  * @brief
  * @retval
  */
void 				task_I2C( void *arguments)
{
	tBSP_I2C_Session	Cmd;

	while (1)
	{
		osDelay(1); // Consider whether this is necessary.

		osMessageQueueGet(Main_Q, &Cmd, NULL, osWaitForever);

		BSP_I2C_Session(Cmd);
	}
}

/**
  * @brief
  * @retval
  */
bool				BSP_I2C_Cmd( tBSP_I2C_Session Cmd)
{
	osMessageQueuePut(Main_Q, &Cmd, 0, 0);

    return true;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_I2C_Session( tBSP_I2C_Session Cmd)
{
	HAL_StatusTypeDef	HAL_result;

	Main_RxLen		= Cmd.RxLen;
	Main_taskHandle	= xTaskGetCurrentTaskHandle();

	HAL_result = HAL_I2C_Master_Transmit_IT(Cmd.i2cHandle, Cmd.Address, Cmd.TxBuf, Cmd.TxLen);
	ulTaskNotifyTake(pdTRUE, 50);
	vTaskDelay(Cmd.DelayAfterTx);

	if( Main_RxLen == 0)
	{
		xTaskNotifyGive(Cmd.taskHandle);
	}
	else
	{
		HAL_result = HAL_I2C_Master_Receive_IT(Cmd.i2cHandle, Cmd.Address, Cmd.RxBuf, Cmd.RxLen);
		ulTaskNotifyTake(pdTRUE, 50);
		vTaskDelay(Cmd.DelayAfterRx);
		xTaskNotifyGive(Cmd.taskHandle);
	}
}

void			HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Main_taskHandle, &xHigherPriorityTaskWoken);
}

void			HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Main_taskHandle, &xHigherPriorityTaskWoken);
}

void			HAL_I2C_ErrorCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Main_taskHandle, &xHigherPriorityTaskWoken);
}

/* USER CODE END 4 */

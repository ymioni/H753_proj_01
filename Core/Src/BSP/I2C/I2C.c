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

static	osMutexId_t 				Main_Mtx;
static	const osMutexAttr_t			Mtx_attributes	= {	.name = "M_I2C"};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	bool		BSP_I2C_Session( tBSP_I2C_Session Cmd);

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
	Main_Mtx	= osMutexNew( &Mtx_attributes);
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
		cnt1ok[6]	++;

		BSP_I2C_Session(Cmd);
	}
}

/**
  * @brief
  * @retval
  */
bool				BSP_I2C_Cmd( tBSP_I2C_Session Cmd)
{
	osStatus_t	status;
	static	bool	f = false;

	if( f == true) cnt1er[6] ++;
	f = true;

	status	= osMutexAcquire( Main_Mtx, pdMS_TO_TICKS( 50));
	if( status == osOK)	cnt1ok[4]	++;
	else				cnt1er[4]	++;

	if( status == osOK)
	{
		status = osMessageQueuePut(Main_Q, &Cmd, 0, pdMS_TO_TICKS( 20));
		osMutexRelease( Main_Mtx);
		if( status == osOK)	cnt1ok[5]	++;
		else				cnt1er[5]	++;
	}

	f = false;
    return (status == osOK);
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_I2C_Session( tBSP_I2C_Session Cmd)
{
	HAL_StatusTypeDef	HAL_result;

	Main_RxLen		= Cmd.RxLen;
	Main_taskHandle	= xTaskGetCurrentTaskHandle();

	HAL_result = HAL_I2C_Master_Transmit_IT(Cmd.i2cHandle, Cmd.Address, Cmd.TxBuf, Cmd.TxLen);
	if( HAL_result == HAL_OK)	cnt1ok[0]	++;
	else						cnt1er[0]	++;
	ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)); // will be released by HAL_I2C_MasterTxCpltCallback()::vTaskNotifyGiveFromISR

	vTaskDelay(Cmd.DelayAfterTx);

	if( Main_RxLen > 0)
	{
		HAL_result = HAL_I2C_Master_Receive_IT(Cmd.i2cHandle, Cmd.Address, Cmd.RxBuf, Cmd.RxLen);
		if( HAL_result == HAL_OK)	cnt1ok[1]	++;
		else						cnt1er[1]	++;
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)); // will be released by HAL_I2C_MasterRxCpltCallback()::vTaskNotifyGiveFromISR
		vTaskDelay(Cmd.DelayAfterRx);
	}

	xTaskNotifyGive(Cmd.taskHandle);

	return( HAL_result == HAL_OK);
}

void				HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	cnt1ok[2]	++;
	vTaskNotifyGiveFromISR(Main_taskHandle, &xHigherPriorityTaskWoken);
}

void				HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	cnt1ok[3]	++;
	vTaskNotifyGiveFromISR(Main_taskHandle, &xHigherPriorityTaskWoken);
}

void				HAL_I2C_ErrorCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	cnt1er[2]	++;
	vTaskNotifyGiveFromISR(Main_taskHandle, &xHigherPriorityTaskWoken);
}

/* USER CODE END 4 */

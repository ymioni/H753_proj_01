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
#include "task_Sensors.h"
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct
{
	I2C_HandleTypeDef*		hI2C;
} Main_Info	= {	.hI2C	= NULL};

struct
{
	void*					handle;
}Main_Targets[eBSP_PER_MAX_VALUE_TARGET];

static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes = {	.name = "Q_Sens_App"};
static	tQ_Sensor_Cmd				Main_Q_Cmd;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	void		BSP_Sensors_InitSensors( void);
static	void		BSP_Sensors_TxCmd2Sensor( tQ_Sensor_Cmd	*cmd);
static	void		BSP_Sensors_Cb_GetData( tBSP_PER_DataResp* data);

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
	Main_Info.hI2C		= handle;

	memset(Main_Targets, 0, sizeof(Main_Targets));

	Main_Targets[eBSP_PER_TARGET_SHT40A].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_STTS22].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LPS22D].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LIS2MDL].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LSM6DSV].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LSM6DSO].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LIS2DUX].handle	= Main_Info.hI2C;

	Main_Q	= osMessageQueueNew(32, sizeof(tQ_Sensor_Cmd), &Q_attributes);

	BSP_Sensors_InitSensors();
}

/**
  * @brief
  * @retval
  */
void 				task_Sensors( void *arguments)
{
	tQ_Sensor_Cmd	Cmd = {0};

	while(1)
	{
		osDelay(1);

		osMessageQueueGet (Main_Q, &Cmd, NULL, osWaitForever);

		BSP_Sensors_TxCmd2Sensor(&Cmd);
	}
}

static	void		BSP_Sensors_InitSensors( void)
{
	// SHT40
	BSP_SHT40_Init(Main_Targets[eBSP_PER_TARGET_SHT40A].handle, BSP_Sensors_Cb_GetData);

	Main_Q_Cmd.target	= eBSP_PER_TARGET_SHT40A;
	Main_Q_Cmd.func		= eBSP_PER_FUNC_TEMP_RH;
	Main_Q_Cmd.arg1		= eBSP_PER_PRCSN_HIGH;
	osMessageQueuePut(Main_Q, &Main_Q_Cmd, 0, 0);
}

static	void		BSP_Sensors_TxCmd2Sensor( tQ_Sensor_Cmd	*cmd)
{
	tBSP_PER_DataCmd	Cmd = {0};

	Cmd.Target		=	cmd->target;
	Cmd.Function	=	cmd->func;

	switch(cmd->target)
	{
	// SHT40
	case	eBSP_PER_TARGET_SHT40A:
		switch(cmd->func)
		{
		case	eBSP_PER_FUNC_TEMP:
		case	eBSP_PER_FUNC_RH:
		case	eBSP_PER_FUNC_TEMP_RH:
			Cmd.Precision	= cmd->arg1;
			break;

		default:
			break;
		}

		BSP_SHT40_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_STTS22:
		break;

	case	eBSP_PER_TARGET_LPS22D:
		break;

	case	eBSP_PER_TARGET_LIS2MDL:
		break;

	case	eBSP_PER_TARGET_LSM6DSV:
		break;

	case	eBSP_PER_TARGET_LSM6DSO:
		break;

	case	eBSP_PER_TARGET_LIS2DUX:
		break;

	default:
		break;
	}
}

static	void		BSP_Sensors_Cb_GetData( tBSP_PER_DataResp* data)
{
	printf("BSP_Sensors_Cb_GetData\n");
}

/* USER CODE END 4 */

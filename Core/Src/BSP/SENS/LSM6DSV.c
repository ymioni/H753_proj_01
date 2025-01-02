/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lsm6dsv..c
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
#include "..\I2C\I2C.h"
#include "LSM6DSV.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_LSM6DSV	cmd;
	bool			set;
}tQ_Cmd;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct __PACKED
{
	uint8_t		SN;
}Data_LSM6DSV_SN;

struct __PACKED
{
	uint8_t		Status;
}Data_LSM6DSV_Status;

static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes		= {	.name = "Q_LSM6DSV"};
static	I2C_HandleTypeDef*			Main_Handle 		= NULL;
static	tCb_Sensor_GetData			Main_CbFunc			= NULL;

static	uint16_t					Main_Timeout		= 1000;
static	uint16_t					Main_Delay 			= 50;

static	uint8_t 					Main_TxBuf[2]		= {0};
static	uint8_t 					Main_TxLen			= 0;
static	uint8_t *					Main_RxBuf			= NULL;
static	uint8_t 					Main_RxLen			= 0;
static	tBSP_I2C_Session			Main_Session		= {0};
static	tBSP_PER_DataResp			Main_Per_DataResp	= {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	bool		BSP_LSM6DSV_Transaction(tQ_Cmd Rec);
static	void		BSP_LSM6DSV_Transaction_Tx(bool Rx, tCmd_LSM6DSV Cmd);
static	void		BSP_LSM6DSV_Transaction_Rx(void);
static	void		BSP_LSM6DSV_Session(void);
static	bool		BSP_LSM6DSV_Transaction_SetData(tCmd_LSM6DSV Cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_LSM6DSV_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc)
{
	Main_Handle = handle;
	Main_CbFunc	= CbFunc;

	Main_Q	= osMessageQueueNew(16, sizeof(tQ_Cmd), &Q_attributes);

	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_LSM6DSV,
										.Function	=	eBSP_PER_FUNC_GET_SN};
		BSP_Sensors_Cmd( &Cmd, false);
	}
}

/**
  * @brief
  * @retval
  */
void 				task_LSM6DSV( void *arguments)
{
	tQ_Cmd Cmd;

	while (1)
	{
		osDelay(1); // Consider whether this is necessary.

		osMessageQueueGet(Main_Q, &Cmd, NULL, osWaitForever);

		BSP_LSM6DSV_Transaction(Cmd);
	}
}

bool				BSP_LSM6DSV_Cmd( tBSP_PER_DataCmd	*cmd)
{
	tQ_Cmd	Cmd = {0};
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))							return false;

	Cmd.cmd	= 0;
	Cmd.set	= false;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_GET_SN:
		Cmd.cmd	= CMD_LSM6DSV_GET_SN;
		break;

	default:
		break;
	}

	if( Cmd.cmd == 0)
		result = false;
	else
		osMessageQueuePut(Main_Q, &Cmd, 0, 0);

	return	result;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LSM6DSV_Transaction(tQ_Cmd Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.cmd)
	{
	case	CMD_LSM6DSV_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen = idx;
		Main_RxBuf	= (uint8_t *)&Data_LSM6DSV_SN;
		Main_RxLen	= sizeof(Data_LSM6DSV_SN);
		break;

	default:
		result = false;
		break;
	}

	if( result == true)
	{
		if( Rec.set == BSP_GET)
		{
			BSP_LSM6DSV_Transaction_Tx(true, Rec.cmd);
		}
		else
		{
			BSP_LSM6DSV_Transaction_Tx(false, Rec.cmd);
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LSM6DSV_Transaction_Tx(bool Rx, tCmd_LSM6DSV Cmd)
{
	Main_Session.i2cHandle		= Main_Handle;
	Main_Session.taskHandle		= xTaskGetCurrentTaskHandle();
	Main_Session.Address		= I2C_DEVICE_ADDRESS_LSM6DSV;
	Main_Session.Device			= eBSP_PER_TARGET_LSM6DSV;
	Main_Session.TxBuf			= Main_TxBuf;
	Main_Session.TxLen			= Main_TxLen;
	Main_Session.RxBuf			= NULL;
	Main_Session.RxLen			= 0;
	Main_Session.Timeout		= Main_Timeout;
	Main_Session.DelayAfterTx	= Main_Delay;
	Main_Session.DelayAfterRx	= 0;

	if( Rx)
		BSP_LSM6DSV_Transaction_Rx();

	BSP_LSM6DSV_Session();

	if( Rx)
		BSP_LSM6DSV_Transaction_SetData(Cmd);
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LSM6DSV_Transaction_Rx(void)
{
	Main_Session.RxBuf			= Main_RxBuf;
	Main_Session.RxLen			= Main_RxLen;
	Main_Session.DelayAfterRx	= Main_Delay;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LSM6DSV_Session(void)
{
	BSP_I2C_Cmd(Main_Session);

	ulTaskNotifyTake(pdTRUE, 1000);
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LSM6DSV_Transaction_SetData(tCmd_LSM6DSV Cmd)
{
	bool result = true;

	switch( Cmd)
	{
	case	CMD_LSM6DSV_GET_SN:
		Main_Per_DataResp.SerialNumber = Main_RxBuf[0];
		break;

	default:
		result = false;
		break;
	}

	printf("LSM6DSV | R: %d | SN: %lX T: %.2f RH: %d\n",
				result,
				Main_Per_DataResp.SerialNumber,
				Main_Per_DataResp.Temperature,
				Main_Per_DataResp.Humidity_i);

	return result;
}

/* USER CODE END 4 */

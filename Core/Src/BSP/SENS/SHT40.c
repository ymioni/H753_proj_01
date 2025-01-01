/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : sht40.c
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
#include "..\I2C\I2C.h"
#include "SHT40.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_SHT40		cmd;
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
	uint16_t	Temperature;
	uint8_t		CRC_Temperature;
	uint16_t	Humidity;
	uint8_t		CRC_Humidity;
}Data_SHT40_Temp;

struct __PACKED
{
	uint16_t	SN1;
	uint8_t		CRC_Temperature;
	uint16_t	SN2;
	uint8_t		CRC_Humidity;
}Data_SHT40_SN;


static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes		= {	.name = "Q_SHT40"};
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
static	bool		BSP_SHT40_Transaction(tQ_Cmd Rec);
static	void		BSP_SHT40_Transaction_Tx(bool Rx, tCmd_SHT40 Cmd);
static	void		BSP_SHT40_Transaction_Rx(void);
static	void		BSP_SHT40_Session(void);
static	void		BSP_SHT40_Cb_SessionEnd(bool result);
static	bool		BSP_SHT40_Transaction_SetData(tCmd_SHT40 Cmd);



/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_SHT40_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc)
{
	tQ_Cmd	Cmd = {0};

	Main_Handle = handle;
	Main_CbFunc	= CbFunc;

	Main_Q	= osMessageQueueNew(16, sizeof(tQ_Cmd), &Q_attributes);

	Cmd.cmd	= CMD_SHT40_GET_SN;
	Cmd.set	= false;
	osMessageQueuePut(Main_Q, &Cmd, 0, 0);
}

/**
  * @brief
  * @retval
  */
void 				task_SHT40( void *arguments)
{
	tQ_Cmd Cmd;

	while (1)
	{
		osDelay(1); // Consider whether this is necessary.

		osMessageQueueGet(Main_Q, &Cmd, NULL, osWaitForever);
		printf("SHT40: %.2X\n", Cmd.cmd);

		BSP_SHT40_Transaction(Cmd);
	}
}

bool				BSP_SHT40_Cmd( tBSP_PER_DataCmd	*cmd)
{
	tQ_Cmd	Cmd = {0};
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))							return false;

	Cmd.cmd	= 0;
	Cmd.set	= false;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_TEMP:
	case	eBSP_PER_FUNC_RH:
	case	eBSP_PER_FUNC_TEMP_RH:
		switch( cmd->Precision)
		{
		case	eBSP_PER_PRCSN_LOW:		Cmd.cmd	= CMD_SHT40_GET_TEMP_RH_PRECISION_LO;	break;
		case	eBSP_PER_PRCSN_MED:		Cmd.cmd	= CMD_SHT40_GET_TEMP_RH_PRECISION_MED;	break;
		case	eBSP_PER_PRCSN_HIGH:
		default:						Cmd.cmd	= CMD_SHT40_GET_TEMP_RH_PRECISION_HI;	break;
		}
		break;

	case	eBSP_PER_FUNC_GET_SN:
		Cmd.cmd	= CMD_SHT40_GET_SN;
		break;

	case	eBSP_PER_FUNC_RESET:
		Cmd.cmd	= CMD_SHT40_RESET;
		break;

	case	eBSP_PER_FUNC_HEATER:
		switch( cmd->Power_mW)
		{
		case	200:
			switch( cmd->Time_msec)
			{
			case	1000:	Cmd.cmd	= CMD_SHT40_HEATER_200MW_1000MSEC;	break;
			case	100:	Cmd.cmd	= CMD_SHT40_HEATER_200MW_100MSEC;	break;
			}
			break;

		case	110:
			switch( cmd->Time_msec)
			{
			case	1000:	Cmd.cmd	= CMD_SHT40_HEATER_110MW_1000MSEC;	break;
			case	100:	Cmd.cmd	= CMD_SHT40_HEATER_110MW_100MSEC;	break;
			}
			break;

		case	20:
			switch( cmd->Time_msec)
			{
			case	1000:	Cmd.cmd	= CMD_SHT40_HEATER_20MW_1000MSEC;	break;
			case	100:	Cmd.cmd	= CMD_SHT40_HEATER_20MW_100MSEC;	break;
			}
			break;
		}
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
static	bool		BSP_SHT40_Transaction(tQ_Cmd Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.cmd)
	{
	case	CMD_SHT40_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen = idx;
		Main_RxBuf	= (uint8_t *)&Data_SHT40_SN;
		Main_RxLen	= sizeof(Data_SHT40_SN);
		break;

	case	CMD_SHT40_RESET:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen = idx;
		Main_RxLen	= 0;
		break;

	case	CMD_SHT40_GET_TEMP_RH_PRECISION_HI:
	case	CMD_SHT40_GET_TEMP_RH_PRECISION_MED:
	case	CMD_SHT40_GET_TEMP_RH_PRECISION_LO:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen = idx;
		Main_RxBuf	= (uint8_t *)&Data_SHT40_Temp;
		Main_RxLen	= sizeof(Data_SHT40_Temp);
		break;

	case	CMD_SHT40_HEATER_200MW_1000MSEC:
	case	CMD_SHT40_HEATER_200MW_100MSEC:
	case	CMD_SHT40_HEATER_110MW_1000MSEC:
	case	CMD_SHT40_HEATER_110MW_100MSEC:
	case	CMD_SHT40_HEATER_20MW_1000MSEC:
	case	CMD_SHT40_HEATER_20MW_100MSEC:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen = idx;
		Main_RxLen	= 0;
		break;

	default:
		result = false;
		break;
	}

	if( result == true)
	{
		if( Rec.set == BSP_GET)
		{
			BSP_SHT40_Transaction_Tx(true, Rec.cmd);
		}
		else
		{
			BSP_SHT40_Transaction_Tx(false, Rec.cmd);
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_SHT40_Transaction_Tx(bool Rx, tCmd_SHT40 Cmd)
{
	Main_Session.i2cHandle		= Main_Handle;
	Main_Session.taskHandle		= xTaskGetCurrentTaskHandle();
	Main_Session.Address		= I2C_DEVICE_ADDRESS_SHT40;
	Main_Session.Device			= eBSP_PER_TARGET_SHT40A;
	Main_Session.TxBuf			= Main_TxBuf;
	Main_Session.TxLen			= Main_TxLen;
	Main_Session.RxBuf			= NULL;
	Main_Session.RxLen			= 0;
	Main_Session.Timeout		= Main_Timeout;
	Main_Session.DelayAfterTx	= Main_Delay;
	Main_Session.DelayAfterRx	= 0;
	Main_Session.Cb_SessionEnd	= BSP_SHT40_Cb_SessionEnd;

	if( Rx)
		BSP_SHT40_Transaction_Rx();

	BSP_SHT40_Session();

	if( Rx)
	{
		BSP_SHT40_Transaction_SetData(Cmd);

		printf("SN: %lX Temp: %.2f RH(f): %.2f RH(i): %d\n",
					Main_Per_DataResp.SerialNumber,
					Main_Per_DataResp.Temperature,
					Main_Per_DataResp.Humidity_f,
					Main_Per_DataResp.Humidity_i);
	}
}

/**
  * @brief
  * @retval
  */
static	void		BSP_SHT40_Transaction_Rx(void)
{
	Main_Session.RxBuf			= Main_RxBuf;
	Main_Session.RxLen			= Main_RxLen;
	Main_Session.DelayAfterRx	= Main_Delay;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_SHT40_Session(void)
{
	HAL_StatusTypeDef	HAL_result;

	HAL_result = HAL_I2C_Master_Transmit_IT(Main_Session.i2cHandle, Main_Session.Address, Main_Session.TxBuf, Main_Session.TxLen);
	ulTaskNotifyTake(pdTRUE, 1000);
	vTaskDelay(Main_Session.DelayAfterTx);
	HAL_result = HAL_I2C_Master_Receive_IT(Main_Session.i2cHandle, Main_Session.Address, Main_Session.RxBuf, Main_Session.RxLen);
	ulTaskNotifyTake(pdTRUE, 1000);
	vTaskDelay(Main_Session.DelayAfterRx);
}

void			HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Main_Session.taskHandle, &xHigherPriorityTaskWoken);
}

void			HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	vTaskNotifyGiveFromISR(Main_Session.taskHandle, &xHigherPriorityTaskWoken);
}

/**
  * @brief
  * @retval
  */
static	void		BSP_SHT40_Cb_SessionEnd(bool result)
{
	printf("BSP_SHT40_Cb_SessionEnd %d\n", result);
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_SHT40_Transaction_SetData(tCmd_SHT40 Cmd)
{
	bool result = true;

	switch( Cmd)
	{
	case	CMD_SHT40_GET_SN:
		BSP_Util_SwapBytes(&Data_SHT40_SN.SN1, sizeof( Data_SHT40_SN.SN1));
		BSP_Util_SwapBytes(&Data_SHT40_SN.SN2, sizeof( Data_SHT40_SN.SN2));
		Main_Per_DataResp.SerialNumber	=	(((uint32_t)Data_SHT40_SN.SN1) << 16) + Data_SHT40_SN.SN2;
		break;

	case	CMD_SHT40_GET_TEMP_RH_PRECISION_HI:
	case	CMD_SHT40_GET_TEMP_RH_PRECISION_MED:
	case	CMD_SHT40_GET_TEMP_RH_PRECISION_LO:
		BSP_Util_SwapBytes(&Data_SHT40_Temp.Temperature, sizeof( Data_SHT40_Temp.Temperature));
		BSP_Util_SwapBytes(&Data_SHT40_Temp.Humidity, sizeof( Data_SHT40_Temp.Humidity));
		Main_Per_DataResp.Temperature	=	BSP_Per_Convert(eBSP_PER_TARGET_SHT40A, eBSP_PER_FUNC_TEMP, Data_SHT40_Temp.Temperature);
		Main_Per_DataResp.Humidity_f	=	BSP_Per_Convert(eBSP_PER_TARGET_SHT40A, eBSP_PER_FUNC_RH, Data_SHT40_Temp.Humidity);
		Main_Per_DataResp.Humidity_i	=	Main_Per_DataResp.Humidity_f;
		break;

	default:
		result = false;
		break;
	}

	return result;
}

/* USER CODE END 4 */

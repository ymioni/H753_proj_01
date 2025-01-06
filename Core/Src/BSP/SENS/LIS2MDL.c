/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lis2mdl..c
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
#include "LIS2MDL.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_LIS2MDL	cmd;
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
}Data_LIS2MDL_SN;

struct __PACKED
{
	uint8_t		Control;
}Data_LIS2MDL_Control;

struct __PACKED
{
	uint8_t		Status;
}Data_LIS2MDL_Status;

struct __PACKED
{
	int16_t		OutX;
	int16_t		OutY;
	int16_t		OutZ;
}Data_LIS2MDL_Axis;

struct __PACKED
{
	uint16_t	Temperature;
}Data_LIS2MDL_Temp;

static	bool						Main_Q_Err			= false;
static	tBSP_PER_Target				Main_Device			= eBSP_PER_TARGET_VOID;
static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes		= {	.name = "Q_LIS2MDL"};
static	I2C_HandleTypeDef*			Main_Handle 		= NULL;
static	tCb_Sensor_GetData			Main_CbFunc			= NULL;
static	uint8_t						Main_Setting_Ctrl_A	= 0x80;
static	uint8_t						Main_Setting_Ctrl_B	= 0x00;
static	uint8_t						Main_Setting_Ctrl_C	= 0x01;

static	uint16_t					Main_Timeout		= 50;
static	uint16_t					Main_Delay 			= 5;

static	uint8_t 					Main_TxBuf[2]		= {0};
static	uint8_t 					Main_TxLen			= 0;
static	uint8_t *					Main_RxBuf			= NULL;
static	uint8_t 					Main_RxLen			= 0;
static	tBSP_I2C_Session			Main_Session		= {0};
static	tBSP_PER_DataResp			Main_Per_DataResp	= {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	bool		BSP_LIS2MDL_Transaction(tQ_Cmd Rec);
static	void		BSP_LIS2MDL_Transaction_Tx(bool Rx, tCmd_LIS2MDL Cmd);
static	void		BSP_LIS2MDL_Transaction_Rx(void);
static	bool		BSP_LIS2MDL_Session(void);
static	bool		BSP_LIS2MDL_Transaction_SetData(tCmd_LIS2MDL Cmd);
static	void		BSP_LIS2MDL_SetCtrl(uint8_t	Value, uint8_t Reg);
static	uint8_t		BSP_LIS2MDL_GetCtrl( uint8_t Reg);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_LIS2MDL_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc)
{
	Main_Handle = handle;
	Main_CbFunc	= CbFunc;

	Main_Q	= osMessageQueueNew(16, sizeof(tQ_Cmd), &Q_attributes);

	// MANDATORY! Don't delete this block
	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_LIS2MDL,
										.Function	=	eBSP_PER_FUNC_SET_CTRL,
										.Control	=	Main_Setting_Ctrl_A,
										.idx		=	0};
		BSP_Sensors_Cmd( &Cmd, false);
	}
	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_LIS2MDL,
										.Function	=	eBSP_PER_FUNC_SET_CTRL,
										.Control	=	Main_Setting_Ctrl_C,
										.idx		=	2};
		BSP_Sensors_Cmd( &Cmd, false);
	}
	// MANDATORY! (end block)

	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_LIS2MDL,
										.Function	=	eBSP_PER_FUNC_GET_SN};
		BSP_Sensors_Cmd( &Cmd, false);
	}
}

/**
  * @brief
  * @retval
  */
void 				task_LIS2MDL( void *arguments)
{
	tQ_Cmd Cmd;

	while (1)
	{
		osDelay(1); // Consider whether this is necessary.

		osMessageQueueGet(Main_Q, &Cmd, NULL, osWaitForever);

		uint8_t	msgs = osMessageQueueGetCount(Main_Q);
		if( (Main_Q_Err == true) && (msgs < 4))
		{
			Main_Q_Err	= false;
			BSP_Sensors_SetErr( Main_Device, eBSP_SENS_ERR_Q_LVL, BSP_CLEAR);
		}

		BSP_LIS2MDL_Transaction(Cmd);
	}
}

bool				BSP_LIS2MDL_Cmd( tBSP_PER_DataCmd	*cmd)
{
	tQ_Cmd	Cmd = {0};
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))							return false;

	Cmd.cmd	= 0;
	Cmd.set	= false;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_GET_SN:
		Cmd.cmd	= CMD_LIS2MDL_GET_SN;
		break;

	case	eBSP_PER_FUNC_SET_CTRL:
		Cmd.cmd	= CMD_LIS2MDL_CTRL_REG_A + cmd->idx;
		Cmd.set	= true;
		BSP_LIS2MDL_SetCtrl(cmd->Control, Cmd.cmd);
		break;

	case	eBSP_PER_FUNC_GET_AXIS:
		Cmd.cmd	= CMD_LIS2MDL_GET_AXIS;
		break;

	case	eBSP_PER_FUNC_TEMP_RH:
		Cmd.cmd	= CMD_LIS2MDL_TEMP_L;
		break;
	}

	if( Cmd.cmd == 0)
		result = false;
	else
	{
		osMessageQueuePut(Main_Q, &Cmd, 0, 0);

		Main_Device	= cmd->Target;
		uint8_t	msgs = osMessageQueueGetCount(Main_Q);
		if( (Main_Q_Err == false) && (msgs > 12))
		{
			Main_Q_Err	= true;
			BSP_Sensors_SetErr( cmd->Target, eBSP_SENS_ERR_Q_LVL, BSP_SET);
		}
	}

	return	result;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LIS2MDL_Transaction(tQ_Cmd Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.cmd)
	{
	case	CMD_LIS2MDL_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_LIS2MDL_SN;
		Main_RxLen	= sizeof(Data_LIS2MDL_SN);
		break;

	case	CMD_LIS2MDL_CTRL_REG_A:
	case	CMD_LIS2MDL_CTRL_REG_B:
	case	CMD_LIS2MDL_CTRL_REG_C:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		if( Rec.set == BSP_SET)
			Main_TxBuf[idx ++]	=	BSP_LIS2MDL_GetCtrl( Rec.cmd);
		Main_TxLen = idx;
		if( Rec.set == BSP_GET)
		{
			Main_RxBuf	= (uint8_t *)&Data_LIS2MDL_Control;
			Main_RxLen	= sizeof(Data_LIS2MDL_Control);
		}
		break;

	case	CMD_LIS2MDL_GET_AXIS:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_LIS2MDL_Axis;
		Main_RxLen	= sizeof(Data_LIS2MDL_Axis);
		break;

	case	CMD_LIS2MDL_TEMP_L:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_LIS2MDL_Temp;
		Main_RxLen	= sizeof(Data_LIS2MDL_Temp);
		break;

	default:
		result = false;
		break;
	}

	if( result == true)
	{
		if( Rec.set == BSP_GET)
		{
			BSP_LIS2MDL_Transaction_Tx(true, Rec.cmd);
		}
		else
		{
			BSP_LIS2MDL_Transaction_Tx(false, Rec.cmd);
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LIS2MDL_Transaction_Tx(bool Rx, tCmd_LIS2MDL Cmd)
{
	Main_Session.i2cHandle		= Main_Handle;
	Main_Session.taskHandle		= xTaskGetCurrentTaskHandle();
	Main_Session.Address		= I2C_DEVICE_ADDRESS_LIS2MDL;
	Main_Session.Device			= eBSP_PER_TARGET_LIS2MDL;
	Main_Session.TxBuf			= Main_TxBuf;
	Main_Session.TxLen			= Main_TxLen;
	Main_Session.RxBuf			= NULL;
	Main_Session.RxLen			= 0;
	Main_Session.Timeout		= Main_Timeout;
	Main_Session.DelayAfterTx	= Main_Delay;

	if( Rx)
		BSP_LIS2MDL_Transaction_Rx();

	if( BSP_LIS2MDL_Session() == true)
	{
		if( Rx)
			BSP_LIS2MDL_Transaction_SetData(Cmd);
	}
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LIS2MDL_Transaction_Rx(void)
{
	Main_Session.RxBuf			= Main_RxBuf;
	Main_Session.RxLen			= Main_RxLen;
	Main_Session.DelayAfterRx	= 0;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LIS2MDL_Session(void)
{
	uint32_t	rv;
	BSP_I2C_Cmd(Main_Session);

	rv = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(PER_TIME_NOTIFY)); // will be released by BSP_I2C_Session()::xTaskNotifyGive
	return( rv > 0);
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LIS2MDL_Transaction_SetData(tCmd_LIS2MDL Cmd)
{
	bool result = true;

	switch( Cmd)
	{
	case	CMD_LIS2MDL_GET_SN:
		Main_Per_DataResp.SerialNumber = Main_RxBuf[0];
		break;

	case	CMD_LIS2MDL_GET_AXIS:
		memcpy(Main_Per_DataResp.Axis, Main_RxBuf, sizeof(Data_LIS2MDL_Axis));
		break;

	case	CMD_LIS2MDL_TEMP_L:
		Main_Per_DataResp.Temperature = BSP_Per_Convert(eBSP_PER_TARGET_LIS2MDL, eBSP_PER_FUNC_TEMP, Data_LIS2MDL_Temp.Temperature);
		break;

	default:
		result = false;
		break;
	}

	switch( Cmd)
	{
	case	CMD_LIS2MDL_GET_SN:
	case	CMD_LIS2MDL_TEMP_L:
		printf("LIS2MDL | R: %d | SN: %lX T: %.2f RH: %d\n",
					result,
					Main_Per_DataResp.SerialNumber,
					Main_Per_DataResp.Temperature,
					Main_Per_DataResp.Humidity_i);
		break;

	case	CMD_LIS2MDL_GET_AXIS:
		printf("LIS2MDL Axis | X: %d Y: %d Z: %d\n",
					Main_Per_DataResp.Axis[0],
					Main_Per_DataResp.Axis[1],
					Main_Per_DataResp.Axis[2]);
		break;
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LIS2MDL_SetCtrl(uint8_t	Value, uint8_t Reg)
{
	if( Reg == CMD_LIS2MDL_CTRL_REG_A)	Main_Setting_Ctrl_A	=	Value;
	if( Reg == CMD_LIS2MDL_CTRL_REG_B)	Main_Setting_Ctrl_B	=	Value;
	if( Reg == CMD_LIS2MDL_CTRL_REG_C)	Main_Setting_Ctrl_C	=	Value;
}

/**
  * @brief
  * @retval
  */
static	uint8_t		BSP_LIS2MDL_GetCtrl( uint8_t Reg)
{
	if( Reg == CMD_LIS2MDL_CTRL_REG_A)	return Main_Setting_Ctrl_A;
	if( Reg == CMD_LIS2MDL_CTRL_REG_B)	return Main_Setting_Ctrl_B;
	if( Reg == CMD_LIS2MDL_CTRL_REG_C)	return Main_Setting_Ctrl_C;

	return 0;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : stts22.c
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
#include "STTS22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_STTS22		cmd;
	bool			set;
	uint8_t			reg_data;
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
static struct __PACKED
{
	uint16_t	Temperature;
}Data_Temp;

static struct __PACKED
{
	uint8_t		SN;
}Data_SN;

static struct __PACKED
{
	uint8_t		Control;
}Data_Control;

static struct __PACKED
{
	uint8_t		Status;
}Data_Status;

static struct __PACKED
{
	uint8_t		Data;
}Data_Register;

static	bool						Main_Q_Err			= false;
static	tBSP_PER_Target				Main_Device			= eBSP_PER_TARGET_VOID;
static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes		= {	.name = "Q_STTS22"};
static	I2C_HandleTypeDef*			Main_Handle 		= NULL;
static	tCb_Sensor_GetData			Main_CbFunc			= NULL;
static	uint8_t						Main_Setting_Ctrl	= 0x3C;

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
static	bool		BSP_STTS22_Transaction(tQ_Cmd Rec);
static	uint8_t		BSP_STTS22_Transaction_default(tQ_Cmd Rec);
static	void		BSP_STTS22_Transaction_Tx(bool Rx, tCmd_STTS22 Cmd);
static	void		BSP_STTS22_Transaction_Rx(void);
static	bool		BSP_STTS22_Session(void);
static	bool		BSP_STTS22_Transaction_SetData(tCmd_STTS22 Cmd);
static	void		BSP_STTS22_SetCtrl(uint8_t	Value);
static	uint8_t		BSP_STTS22_GetCtrl(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_STTS22_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc)
{
	tBSP_SENS_Q_Cmd Dest	= {0};
	Main_Handle = handle;
	Main_CbFunc	= CbFunc;

	Main_Q	= osMessageQueueNew(16, sizeof(tQ_Cmd), &Q_attributes);

	// MANDATORY! Don't delete this block
	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_STTS22,
										.Function	=	eBSP_PER_FUNC_SET_CTRL,
										.Control	=	Main_Setting_Ctrl};
		BSP_Sensors_Cmd( &Dest, &Cmd, false);
	}
	// MANDATORY! (end block)

	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_STTS22,
										.Function	=	eBSP_PER_FUNC_GET_SN};
		BSP_Sensors_Cmd( &Dest, &Cmd, false);
	}
}

/**
  * @brief
  * @retval
  */
void 				task_STTS22( void *arguments)
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
#ifdef	MY_DEBUG
	cnt2ok[3]	++;
	val[3]	= msgs;
#endif

		BSP_STTS22_Transaction(Cmd);
	}
}

bool				BSP_STTS22_Cmd( tBSP_PER_DataCmd	*cmd)
{
	tQ_Cmd	Cmd = {0};
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))							return false;

	Cmd.cmd	= 0;
	Cmd.set	= false;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_TEMP:
	case	eBSP_PER_FUNC_TEMP_RH:
		Cmd.cmd	= CMD_STTS22_TEMP_L; // reads also CMD_STTS22_TEMP_H in a single command
		break;

	case	eBSP_PER_FUNC_RH:
		break;

	case	eBSP_PER_FUNC_GET_SN:
		Cmd.cmd	= CMD_STTS22_GET_SN;
		break;

	case	eBSP_PER_FUNC_GET_CTRL:
		Cmd.cmd	= CMD_STTS22_CTRL;
		break;

	case	eBSP_PER_FUNC_SET_CTRL:
		Cmd.cmd	= CMD_STTS22_CTRL;
		Cmd.set	= true;
		BSP_STTS22_SetCtrl(cmd->Control);
		break;

	case	eBSP_PER_FUNC_GET_STATUS:
		Cmd.cmd	= CMD_STTS22_STATUS;
		break;

	default:
		Cmd.cmd 		= cmd->Reg_addr;
		Cmd.reg_data	= cmd->Reg_data;
		Cmd.set			= cmd->Reg_set;
		break;
	}

	if( Cmd.cmd == 0)
		result = false;
	else
	{
		osStatus_t	status;
		status	= osMessageQueuePut(Main_Q, &Cmd, 0, 0);

		Main_Device	= cmd->Target;
		uint8_t	msgs = osMessageQueueGetCount(Main_Q);
		if( (Main_Q_Err == false) && (msgs > 12))
		{
			Main_Q_Err	= true;
			BSP_Sensors_SetErr( cmd->Target, eBSP_SENS_ERR_Q_LVL, BSP_SET);
		}
		result = (status == osOK);

#ifdef	MY_DEBUG
	if( status == osOK)		cnt2ok[2]	++;
	else					cnt2er[2]	++;
	val[2]	= msgs;
#endif
	}

	return	result;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_STTS22_Transaction(tQ_Cmd Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.cmd)
	{
	case	CMD_STTS22_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_SN;
		Main_RxLen	= sizeof(Data_SN);
		break;

	case	CMD_STTS22_TEMP_H_LIMIT:
		Main_TxBuf[idx ++]	=	Rec.cmd;
//		if( Rec.Set == BSP_SET)
//			Main_TxBuf[idx ++]	=	BSP_STTS22_GetTemp_H_Limit(); // ensure the function returns uint16_t
		Main_TxLen = idx;
		if( Rec.set == BSP_GET)
		{
			Main_RxBuf	= (uint8_t *)&Data_Temp;
			Main_RxLen	= sizeof(Data_Temp);
		}
		break;

	case	CMD_STTS22_CTRL:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		if( Rec.set == BSP_SET)
			Main_TxBuf[idx ++]	=	BSP_STTS22_GetCtrl();
		Main_TxLen = idx;
		if( Rec.set == BSP_GET)
		{
			Main_RxBuf	= (uint8_t *)&Data_Control;
			Main_RxLen	= sizeof(Data_Control);
		}
		break;

	case	CMD_STTS22_STATUS:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_Status;
		Main_RxLen	= sizeof(Data_Status);
		break;

	case	CMD_STTS22_TEMP_L:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_Temp;
		Main_RxLen	= sizeof(Data_Temp);
		break;

	default:
		Main_TxLen 	= BSP_STTS22_Transaction_default( Rec);
		break;
	}

	if( result == true)
	{
		if( Rec.set == BSP_GET)
		{
			BSP_STTS22_Transaction_Tx(true, Rec.cmd);
		}
		else
		{
			BSP_STTS22_Transaction_Tx(false, Rec.cmd);
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	uint8_t		BSP_STTS22_Transaction_default(tQ_Cmd Rec)
{
	uint8_t	idx = 0;

	if( Rec.cmd < 0x100)
		Main_TxBuf[idx ++]	=	Rec.cmd;
	else
	{
		Main_TxBuf[idx ++]	=	(Rec.cmd >> 8);
		Main_TxBuf[idx ++]	=	Rec.cmd >> 8;
	}

	if( Rec.set)
		Main_TxBuf[idx ++]	=	Rec.reg_data;
	else
	{
		Main_RxBuf	= (uint8_t *)&Data_Register;
		Main_RxLen	= sizeof(Data_Register);
	}

	return idx;
}
/**
  * @brief
  * @retval
  */
static	void		BSP_STTS22_Transaction_Tx(bool Rx, tCmd_STTS22 Cmd)
{
	Main_Session.i2cHandle		= Main_Handle;
	Main_Session.taskHandle		= xTaskGetCurrentTaskHandle();
	Main_Session.Address		= I2C_DEVICE_ADDRESS_STTS22;
	Main_Session.Device			= eBSP_PER_TARGET_STTS22;
	Main_Session.TxBuf			= Main_TxBuf;
	Main_Session.TxLen			= Main_TxLen;
	Main_Session.RxBuf			= NULL;
	Main_Session.RxLen			= 0;
	Main_Session.Timeout		= Main_Timeout;
	Main_Session.DelayAfterTx	= Main_Delay;

	if( Rx)
		BSP_STTS22_Transaction_Rx();

	if( BSP_STTS22_Session() == true)
	{
		if( Rx)
			BSP_STTS22_Transaction_SetData(Cmd);
	}
}

/**
  * @brief
  * @retval
  */
static	void		BSP_STTS22_Transaction_Rx(void)
{
	Main_Session.RxBuf			= Main_RxBuf;
	Main_Session.RxLen			= Main_RxLen;
	Main_Session.DelayAfterRx	= 0;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_STTS22_Session(void)
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
static	bool		BSP_STTS22_Transaction_SetData(tCmd_STTS22 Cmd)
{
	bool result = true;

	switch( Cmd)
	{
	case	CMD_STTS22_GET_SN:
		Main_Per_DataResp.SerialNumber = Main_RxBuf[0];
		break;

	case	CMD_STTS22_TEMP_H_LIMIT:
		break;

	case	CMD_STTS22_TEMP_L_LIMIT:
		break;

	case	CMD_STTS22_CTRL:
		Main_Per_DataResp.Control = Main_RxBuf[0];
		break;

	case	CMD_STTS22_STATUS:
		Main_Per_DataResp.Status = Main_RxBuf[0];
		break;

	case	CMD_STTS22_TEMP_L:
		Main_Per_DataResp.Temperature	=	BSP_Per_Convert(eBSP_PER_TARGET_STTS22, eBSP_PER_FUNC_TEMP, Data_Temp.Temperature);
		break;

	default:
		Main_Per_DataResp.Reg_addr	= Cmd;
		Main_Per_DataResp.Reg_data	= Data_Register.Data;

#ifdef MY_DEBUG_PRINTF
		printf("STTS22 (%.2x) | Reg: %.2X\n",
					Main_Per_DataResp.Reg_addr,
					Main_Per_DataResp.Reg_data);
#endif
		return true;
	}

#ifdef MY_DEBUG_PRINTF
	printf("STTS22 | R: %d | SN: %lX T: %.2f RH: %d\n",
				result,
				Main_Per_DataResp.SerialNumber,
				Main_Per_DataResp.Temperature,
				Main_Per_DataResp.Humidity_i);
#endif

	return result;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_STTS22_SetCtrl(uint8_t	Value)
{
	Main_Setting_Ctrl	=	Value;
}

/**
  * @brief
  * @retval
  */
static	uint8_t		BSP_STTS22_GetCtrl(void)
{
	return	Main_Setting_Ctrl;
}
/* USER CODE END 4 */

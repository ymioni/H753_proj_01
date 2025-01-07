/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lsm6dso..c
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
#include "LSM6DSO.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_LSM6DSO	cmd;
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
	uint8_t		Data;
}Data_Register;

static	bool						Main_Q_Err			= false;
static	tBSP_PER_Target				Main_Device			= eBSP_PER_TARGET_VOID;
static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes		= {	.name = "Q_LSM6DSO"};
static	I2C_HandleTypeDef*			Main_Handle 		= NULL;
static	tCb_Sensor_GetData			Main_CbFunc			= NULL;

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
static	bool		BSP_LSM6DSO_Transaction(tQ_Cmd Rec);
static	uint8_t		BSP_LSM6DSO_Transaction_default(tQ_Cmd Rec);
static	void		BSP_LSM6DSO_Transaction_Tx(bool Rx, tCmd_LSM6DSO Cmd);
static	void		BSP_LSM6DSO_Transaction_Rx(void);
static	bool		BSP_LSM6DSO_Session(void);
static	bool		BSP_LSM6DSO_Transaction_SetData(tCmd_LSM6DSO Cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_LSM6DSO_Init( I2C_HandleTypeDef *handle, tCb_Sensor_GetData	CbFunc)
{
	Main_Handle = handle;
	Main_CbFunc	= CbFunc;

	Main_Q	= osMessageQueueNew(16, sizeof(tQ_Cmd), &Q_attributes);

	{
		tBSP_PER_DataCmd	Cmd	=	{	.Target		=	eBSP_PER_TARGET_LSM6DSO};

		Cmd.Function	=	eBSP_PER_FUNC_GET_SN;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_SET_REG;
		Cmd.Reg_addr	= CMD_LSM6DSO_CTRL1;
		Cmd.Reg_data	= 0x60;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_SET_REG;
		Cmd.Reg_addr	= CMD_LSM6DSO_CTRL2;
		Cmd.Reg_data	= 0x60;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_SET_REG;
		Cmd.Reg_addr	= CMD_LSM6DSO_CTRL3;
		Cmd.Reg_data	= 0x04;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_GET_REG;
		Cmd.Reg_addr	= CMD_LSM6DSO_CTRL1;
		Cmd.Reg_data	= 0x60;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_GET_REG;
		Cmd.Reg_addr	= CMD_LSM6DSO_CTRL2;
		Cmd.Reg_data	= 0x60;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_GET_REG;
		Cmd.Reg_addr	= CMD_LSM6DSO_CTRL3;
		Cmd.Reg_data	= 0x04;
		BSP_Sensors_Cmd( &Cmd, false);

		Cmd.Function	= eBSP_PER_FUNC_TEMP_RH;
		BSP_Sensors_Cmd( &Cmd, false);
	}
}

/**
  * @brief
  * @retval
  */
void 				task_LSM6DSO( void *arguments)
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
	cnt2ok[11]	++;
	val[11]	= msgs;
#endif

		BSP_LSM6DSO_Transaction(Cmd);
	}
}

bool				BSP_LSM6DSO_Cmd( tBSP_PER_DataCmd	*cmd)
{
	tQ_Cmd	Cmd = {0};
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))							return false;

	Cmd.cmd	= 0;
	Cmd.set	= false;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_GET_SN:
		Cmd.cmd	= CMD_LSM6DSO_GET_SN;
		break;

	case	eBSP_PER_FUNC_TEMP_RH:
		Cmd.cmd = CMD_LSM6DSO_TEMP_L;
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
#ifdef	MY_DEBUG
	if( status == osOK)		cnt2ok[10]	++;
	else					cnt2er[10]	++;
	val[10]	= msgs;
#endif
	}

	return	result;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LSM6DSO_Transaction(tQ_Cmd Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.cmd)
	{
	case	CMD_LSM6DSO_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_SN;
		Main_RxLen	= sizeof(Data_SN);
		break;

	case	CMD_LSM6DSO_TEMP_L:
		Main_TxBuf[idx ++]	=	Rec.cmd;
		Main_TxLen 	= idx;
		Main_RxBuf	= (uint8_t *)&Data_Temp;
		Main_RxLen	= sizeof(Data_Temp);
		break;

	default:
		Main_TxLen 	= BSP_LSM6DSO_Transaction_default( Rec);
		break;
	}

	if( result == true)
	{
		if( Rec.set == BSP_GET)
		{
			BSP_LSM6DSO_Transaction_Tx(true, Rec.cmd);
		}
		else
		{
			BSP_LSM6DSO_Transaction_Tx(false, Rec.cmd);
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	uint8_t		BSP_LSM6DSO_Transaction_default(tQ_Cmd Rec)
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
static	void		BSP_LSM6DSO_Transaction_Tx(bool Rx, tCmd_LSM6DSO Cmd)
{
	Main_Session.i2cHandle		= Main_Handle;
	Main_Session.taskHandle		= xTaskGetCurrentTaskHandle();
	Main_Session.Address		= I2C_DEVICE_ADDRESS_LSM6DSO;
	Main_Session.Device			= eBSP_PER_TARGET_LSM6DSO;
	Main_Session.TxBuf			= Main_TxBuf;
	Main_Session.TxLen			= Main_TxLen;
	Main_Session.RxBuf			= NULL;
	Main_Session.RxLen			= 0;
	Main_Session.Timeout		= Main_Timeout;
	Main_Session.DelayAfterTx	= Main_Delay;

	if( Rx)
		BSP_LSM6DSO_Transaction_Rx();

	if( BSP_LSM6DSO_Session() == true)
	{
		if( Rx)
			BSP_LSM6DSO_Transaction_SetData(Cmd);
	}
}

/**
  * @brief
  * @retval
  */
static	void		BSP_LSM6DSO_Transaction_Rx(void)
{
	Main_Session.RxBuf			= Main_RxBuf;
	Main_Session.RxLen			= Main_RxLen;
	Main_Session.DelayAfterRx	= 0;
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_LSM6DSO_Session(void)
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
static	bool		BSP_LSM6DSO_Transaction_SetData(tCmd_LSM6DSO Cmd)
{
	bool result = true;

	switch( Cmd)
	{
	case	CMD_LSM6DSO_GET_SN:
		Main_Per_DataResp.SerialNumber = Data_SN.SN;
		break;

	case	CMD_LSM6DSO_TEMP_L:
		Main_Per_DataResp.Temperature = BSP_Per_Convert(eBSP_PER_TARGET_LSM6DSO, eBSP_PER_FUNC_TEMP, Data_Temp.Temperature);
		break;

	default:
		Main_Per_DataResp.Reg_addr	= Cmd;
		Main_Per_DataResp.Reg_data	= Data_Register.Data;

		printf("LSM6DSO (%.2x) | Reg: %.2X\n",
					Main_Per_DataResp.Reg_addr,
					Main_Per_DataResp.Reg_data);
		return true;
	}

	printf("LSM6DSO | R: %d | SN: %lX T: %.2f RH: %d\n",
				result,
				Main_Per_DataResp.SerialNumber,
				Main_Per_DataResp.Temperature,
				Main_Per_DataResp.Humidity_i);

	return result;
}

/* USER CODE END 4 */

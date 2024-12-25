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
#include "..\I2C\I2C.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include "SHT40.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_SHT40		Cmd;
	bool			Set;
}tCmdQueue;


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

static	tCb_GetData_SHT40		Main_Cb_GetData_SHT40;
static  tBSP_PER_DataCmd		Main_Per_DataCmd	= {0};
static	tBSP_PER_DataResp		Main_Per_DataResp	= {0};

static 	bool					Main_Active			= false;
static	uint8_t					Main_State			= 0;
static	uint32_t				Main_Target			= 0;

static	I2C_HandleTypeDef*		Main_Handle			= NULL;
static	uint16_t				Main_Timer			= 0;
static	uint16_t				Main_Timeout		= 1000;
static	uint16_t				Main_Delay 			= 50;

static	uint8_t 				Main_TxBuf[2]		= {0};
static	uint8_t 				Main_TxLen			= 0;
static	uint8_t *				Main_RxBuf			= NULL;
static	uint8_t 				Main_RxLen			= 0;

#define MAX_Q_LEN				10
static	tCmdQueue				Main_Q[MAX_Q_LEN]	= {0};
static	uint8_t					Main_Q_Idx_W		= 0;
static	uint8_t					Main_Q_Idx_R		= 0;
static	uint8_t					Main_Q_Idx_Cnt		= 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	bool			BSP_SHT40_Enqueue( tCmd_SHT40 Cmd, bool Set);
static	tCmdQueue		BSP_SHT40_Dequeue( void);
static	bool			BSP_SHT40_Transaction( tCmdQueue Rec);
static	bool			BSP_SHT40_Transaction_TxRx(void);
static	bool			BSP_SHT40_Transaction_Tx(void);
static	bool			BSP_SHT40_Transaction_SetData(tCmd_SHT40 Cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
bool			BSP_SHT40_Init( I2C_HandleTypeDef *handle, tCb_GetData_SHT40	CbFunc)
{
	if( BSP_RespCodes_Assert_BSP((handle == NULL), BSP_ERROR_HANDLE_ERR))				return false;
	if( BSP_RespCodes_Assert_BSP((CbFunc == NULL), BSP_ERROR_PARAM_NULL))				return false;

	Main_Handle					= handle;
	Main_Cb_GetData_SHT40		= CbFunc;

	Main_Per_DataResp.Address	= (I2C_DEVICE_ADDRESS_SHT40 >> 1);

	Main_Active = 	true;
	Main_Target	=	0;
	Main_State	=	0;

	Main_Per_DataCmd.handle		=	handle;

	// Optional
	Main_Per_DataCmd.Function	=	eBSP_PER_FUNC_GET_SN;
	BSP_SHT40_Cmd( &Main_Per_DataCmd);

	return true;
}

/**
  * @brief
  * @retval
  */
void 			BSP_SHT40_MainLoop( void)
{
	if( Main_Active == false)			return;
	if( HAL_GetTick() < Main_Target)	return;

	switch(Main_State)
	{
	case	0:
		tCmdQueue Rec = BSP_SHT40_Dequeue();
		if( Rec.Cmd != 0)
		{
			BSP_SHT40_Transaction(Rec);

			switch( Rec.Cmd)
			{
			case	CMD_SHT40_GET_TEMP_RH_PRECISION_HI:
			case	CMD_SHT40_GET_TEMP_RH_PRECISION_MED:
			case	CMD_SHT40_GET_TEMP_RH_PRECISION_LO:
				Main_Timer	=	5000;
				Main_State ++;
				break;

			default:
				break;
			}
		}
		return;

	case	1:
		Main_Per_DataCmd.Function	=	eBSP_PER_FUNC_TEMP_RH;
		BSP_SHT40_Cmd( &Main_Per_DataCmd);
		Main_State = 0;
		break;

	default:
		Main_Timer	=	0;
		Main_State	=	0;
		break;
	}

	Main_Target = HAL_GetTick() + Main_Timer;
}

/**
  * @brief
  * @retval
  */
bool			BSP_SHT40_Cmd( tBSP_PER_DataCmd	*cmd)
{
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))				return false;
	if( BSP_RespCodes_Assert_BSP((cmd->handle == NULL), BSP_ERROR_HANDLE_ERR))		return false;

	Main_Per_DataCmd 	= *cmd;
	Main_Handle			= cmd->handle;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_TEMP:
	case	eBSP_PER_FUNC_RH:
	case	eBSP_PER_FUNC_TEMP_RH:
		switch( cmd->Precision)
		{
		case	eBSP_PER_PRCSN_LOW:		result = BSP_SHT40_Enqueue(CMD_SHT40_GET_TEMP_RH_PRECISION_LO, BSP_GET);	break;
		case	eBSP_PER_PRCSN_MED:		result = BSP_SHT40_Enqueue(CMD_SHT40_GET_TEMP_RH_PRECISION_MED, BSP_GET);	break;
		case	eBSP_PER_PRCSN_HIGH:
		default:						result = BSP_SHT40_Enqueue(CMD_SHT40_GET_TEMP_RH_PRECISION_HI, BSP_GET);	break;
		}
		break;

	case	eBSP_PER_FUNC_GET_SN:
		result = BSP_SHT40_Enqueue(CMD_SHT40_GET_SN, BSP_GET);
		break;

	case	eBSP_PER_FUNC_RESET:
		result = BSP_SHT40_Enqueue(CMD_SHT40_RESET, BSP_SET);
		break;

	case	eBSP_PER_FUNC_HEATER:
		switch( cmd->Power_mW)
		{
		case	200:
			switch( cmd->Time_msec)
			{
			case	1000:	result = BSP_SHT40_Enqueue(CMD_SHT40_HEATER_200MW_1000MSEC, BSP_SET);	break;
			case	100:	result = BSP_SHT40_Enqueue(CMD_SHT40_HEATER_200MW_100MSEC, BSP_SET);	break;
			}
			break;

		case	110:
			switch( cmd->Time_msec)
			{
			case	1000:	result = BSP_SHT40_Enqueue(CMD_SHT40_HEATER_110MW_1000MSEC, BSP_SET);	break;
			case	100:	result = BSP_SHT40_Enqueue(CMD_SHT40_HEATER_110MW_100MSEC, BSP_SET);	break;
			}
			break;

		case	20:
			switch( cmd->Time_msec)
			{
			case	1000:	result = BSP_SHT40_Enqueue(CMD_SHT40_HEATER_20MW_1000MSEC, BSP_SET);	break;
			case	100:	result = BSP_SHT40_Enqueue(CMD_SHT40_HEATER_20MW_100MSEC, BSP_SET);		break;
			}
			break;
		}
		break;

	default:
		return false;
		break;
	}

	return	result;
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_SHT40_Enqueue( tCmd_SHT40 Cmd, bool Set)
{
	tCmdQueue Rec = {.Cmd = Cmd, .Set = Set};

	if( Main_Q_Idx_Cnt >= MAX_Q_LEN)
		return false;

	if( Main_Q_Idx_W >= MAX_Q_LEN)	Main_Q_Idx_W	=	0;
	Main_Q[Main_Q_Idx_W ++]	=	Rec;
	if( Main_Q_Idx_W >= MAX_Q_LEN)	Main_Q_Idx_W	=	0;
	Main_Q_Idx_Cnt	++;

	return true;
}

/**
  * @brief
  * @retval
  */
static	tCmdQueue		BSP_SHT40_Dequeue( void)
{
	tCmdQueue	Rec = {0};

	if(Main_Q_Idx_Cnt > 0)
	{
		if( Main_Q_Idx_R >= MAX_Q_LEN)	Main_Q_Idx_R	=	0;
		Rec = Main_Q[Main_Q_Idx_R ++];
		if( Main_Q_Idx_W >= MAX_Q_LEN)	Main_Q_Idx_R	=	0;
		Main_Q_Idx_Cnt --;
	}

	return Rec;
}

/**
  * @brief
  * @retval
  */
static	bool	BSP_SHT40_Transaction(tCmdQueue Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.Cmd)
	{
	case	CMD_SHT40_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.Cmd;
		Main_TxLen = idx;
		Main_RxBuf	= (uint8_t *)&Data_SHT40_SN;
		Main_RxLen	= sizeof(Data_SHT40_SN);
		break;

	case	CMD_SHT40_RESET:
		Main_TxBuf[idx ++]	=	Rec.Cmd;
		Main_TxLen = idx;
		Main_RxLen	= 0;
		break;

	case	CMD_SHT40_GET_TEMP_RH_PRECISION_HI:
	case	CMD_SHT40_GET_TEMP_RH_PRECISION_MED:
	case	CMD_SHT40_GET_TEMP_RH_PRECISION_LO:
		Main_TxBuf[idx ++]	=	Rec.Cmd;
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
		Main_TxBuf[idx ++]	=	Rec.Cmd;
		Main_TxLen = idx;
		Main_RxLen	= 0;
		break;

	default:
		result = false;
		break;
	}

	if( result == true)
	{
		if( Rec.Set == BSP_GET)
		{
			result = BSP_SHT40_Transaction_TxRx();

			if( result == true)
			{
				if( BSP_SHT40_Transaction_SetData(Rec.Cmd) == true)
				{
					if(Main_Cb_GetData_SHT40 != NULL)
						Main_Cb_GetData_SHT40(&Main_Per_DataResp);
				}
			}
		}
		else
		{
			result = BSP_SHT40_Transaction_Tx();
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_SHT40_Transaction_TxRx(void)
{
	HAL_StatusTypeDef	HAL_Result;

	if( BSP_SHT40_Transaction_Tx() == false)
		return false;
	else
	{
		HAL_Result = HAL_I2C_Master_Receive(Main_Handle, I2C_DEVICE_ADDRESS_SHT40, Main_RxBuf, Main_RxLen, Main_Timeout);
		if( BSP_RespCodes_Assert_HAL((HAL_Result != HAL_OK), eBSP_RESP_CODE_HAL_ERR, HAL_Result, Main_Handle))	return false;
		return true;
	}
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_SHT40_Transaction_Tx(void)
{
	HAL_StatusTypeDef	HAL_Result;

	if( BSP_RespCodes_Assert_BSP((Main_TxLen == 0), BSP_ERROR_PARAM_ZERO))				return false;
	if( BSP_RespCodes_Assert_BSP((Main_TxBuf[0] == 0), BSP_ERROR_PARAM_ZERO))			return false;

	HAL_Result = HAL_I2C_Master_Transmit(Main_Handle, I2C_DEVICE_ADDRESS_SHT40, Main_TxBuf, Main_TxLen, Main_Timeout);
	HAL_Delay(Main_Delay);

	if( BSP_RespCodes_Assert_HAL((HAL_Result != HAL_OK), eBSP_RESP_CODE_HAL_ERR, HAL_Result, Main_Handle))	return false;

	return true;
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

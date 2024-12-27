/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lps22d.c
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
#include "LPS22D.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_LPS22D		Cmd;
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
	uint8_t		SN;
}Data_LPS22D_SN;

struct __PACKED
{
	uint8_t		Status;
}Data_LPS22D_Status;

static	tCb_GetData_LPS22D		Main_Cb_GetData_LPS22D;
static  tBSP_PER_DataCmd		Main_Per_DataCmd	= {0};
static	tBSP_PER_DataResp		Main_Per_DataResp	= {0};

static 	bool					Main_Active			= false;
static	uint8_t					Main_State			= 0;
static	uint32_t				Main_Target			= 0;

static	uint8_t					Main_Setting_Ctrl	= 0x3C;

static	I2C_HandleTypeDef*		Main_Handle			= NULL;
static	uint16_t				Main_Timer			= 0;
static	uint16_t				Main_Timeout		= 1000;
static	uint16_t				Main_Delay 			= 50;

static	uint8_t 				Main_TxBuf[2]		= {0};
static	uint8_t 				Main_TxLen			= 0;
static	uint8_t *				Main_RxBuf			= NULL;
static	uint8_t 				Main_RxLen			= 0;
static	uint16_t 				Main_RxVal16b		= 0;

#define MAX_Q_LEN				10
static	tCmdQueue				Main_Q[MAX_Q_LEN]	= {0};
static	uint8_t					Main_Q_Idx_W		= 0;
static	uint8_t					Main_Q_Idx_R		= 0;
static	uint8_t					Main_Q_Idx_Cnt		= 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	void			BSP_LPS22D_SetCtrl(uint8_t	Value);
static	uint8_t			BSP_LPS22D_GetCtrl(void);
static	bool			BSP_LPS22D_Enqueue( tCmd_LPS22D Cmd, bool Set);
static	tCmdQueue		BSP_LPS22D_Dequeue( void);
static	bool			BSP_LPS22D_Transaction( tCmdQueue Rec);
static	bool			BSP_LPS22D_Transaction_TxRx(void);
static	bool			BSP_LPS22D_Transaction_Tx(void);
static	bool			BSP_LPS22D_Transaction_SetData(tCmd_LPS22D Cmd);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
bool			BSP_LPS22D_Init( I2C_HandleTypeDef *handle, tCb_GetData_LPS22D	CbFunc)
{
	if( BSP_RespCodes_Assert_BSP((handle == NULL), BSP_ERROR_HANDLE_ERR))				return false;
	if( BSP_RespCodes_Assert_BSP((CbFunc == NULL), BSP_ERROR_PARAM_NULL))				return false;

	Main_Handle					= handle;
	Main_Cb_GetData_LPS22D		= CbFunc;

	Main_Per_DataResp.Address	= (I2C_DEVICE_ADDRESS_LPS22D >> 1);

	Main_Active = 	true;
	Main_Target	=	0;
	Main_State	=	0;

	Main_Per_DataCmd.handle		=	handle;

	// MANDATORY! Don't delete this block
	// MANDATORY! (end block)

	// Optional
	Main_Per_DataCmd.Function	=	eBSP_PER_FUNC_GET_SN;
	BSP_LPS22D_Cmd( &Main_Per_DataCmd);

	return true;
}

/**
  * @brief
  * @retval
  */
void 			BSP_LPS22D_MainLoop( void)
{
	if( Main_Active == false)			return;
	if( HAL_GetTick() < Main_Target)	return;

	switch(Main_State)
	{
	case	0:
		tCmdQueue Rec = BSP_LPS22D_Dequeue();
		if( Rec.Cmd != 0)
		{
			BSP_LPS22D_Transaction(Rec);

//			if( Rec.Cmd == CMD_LPS22D_TEMP_H)
//			{
//				Main_Timer	=	100;
//				Main_State ++;
//			}
		}
		return;

	case	1:
		Main_Per_DataCmd.Function	=	eBSP_PER_FUNC_TEMP;
		BSP_LPS22D_Cmd( &Main_Per_DataCmd);
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
bool			BSP_LPS22D_Cmd( tBSP_PER_DataCmd	*cmd)
{
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))				return false;
	if( BSP_RespCodes_Assert_BSP((cmd->handle == NULL), BSP_ERROR_HANDLE_ERR))		return false;

	Main_Per_DataCmd 	= *cmd;
	Main_Handle			= cmd->handle;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_GET_SN:
		result = BSP_LPS22D_Enqueue(CMD_LPS22D_GET_SN, BSP_GET);
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
static	void			BSP_LPS22D_SetCtrl(uint8_t	Value)
{
	Main_Setting_Ctrl	=	Value;
}

/**
  * @brief
  * @retval
  */
static	uint8_t			BSP_LPS22D_GetCtrl(void)
{
	return	Main_Setting_Ctrl;
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LPS22D_Enqueue( tCmd_LPS22D Cmd, bool Set)
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
static	tCmdQueue		BSP_LPS22D_Dequeue( void)
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
static	bool	BSP_LPS22D_Transaction(tCmdQueue Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.Cmd)
	{
	case	CMD_LPS22D_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.Cmd;
		Main_TxLen = idx;
		Main_RxBuf	= (uint8_t *)&Data_LPS22D_SN;
		Main_RxLen	= sizeof(Data_LPS22D_SN);
		break;

	default:
		result = false;
		break;
	}

	if( result == true)
	{
		if( Rec.Set == BSP_GET)
		{
			result = BSP_LPS22D_Transaction_TxRx();

			if( result == true)
			{
				if( BSP_LPS22D_Transaction_SetData(Rec.Cmd) == true)
				{
					if(Main_Cb_GetData_LPS22D != NULL)
						Main_Cb_GetData_LPS22D(&Main_Per_DataResp);
				}
			}
		}
		else
		{
			result = BSP_LPS22D_Transaction_Tx();
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LPS22D_Transaction_TxRx(void)
{
	HAL_StatusTypeDef	HAL_Result;

	if( BSP_LPS22D_Transaction_Tx() == false)
		return false;
	else
	{
		HAL_Result = HAL_I2C_Master_Receive(Main_Handle, I2C_DEVICE_ADDRESS_LPS22D, Main_RxBuf, Main_RxLen, Main_Timeout);
		if( BSP_RespCodes_Assert_HAL((HAL_Result != HAL_OK), eBSP_RESP_CODE_HAL_ERR, HAL_Result, Main_Handle))	return false;
		return true;
	}
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LPS22D_Transaction_Tx(void)
{
	HAL_StatusTypeDef	HAL_Result;

	if( BSP_RespCodes_Assert_BSP((Main_TxLen == 0), BSP_ERROR_PARAM_ZERO))				return false;
	if( BSP_RespCodes_Assert_BSP((Main_TxBuf[0] == 0), BSP_ERROR_PARAM_ZERO))			return false;

	HAL_Result = HAL_I2C_Master_Transmit(Main_Handle, I2C_DEVICE_ADDRESS_LPS22D, Main_TxBuf, Main_TxLen, Main_Timeout);
	HAL_Delay(Main_Delay);

	if( BSP_RespCodes_Assert_HAL((HAL_Result != HAL_OK), eBSP_RESP_CODE_HAL_ERR, HAL_Result, Main_Handle))	return false;

	return true;
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LPS22D_Transaction_SetData(tCmd_LPS22D Cmd)
{
	bool result = true;

	switch( Cmd)
	{
	case	CMD_LPS22D_GET_SN:
		Main_Per_DataResp.SerialNumber = Main_RxBuf[0];
		break;

	default:
		result = false;
		break;
	}

	return result;
}

/* USER CODE END 4 */

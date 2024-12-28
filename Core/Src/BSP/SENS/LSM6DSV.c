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
#include "..\I2C\I2C.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include "LSM6DSV.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	struct
{
	tCmd_LSM6DSV		Cmd;
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
}Data_LSM6DSV_SN;

static	tCb_GetData_LSM6DSV		Main_Cb_GetData_LSM6DSV;
static  tBSP_PER_DataCmd		Main_Per_DataCmd	= {0};
static	tBSP_PER_DataResp		Main_Per_DataResp	= {0};

static 	bool					Main_Active			= false;
static	uint8_t					Main_State			= 0;
static	uint32_t				Main_Target			= 0;

static	I2C_HandleTypeDef*		Main_Handle			= NULL;
static	uint16_t				Main_Timer			= 0;
static	uint16_t				Main_Timeout		= 1000;
static	uint16_t				Main_Delay 			= 50;
static	tCmd_LSM6DSV				Main_Cmd			= 0;
static  bool					Main_Set			= false;
static  bool					Main_Wait4Rx		= false;

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
static	bool			BSP_LSM6DSV_Enqueue( tCmd_LSM6DSV Cmd, bool Set);
static	tCmdQueue		BSP_LSM6DSV_Dequeue( void);
static	bool			BSP_LSM6DSV_Transaction( tCmdQueue Rec);
static	bool			BSP_LSM6DSV_Transaction_TxRx(void);
static	bool			BSP_LSM6DSV_Transaction_Tx(void);
static	bool			BSP_LSM6DSV_Transaction_Rx(void);
static	bool			BSP_LSM6DSV_Transaction_SetData(tCmd_LSM6DSV Cmd);
static	void			BSP_LSM6DSV_Cb_TxDone(bool result);
static	void			BSP_LSM6DSV_Cb_RxDone(bool result);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
bool			BSP_LSM6DSV_Init( I2C_HandleTypeDef *handle, tCb_GetData_LSM6DSV	CbFunc)
{
	if( BSP_RespCodes_Assert_BSP((handle == NULL), BSP_ERROR_HANDLE_ERR))				return false;
	if( BSP_RespCodes_Assert_BSP((CbFunc == NULL), BSP_ERROR_PARAM_NULL))				return false;

	Main_Handle					= handle;
	Main_Cb_GetData_LSM6DSV		= CbFunc;

	Main_Per_DataResp.Address	= (I2C_DEVICE_ADDRESS_LSM6DSV >> 1);

	Main_Active = 	true;
	Main_Target	=	0;
	Main_State	=	0;

	Main_Per_DataCmd.handle		=	handle;

	// Optional
	Main_Per_DataCmd.Function	=	eBSP_PER_FUNC_GET_SN;
	BSP_LSM6DSV_Cmd( &Main_Per_DataCmd);

	return true;
}

/**
  * @brief
  * @retval
  */
void 			BSP_LSM6DSV_MainLoop( void)
{
	if( Main_Active == false)			return;
	if( HAL_GetTick() < Main_Target)	return;

	switch(Main_State)
	{
	case	0:
		if( BSP_I2C_IsBusy())	break;

		tCmdQueue Rec = BSP_LSM6DSV_Dequeue();
		if( Rec.Cmd != 0)
		{
			BSP_LSM6DSV_Transaction(Rec);
			Main_State ++;
		}
		break;

	case	1:
		break;

	case	2:
		Main_Timer	=	Main_Delay;
		Main_State ++;
		break;

	case	3:
		if( Main_Wait4Rx == true)
		{
			BSP_LSM6DSV_Transaction_Rx();
			Main_Timer	=	Main_Delay;
		}
		Main_State 	=	0;
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
bool			BSP_LSM6DSV_Cmd( tBSP_PER_DataCmd	*cmd)
{
	bool	result = true;

	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))				return false;
	if( BSP_RespCodes_Assert_BSP((cmd->handle == NULL), BSP_ERROR_HANDLE_ERR))		return false;

	Main_Per_DataCmd 	= *cmd;
	Main_Handle			= cmd->handle;

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_GET_SN:
		result = BSP_LSM6DSV_Enqueue(CMD_LSM6DSV_GET_SN, BSP_GET);
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
static	bool			BSP_LSM6DSV_Enqueue( tCmd_LSM6DSV Cmd, bool Set)
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
static	tCmdQueue		BSP_LSM6DSV_Dequeue( void)
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
static	bool	BSP_LSM6DSV_Transaction(tCmdQueue Rec)
{
	bool	result = true;
	uint8_t	idx = 0;

	switch(Rec.Cmd)
	{
	case	CMD_LSM6DSV_GET_SN:
		Main_TxBuf[idx ++]	=	Rec.Cmd;
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
		Main_Cmd = Rec.Cmd;
		Main_Set = Rec.Set;
		if( Main_Set == BSP_GET)
		{
			result = BSP_LSM6DSV_Transaction_TxRx();
		}
		else
		{
			result = BSP_LSM6DSV_Transaction_Tx();
		}
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LSM6DSV_Transaction_TxRx(void)
{
	Main_Wait4Rx = true;
	return( BSP_LSM6DSV_Transaction_Tx());
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LSM6DSV_Transaction_Tx(void)
{
	tBSP_I2C_TxRx	BSP_I2C_TxRx = {	.handle		= Main_Handle,
										.Address	= I2C_DEVICE_ADDRESS_LSM6DSV,
										.Device		= eBSP_PER_TARGET_LSM6DSV,
										.pData		= Main_TxBuf,
										.Size		= Main_TxLen,
										.Timeout	= (Main_Set == BSP_SET) ? 0 : Main_Timeout,
										.Cb_TxDone	= BSP_LSM6DSV_Cb_TxDone};
	while(BSP_I2C_IsBusy());
	return(BSP_I2C_Transmit_IT(&BSP_I2C_TxRx));
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LSM6DSV_Transaction_Rx(void)
{
	bool	result;

	tBSP_I2C_TxRx	BSP_I2C_TxRx = {	.handle		= Main_Handle,
										.Address	= I2C_DEVICE_ADDRESS_LSM6DSV,
										.Device		= eBSP_PER_TARGET_LSM6DSV,
										.pData		= Main_RxBuf,
										.Size		= Main_RxLen,
										.Timeout	= Main_Timeout,
										.Cb_RxDone	= BSP_LSM6DSV_Cb_RxDone};
	result =	BSP_I2C_Receive_IT(&BSP_I2C_TxRx);
	if( result == false)
		Main_Wait4Rx = false;

	return( result);
}

/**
  * @brief
  * @retval
  */
static	void			BSP_LSM6DSV_Cb_TxDone(bool result)
{
	if( result == false)
	{
		Main_Wait4Rx = false;
		return; // Timeout
	}

	Main_State = 2;
}

/**
  * @brief
  * @retval
  */
static	void			BSP_LSM6DSV_Cb_RxDone(bool result)
{
	Main_Wait4Rx = false;
	if( result == false)
		return; // Timeout

	if( BSP_LSM6DSV_Transaction_SetData(Main_Cmd) == true)
	{
		if(Main_Cb_GetData_LSM6DSV != NULL)
			Main_Cb_GetData_LSM6DSV(&Main_Per_DataResp);
	}
}

/**
  * @brief
  * @retval
  */
static	bool			BSP_LSM6DSV_Transaction_SetData(tCmd_LSM6DSV Cmd)
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

	return result;
}

/* USER CODE END 4 */

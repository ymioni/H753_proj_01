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
#include "..\I2C\I2C.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include "STTS22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct __PACKED
{
	uint8_t		Temperature;
}Data_STTS22_Temp;

struct __PACKED
{
	uint8_t		SN;
}Data_STTS22_SN;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static	tCb_GetData_STTS22		Main_Cb_GetData_STTS22;
static	tBSP_PER_DataResp		Main_CbFunc_Data	= {0};
static	tCmd_STTS22 			Main_Cmd 			= 0;
static	uint16_t				Main_Timeout		= 1000;
static	uint16_t				Main_Delay 			= 50;
static	uint8_t *				Main_RxBuf			= NULL;
static	uint8_t 				Main_RxLen			= 0;
static	I2C_HandleTypeDef*		Main_Handle			= NULL;

static 	bool					Main_Active			= false;
static	uint8_t					Main_State			= 0;
static	uint16_t				Main_Timer			= 0;
static	uint32_t				Main_Target			= 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	bool	BSP_STTS22_Transaction( void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
bool			BSP_STTS22_Init( tCb_GetData_STTS22	CbFunc)
{
	if( BSP_RespCodes_Assert_BSP((CbFunc == NULL), BSP_ERROR_PARAM_NULL))				return false;

	Main_Cb_GetData_STTS22	=	CbFunc;

	return true;
}

/**
  * @brief
  * @retval
  */
void 			BSP_STTS22_MainLoop( void)
{
	if( Main_Active == false)			return;
	if( HAL_GetTick() < Main_Target)	return;

	switch(Main_State)
	{
	case	0:
		return;

	case	1:
		Main_Timer	=	5000;
		BSP_STTS22_Transaction();
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
bool			BSP_STTS22_Cmd( tBSP_PER_DataCmd	*cmd)
{
	bool	result = true;

	printf("BSP_STTS22_Cmd (%d %d)\n", cmd->Function, cmd->Precision);

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_TEMP:
	case	eBSP_PER_FUNC_RH:
	case	eBSP_PER_FUNC_TEMP_RH:
		Main_RxBuf		= (uint8_t *)&Data_STTS22_Temp;
		Main_RxLen		= sizeof(Data_STTS22_Temp);
		Main_Cmd		= CMD_STTS22_TEMP_H;
		break;

	case	eBSP_PER_FUNC_GET_SN	:
		Main_RxBuf		= (uint8_t *)&Data_STTS22_SN;
		Main_RxLen		= sizeof(Data_STTS22_SN);
		Main_Cmd		= CMD_STTS22_GET_SN;
		break;

	default:	result = false;	break;
	}

	printf("[1] result (%d)\n", result);

	if( result == true)
	{
		Main_Handle	=	cmd->handle;

		result = BSP_STTS22_Transaction();
		printf("[2] result (%d)\n", result);
		if( result == true)
		{
			Main_Active	=	true;
			Main_State	=	1;
		}
	}

	return	result;
}

/**
  * @brief
  * @retval
  */
static	bool	BSP_STTS22_Transaction(void)
{
	HAL_StatusTypeDef	HAL_Result;
	uint16_t			Data16b;

	HAL_Result = HAL_I2C_Master_Transmit(Main_Handle, I2C_DEVICE_ADDRESS_STTS22, &Main_Cmd, 1, Main_Timeout);
	printf("[1] HAL_I2C_Master_Transmit = %d\n", HAL_Result);

	if( HAL_Result == HAL_OK)
	{
		HAL_Delay(Main_Delay);
		HAL_Result = HAL_I2C_Master_Receive(Main_Handle, I2C_DEVICE_ADDRESS_STTS22, Main_RxBuf, Main_RxLen, Main_Timeout);
		printf("[2] HAL_I2C_Master_Receive = %d\n", HAL_Result);
	}

	if( Main_Cmd == CMD_STTS22_TEMP_H)
	{
		if( HAL_Result == HAL_OK)
		{
			Data16b	=	Data_STTS22_Temp.Temperature;
			printf("[1] Data16b %d\n", Data16b);
			Data16b	<<= 8;
			printf("[2] Data16b %d\n", Data16b);
			Main_Cmd	=	CMD_STTS22_TEMP_L;
			HAL_Result = HAL_I2C_Master_Transmit(Main_Handle, I2C_DEVICE_ADDRESS_STTS22, &Main_Cmd, 1, Main_Timeout);
			printf("[3] HAL_I2C_Master_Transmit = %d\n", HAL_Result);
		}

		if( HAL_Result == HAL_OK)
		{
			HAL_Delay(Main_Delay);
			HAL_Result = HAL_I2C_Master_Receive(Main_Handle, I2C_DEVICE_ADDRESS_STTS22, Main_RxBuf, Main_RxLen, Main_Timeout);
			printf("[4] HAL_I2C_Master_Transmit = %d\n", HAL_Result);
		}

		if( HAL_Result == HAL_OK)
		{
			Data16b	+=	Data_STTS22_Temp.Temperature;
			printf("[3] Data16b %d\n", Data16b);
			Main_CbFunc_Data.Temperature	= 	BSP_Per_Convert( eBSP_PER_TARGET_STTS22, eBSP_PER_FUNC_TEMP, Data16b);
			Main_CbFunc_Data.Humidity_f		=	0.0f;
			Main_CbFunc_Data.Humidity_i		=	Main_CbFunc_Data.Humidity_f;
		}

		Main_Cmd = CMD_STTS22_TEMP_H;
	}

	if( Main_Cmd == CMD_STTS22_GET_SN)
	{
		Main_CbFunc_Data.SerialNumber	=	Data_STTS22_SN.SN;
	}

	if( HAL_Result == HAL_OK)
	{
		Main_Cb_GetData_STTS22( &Main_CbFunc_Data);
	}

	return (HAL_Result == HAL_OK);
}

/* USER CODE END 4 */

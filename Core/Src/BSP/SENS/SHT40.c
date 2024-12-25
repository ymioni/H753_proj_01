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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static	tCb_GetData_SHT40		Main_Cb_GetData_SHT40;
static	tBSP_PER_DataResp		Main_CbFunc_Data	= {0};
static	tCmd_SHT40 				Main_Cmd 			= 0;
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
static	bool	BSP_SHT40_Transaction( void);

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
	if( BSP_RespCodes_Assert_BSP((CbFunc == NULL), BSP_ERROR_PARAM_NULL))				return false;

	Main_Cb_GetData_SHT40	=	CbFunc;

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
		return;

	case	1:
		Main_Timer	=	5000;
		BSP_SHT40_Transaction();
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

	// printf("BSP_SHT40_Cmd (%d %d)\n", cmd->Function, cmd->Precision);

	switch(cmd->Function)
	{
	case	eBSP_PER_FUNC_TEMP:
	case	eBSP_PER_FUNC_RH:
	case	eBSP_PER_FUNC_TEMP_RH:
		Main_RxBuf		= (uint8_t *)&Data_SHT40_Temp;
		Main_RxLen		= sizeof(Data_SHT40_Temp);

		switch(cmd->Precision)
		{
		case	eBSP_PER_PRCSN_LOW	:	Main_Cmd	=	CMD_SHT40_GET_TEMP_RH_PRECISION_LO;		break;
		case	eBSP_PER_PRCSN_MED	:	Main_Cmd	=	CMD_SHT40_GET_TEMP_RH_PRECISION_MED;	break;
		case	eBSP_PER_PRCSN_HIGH	:	Main_Cmd	=	CMD_SHT40_GET_TEMP_RH_PRECISION_HIGH;	break;
		default:	result = false;																break;
		}
		break;

	case	eBSP_PER_FUNC_HEATER	:	result = false; /* TBD */	break;
	case	eBSP_PER_FUNC_GET_SN	:
		Main_RxBuf		= (uint8_t *)&Data_SHT40_SN;
		Main_RxLen		= sizeof(Data_SHT40_SN);
		Main_Cmd	=	CMD_SHT40_GET_SN;
		break;

	default:	result = false;	break;
	}

	// printf("[1] result (%d)\n", result);

	if( result == true)
	{
		Main_Handle	=	cmd->handle;

		result = BSP_SHT40_Transaction();
		// printf("[2] result (%d)\n", result);
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
static	bool	BSP_SHT40_Transaction(void)
{
	HAL_StatusTypeDef	HAL_Result;

	HAL_Result = HAL_I2C_Master_Transmit(Main_Handle, I2C_DEVICE_ADDRESS_SHT40, &Main_Cmd, 1, Main_Timeout);
	// printf("[1] HAL_I2C_Master_Transmit = %d\n", HAL_Result);

	if( HAL_Result == HAL_OK)
	{
		HAL_Delay(Main_Delay);
		HAL_Result = HAL_I2C_Master_Receive(Main_Handle, I2C_DEVICE_ADDRESS_SHT40, Main_RxBuf, Main_RxLen, Main_Timeout);
		// printf("[2] HAL_I2C_Master_Receive = %d\n", HAL_Result);
	}

	if( HAL_Result == HAL_OK)
	{
		BSP_Util_SwapBytes(&Data_SHT40_Temp.Temperature, sizeof(Data_SHT40_Temp.Temperature));
		BSP_Util_SwapBytes(&Data_SHT40_Temp.Humidity, sizeof(Data_SHT40_Temp.Humidity));

		Main_CbFunc_Data.Temperature	=	BSP_Per_Convert( eBSP_PER_TARGET_SHT40A, eBSP_PER_FUNC_TEMP, Data_SHT40_Temp.Temperature);
		Main_CbFunc_Data.Humidity_f		=	BSP_Per_Convert( eBSP_PER_TARGET_SHT40A, eBSP_PER_FUNC_RH, Data_SHT40_Temp.Humidity);
		Main_CbFunc_Data.Humidity_i		=	Main_CbFunc_Data.Humidity_f;

		Main_Cb_GetData_SHT40( &Main_CbFunc_Data);
	}

	return (HAL_Result == HAL_OK);
}

/* USER CODE END 4 */

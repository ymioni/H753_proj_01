/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : I2C.c
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

#include "I2C.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include ".\SENS\SHT40.h"
#include ".\SENS\STTS22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static	I2C_HandleTypeDef*	Main_Handle	= NULL;
static	tBSP_PER_DataCmd	Main_cmd	= {0};
static	tBSP_PER_DataResp	Main_resp	= {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void			BSP_I2C_Cb_GetData_SHT40( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_STTS22( tBSP_PER_DataResp *Data);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
bool			BSP_I2C_Init( I2C_HandleTypeDef *handle)
{
	BSP_SHT40_Init( handle, BSP_I2C_Cb_GetData_SHT40);
	BSP_STTS22_Init( handle, BSP_I2C_Cb_GetData_STTS22);
	return true;
}

static void			BSP_I2C_Cb_GetData_SHT40( tBSP_PER_DataResp *Data)
{
	printf("[SHT40A] CbFunc OK!!! Addr:%.2X, SN:%lX, Temp:%.2f, H:%d\n\n",
			Data->Address,
			Data->SerialNumber,
			Data->Temperature,
			Data->Humidity_i);
}

static void			BSP_I2C_Cb_GetData_STTS22( tBSP_PER_DataResp *Data)
{
	printf("[STTS22] CbFunc OK!!! Addr:%.2X, SN:%lX, CTRL:%.2X, STAT:%.2X, Temp:%.2f, H:%d\n\n",
			Data->Address,
			Data->SerialNumber,
			Data->Control,
			Data->Status,
			Data->Temperature,
			Data->Humidity_i);
}


/**
  * @brief
  * @retval
  */
void 			BSP_I2C_MainLoop( void)
{
	BSP_SHT40_MainLoop();
	BSP_STTS22_MainLoop();
}

/**
  * @brief
  * @retval
  */
bool			BSP_I2C_Cmd(I2C_HandleTypeDef *handle, tBSP_PER_DataCmd *cmd, tBSP_PER_DataResp *resp)
{
	if( BSP_RespCodes_Assert_BSP((handle == NULL), BSP_ERROR_HANDLE_ERR))				return false;
	if( BSP_RespCodes_Assert_BSP((cmd == NULL), BSP_ERROR_PARAM_NULL))					return false;
	if( BSP_RespCodes_Assert_BSP((resp == NULL), BSP_ERROR_PARAM_NULL))					return false;

	Main_Handle	= handle;
	Main_cmd	= *cmd;
	Main_resp	= *resp;

	cmd->handle = handle;

	switch( cmd->Target)
	{
	case	eBSP_PER_TARGET_SHT40A:
		BSP_SHT40_Cmd( cmd);
		break;

	case	eBSP_PER_TARGET_STTS22:
		BSP_STTS22_Cmd( cmd);
		break;

	default:
		break;
	}

    return true;
}

/* USER CODE END 4 */

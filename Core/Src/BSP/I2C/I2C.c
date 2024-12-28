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
#include ".\SENS\LPS22D.h"
#include ".\SENS\LIS2MDL.h"
#include ".\SENS\LSM6DSV.h"
#include ".\SENS\LSM6DSO.h"
#include ".\SENS\LIS2DUX.h"
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

// Bus handling
static	tBSP_PER_Target		Main_ActiveDevice	= eBSP_PER_TARGET_VOID;
static	uint16_t			Main_TO_Value		= 0;
static	uint32_t			Main_TO_Target		= 0;
static	tBSP_I2C_TxRx		Main_BSP_I2C_TxRx	= {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void			BSP_I2C_Cb_GetData_SHT40( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_STTS22( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_LPS22D( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_LIS2MDL( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_LSM6DSV( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_LSM6DSO( tBSP_PER_DataResp *Data);
static void			BSP_I2C_Cb_GetData_LIS2DUX( tBSP_PER_DataResp *Data);

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
	BSP_LPS22D_Init( handle, BSP_I2C_Cb_GetData_LPS22D);
	BSP_LIS2MDL_Init( handle, BSP_I2C_Cb_GetData_LIS2MDL);
	BSP_LSM6DSV_Init( handle, BSP_I2C_Cb_GetData_LSM6DSV);
	BSP_LSM6DSO_Init( handle, BSP_I2C_Cb_GetData_LSM6DSO);
	BSP_LIS2DUX_Init( handle, BSP_I2C_Cb_GetData_LIS2DUX);
	return true;
}

/**
  * @brief
  * @retval
  */
void 			BSP_I2C_MainLoop( void)
{
	#define	Time		100
	static	uint32_t	Target	= 0;

	switch(Main_ActiveDevice)
	{
	case	eBSP_PER_TARGET_SHT40A:		BSP_SHT40_MainLoop();	break;
	case	eBSP_PER_TARGET_STTS22:		BSP_STTS22_MainLoop();	break;
	case	eBSP_PER_TARGET_LPS22D:		BSP_LPS22D_MainLoop();	break;
	case	eBSP_PER_TARGET_LIS2MDL:	BSP_LIS2MDL_MainLoop();	break;
	case	eBSP_PER_TARGET_LSM6DSV:	BSP_LSM6DSV_MainLoop();	break;
	case	eBSP_PER_TARGET_LSM6DSO:	BSP_LSM6DSO_MainLoop();	break;
	case	eBSP_PER_TARGET_LIS2DUX:	BSP_LIS2DUX_MainLoop();	break;
	default:
		BSP_SHT40_MainLoop();
		BSP_STTS22_MainLoop();
		BSP_LPS22D_MainLoop();
		BSP_LIS2MDL_MainLoop();
		BSP_LSM6DSV_MainLoop();
		BSP_LSM6DSO_MainLoop();
		BSP_LIS2DUX_MainLoop();
		break;
	}

	if( (Main_TO_Value > 0) && (HAL_GetTick() >= Main_TO_Target)) // TIMEOUT
	{
		Main_ActiveDevice	= 0;
		Main_TO_Value		= 0;
		Main_TO_Target		= 0;
		Main_BSP_I2C_TxRx.Cb_RxDone(false);
	}

	//	DEBUG
	if( Main_ActiveDevice > eBSP_PER_TARGET_VOID)
	{
		Target = HAL_GetTick() + Time;
		return;
	}

	if( HAL_GetTick() > Target)
	{
		Target = HAL_GetTick() + Time;

		Main_cmd.Target 	= eBSP_PER_TARGET_SHT40A;
		Main_cmd.Function	= eBSP_PER_FUNC_TEMP_RH;
		Main_cmd.Precision	= eBSP_PER_PRCSN_HIGH;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);

		Main_cmd.Target 	= eBSP_PER_TARGET_STTS22;
		Main_cmd.Function	= eBSP_PER_FUNC_TEMP;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);

		Main_cmd.Target 	= eBSP_PER_TARGET_LPS22D;
		Main_cmd.Function	= eBSP_PER_FUNC_GET_SN;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);

		Main_cmd.Target 	= eBSP_PER_TARGET_LIS2MDL;
		Main_cmd.Function	= eBSP_PER_FUNC_GET_SN;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);

		Main_cmd.Target 	= eBSP_PER_TARGET_LSM6DSV;
		Main_cmd.Function	= eBSP_PER_FUNC_GET_SN;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);

		Main_cmd.Target 	= eBSP_PER_TARGET_LSM6DSO;
		Main_cmd.Function	= eBSP_PER_FUNC_GET_SN;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);

		Main_cmd.Target 	= eBSP_PER_TARGET_LIS2DUX;
		Main_cmd.Function	= eBSP_PER_FUNC_GET_SN;
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);
	}
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

	case	eBSP_PER_TARGET_LPS22D:
		BSP_LPS22D_Cmd( cmd);
		break;

	case	eBSP_PER_TARGET_LIS2MDL:
		BSP_LIS2MDL_Cmd( cmd);
		break;

	case	eBSP_PER_TARGET_LSM6DSV:
		BSP_LSM6DSV_Cmd( cmd);
		break;

	case	eBSP_PER_TARGET_LSM6DSO:
		BSP_LSM6DSO_Cmd( cmd);
		break;

	case	eBSP_PER_TARGET_LIS2DUX:
		BSP_LIS2DUX_Cmd( cmd);
		break;

	default:
		break;
	}

    return true;
}

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
bool			BSP_I2C_IsBusy( void)
{
	return(Main_ActiveDevice != eBSP_PER_TARGET_VOID);
}

/**
  * @brief
  * @retval
  */
bool			BSP_I2C_Transmit_IT(tBSP_I2C_TxRx*	BSP_I2C_TxRx)
{
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx == NULL), BSP_ERROR_PARAM_NULL))			return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->handle == NULL), BSP_ERROR_HANDLE_ERR))	return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->Address == 0), BSP_ERROR_PARAM_ZERO))	return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->pData == NULL), BSP_ERROR_PARAM_NULL))	return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->Size == 0), BSP_ERROR_PARAM_ZERO))		return false;

	Main_BSP_I2C_TxRx	= *BSP_I2C_TxRx;

	HAL_StatusTypeDef	HAL_result = HAL_I2C_Master_Transmit_IT(BSP_I2C_TxRx->handle, BSP_I2C_TxRx->Address, BSP_I2C_TxRx->pData, BSP_I2C_TxRx->Size);
	if( BSP_RespCodes_Assert_HAL((HAL_result != HAL_OK), eBSP_RESP_CODE_HAL_ERR, HAL_result, BSP_I2C_TxRx->handle))	return false;

	Main_ActiveDevice	= BSP_I2C_TxRx->Device;
	Main_TO_Value		= BSP_I2C_TxRx->Timeout;
	Main_TO_Target		= HAL_GetTick() + BSP_I2C_TxRx->Timeout;

	return true;
}

/**
  * @brief
  * @retval
  */
bool			BSP_I2C_Receive_IT(tBSP_I2C_TxRx*	BSP_I2C_TxRx)
{
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx == NULL), BSP_ERROR_PARAM_NULL))			return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->handle == NULL), BSP_ERROR_HANDLE_ERR))	return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->Address == 0), BSP_ERROR_PARAM_ZERO))	return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->pData == NULL), BSP_ERROR_PARAM_NULL))	return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->Size == 0), BSP_ERROR_PARAM_ZERO))		return false;
	if( BSP_RespCodes_Assert_BSP((BSP_I2C_TxRx->Timeout == 0), BSP_ERROR_PARAM_ZERO))	return false;

	Main_BSP_I2C_TxRx	= *BSP_I2C_TxRx;

	// stretch timeout

	HAL_StatusTypeDef	HAL_result = HAL_I2C_Master_Receive_IT(BSP_I2C_TxRx->handle, BSP_I2C_TxRx->Address, BSP_I2C_TxRx->pData, BSP_I2C_TxRx->Size);
	if( HAL_result != HAL_OK)
	{
		Main_ActiveDevice	= eBSP_PER_TARGET_VOID;
	}
	if( BSP_RespCodes_Assert_HAL((HAL_result != HAL_OK), eBSP_RESP_CODE_HAL_ERR, HAL_result, BSP_I2C_TxRx->handle))	return false;

	Main_ActiveDevice	= BSP_I2C_TxRx->Device;
	Main_TO_Value		= BSP_I2C_TxRx->Timeout;
	Main_TO_Target		= HAL_GetTick() + BSP_I2C_TxRx->Timeout;

	return true;
}

/**
  * @brief
  * @retval
  */
void			HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *handle)
{
	Main_BSP_I2C_TxRx.Cb_TxDone(true);

	if( Main_BSP_I2C_TxRx.Timeout == 0) // no need to receive data
	{
		Main_ActiveDevice	= eBSP_PER_TARGET_VOID;
		Main_TO_Value		= 0;
		Main_TO_Target		= 0;
	}
}

/**
  * @brief
  * @retval
  */
void			HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle)
{
	Main_BSP_I2C_TxRx.Cb_RxDone(true);

	Main_ActiveDevice	= eBSP_PER_TARGET_VOID;
	Main_TO_Value		= 0;
	Main_TO_Target		= 0;
}

/**
  * @brief
  * @retval
  */
static void			BSP_I2C_Cb_GetData_SHT40( tBSP_PER_DataResp *Data)
{
	printf("[SHT40A] Addr:%.2X, SN:%lX, Temp:%.2f, H:%d\n",
			Data->Address,
			Data->SerialNumber,
			Data->Temperature,
			Data->Humidity_i);
}

/**
  * @brief
  * @retval
  */
static void			BSP_I2C_Cb_GetData_STTS22( tBSP_PER_DataResp *Data)
{
	printf("[STTS22] Addr:%.2X, SN:%lX, CTRL:%.2X, STAT:%.2X, Temp:%.2f, H:%d\n",
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
static void			BSP_I2C_Cb_GetData_LPS22D( tBSP_PER_DataResp *Data)
{
	printf("[LPS22D] Addr:%.2X, SN:%lX\n",
			Data->Address,
			Data->SerialNumber);
}

/**
  * @brief
  * @retval
  */
static void			BSP_I2C_Cb_GetData_LIS2MDL( tBSP_PER_DataResp *Data)
{
	printf("[LIS2MDL] Addr:%.2X, SN:%lX\n",
			Data->Address,
			Data->SerialNumber);
}

/**
  * @brief
  * @retval
  */
static void			BSP_I2C_Cb_GetData_LSM6DSV( tBSP_PER_DataResp *Data)
{
	printf("[LSM6DSV] Addr:%.2X, SN:%lX\n",
			Data->Address,
			Data->SerialNumber);
}

/**
  * @brief
  * @retval
  */
static void			BSP_I2C_Cb_GetData_LSM6DSO( tBSP_PER_DataResp *Data)
{
	printf("[LSM6DSO] Addr:%.2X, SN:%lX\n",
			Data->Address,
			Data->SerialNumber);
}

/**
  * @brief
  * @retval
  */
static void			BSP_I2C_Cb_GetData_LIS2DUX( tBSP_PER_DataResp *Data)
{
	printf("[LIS2DUX] Addr:%.2X, SN:%lX\n",
			Data->Address,
			Data->SerialNumber);
}

/* USER CODE END 4 */

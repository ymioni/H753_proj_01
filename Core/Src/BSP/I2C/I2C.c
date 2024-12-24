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
struct __PACKED
{
	uint16_t	Temperature;
	uint8_t		CRC_Temperature;
	uint16_t	Humidity;
	uint8_t		CRC_Humidity;
}Data_SHT40;

struct __PACKED
{
	uint8_t		SerialNumber;
}Data_STTS22;

static	bool				Main_Active 		= false;
static	uint16_t			Main_Time 			= 0;
static	uint32_t			Main_Time_Target	= 0;
static	uint8_t				Main_State 			= 0;
static	uint32_t			Main_Error			= 0;
static	uint16_t			Main_Timeout		= 0;
static	uint16_t			Main_Delay			= 0;
static	I2C_HandleTypeDef*	Main_Handle		= NULL;
static	tBSP_PER_DataCmd	Main_cmd = {};
static	tBSP_PER_DataResp	Main_resp = {};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	void	BSP_I2C_Error( I2C_HandleTypeDef *handle);
static	uint8_t	BSP_I2C_GetAddress( tBSP_PER_Target Target);
static	uint8_t	BSP_I2C_GetCmd( tBSP_PER_Target Target, tBSP_PER_Func Function, tBSP_PER_Precision Precision);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_I2C_Init( void)
{

}

/**
  * @brief
  * @retval
  */
void 				BSP_I2C_MainLoop( void)
{
	if( Main_Active == false)	return;

	if( HAL_GetTick() < Main_Time_Target)	return;

	switch( Main_State)
	{
	case	0:
		return;

	case	1:
		BSP_I2C_Cmd(Main_Handle, &Main_cmd, &Main_resp);
		Main_Time	=	5000;
		break;

	default:
		Main_State	=	0;
		return;
	}

	Main_Time_Target = HAL_GetTick() + Main_Time;
}

/**
  * @brief
  * @retval
  */
int				BSP_I2C_Cmd(I2C_HandleTypeDef *handle, tBSP_PER_DataCmd *cmd, tBSP_PER_DataResp *resp)
{
    uint8_t	address = 0;
	uint8_t command = 0;

    if( handle == NULL)			return -1;
    if( cmd == NULL)			return -2;
	if( resp == NULL)			return -3;

	Main_Timeout	= 1000;
	Main_Delay		= 50;
	address	=	BSP_I2C_GetAddress(cmd->Target);
	command =	BSP_I2C_GetCmd(cmd->Target, cmd->Function, cmd->Precision);

	if( address == 0)			return -4;
    if( command == 0)			return -5;

	Main_Active		= true;
	Main_Handle		= handle;
	Main_State		= 1;

    if (HAL_I2C_Master_Transmit(handle, address, &command, 1, Main_Timeout) != HAL_OK)
    {
    	BSP_I2C_Error(handle);
    	return -6;
    }

	HAL_Delay(Main_Delay);

	resp->Address		=	address;
	if( cmd->Target == eBSP_PER_TARGET_SHT40A)
	{
		if (HAL_I2C_Master_Receive(handle, address, (uint8_t *)&Data_SHT40, sizeof( Data_SHT40), Main_Timeout) != HAL_OK)
	    {
	    	BSP_I2C_Error(handle);
	    	return -7;
	    }

		BSP_Util_SwapBytes(&Data_SHT40.Temperature, sizeof( Data_SHT40.Temperature));
		BSP_Util_SwapBytes(&Data_SHT40.Humidity, sizeof( Data_SHT40.Humidity));

		resp->Temperature	=	BSP_Per_Convert( eBSP_PER_TARGET_SHT40A, eBSP_PER_FUNC_TEMP, Data_SHT40.Temperature);
		resp->Humidity_f	=	BSP_Per_Convert( eBSP_PER_TARGET_SHT40A, eBSP_PER_FUNC_RH, Data_SHT40.Humidity);
		resp->Humidity_i	= 	resp->Humidity_f;
	}

	if( cmd->Target == eBSP_PER_TARGET_STTS22)
	{
		if (HAL_I2C_Master_Receive(handle, address, (uint8_t *)&Data_STTS22, sizeof( Data_STTS22), Main_Timeout) != HAL_OK)
	    {
	    	BSP_I2C_Error(handle);
	    	return -7;
	    }

		resp->SerialNumber	=	Data_STTS22.SerialNumber;
	}

	if( cmd->Function == eBSP_PER_FUNC_TEMP_RH)
	{
		printf("Temperature: %.2fC\n", resp->Temperature);
		printf("Humidity: %d%%\n\n", resp->Humidity_i);
	}

	if( cmd->Function == eBSP_PER_FUNC_GET_SN)
	{
		printf("S/N: %.2lX\n", resp->SerialNumber);
	}

	Main_cmd	= *cmd;
	Main_resp	= *resp;

    return 1;
}

/**
  * @brief
  * @retval
  */
static	void	BSP_I2C_Error( I2C_HandleTypeDef *handle)
{
	Main_Error =  HAL_I2C_GetError(handle);
    printf("I2C Error! (%.8lX)\n", Main_Error);
}

/**
  * @brief
  * @retval
  */
static	uint8_t	BSP_I2C_GetAddress( tBSP_PER_Target Target)
{
	uint8_t	address = 0;

	switch( Target)
	{
	case	eBSP_PER_TARGET_SHT40A:
		address =	I2C1_DEVICE_ADDRESS_SHT40;
	    break;

	case	eBSP_PER_TARGET_STTS22:
		address =	I2C1_DEVICE_ADDRESS_STTS22;
	    break;

	default:
		break;
	}

	return address;
}

/**
  * @brief
  * @retval
  */
static	uint8_t	BSP_I2C_GetCmd( tBSP_PER_Target Target, tBSP_PER_Func Function, tBSP_PER_Precision Precision)
{
	uint8_t	cmd = 0;

	switch( Target)
	{
	case	eBSP_PER_TARGET_SHT40A:
	    switch( Function)
	    {
	    case	eBSP_PER_FUNC_TEMP_RH:
	    	switch( Precision)
	    	{
	    	case	eBSP_PER_PRCSN_HIGH:
	    	default:
	        	cmd = 0xFD;
	        	Main_Timeout	= 1000;
	        	Main_Delay		= 50;
	    		break;
	    	}
	    	break;

	    default:
	    	break;
	    }
		break;

	case	eBSP_PER_TARGET_STTS22:
	    switch( Function)
	    {
		case	eBSP_PER_FUNC_GET_SN:
		cmd = 0x01;
		Main_Timeout	= 1000;
		Main_Delay		= 50;
		break;

	    default:
	    	break;
	    }
		break;

	default:
		break;
	}

	return cmd;
}

/* USER CODE END 4 */

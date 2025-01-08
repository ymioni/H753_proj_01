/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : peripherals.c
  * @brief          : peripherals program body
  ******************************************************************************
  * @attention
  *
  *
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
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Peripherals.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
static float		BSP_Per_Temp2sComplement( int16_t Value);
static float		BSP_Per_XLG( int16_t Value, float factor, uint16_t divider);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief
  * @retval
  */
float				BSP_Per_Convert( tBSP_PER_Target Target, tBSP_PER_Func Function, uint32_t Value)
{
    float result = 0.0;

	switch( Target)
	{
	case	eBSP_PER_TARGET_SHT40A:
		switch( Function)
		{
		case	eBSP_PER_FUNC_TEMP	:
	        result = -45.0f + (175.0f * (Value / 65535.0f));
			break;

		case	eBSP_PER_FUNC_RH	:
	        result = -6.0f + (125.0f * (Value / 65535.0f));
	        if( result < 0)		result = 0.0;
	        if( result > 100)	result = 100.0;
			break;
		}
		break;

	case eBSP_PER_TARGET_STTS22:
		switch( Function)
		{
		case	eBSP_PER_FUNC_TEMP:
		if( Value < 0x8000)
			result = (Value / 100.0);
		else
			result = (((int)Value - 0x10000) / 100.0);
		break;
		}
		break;

	case eBSP_PER_TARGET_LIS2MDL:
		switch( Function)
		{
		case	eBSP_PER_FUNC_TEMP:
		result = BSP_Per_Temp2sComplement( Value);
		break;
		}
		break;

	case eBSP_PER_TARGET_LSM6DSV:
	case eBSP_PER_TARGET_LSM6DSO:
		switch( Function)
		{
			case	eBSP_PER_FUNC_TEMP:
			result = BSP_Per_Temp2sComplement( Value);
			break;
		}
		break;
	}

	return result;
}

/**
  * @brief Accelerometer, Gyroscope
  * @retval
  */
float				BSP_Per_Convert_XLG( tBSP_PER_Target Target, tBSP_PER_Func Function, uint32_t Value, float factor, uint16_t divider)
{
    float result = 0.0;

    if( divider == 0)
    {
    	return 0.0;
    }

	switch( Target)
	{
	case eBSP_PER_TARGET_LSM6DSV:
	case eBSP_PER_TARGET_LSM6DSO:
		switch( Function)
		{
			case	eBSP_PER_FUNC_GET_GYRO:
			result = BSP_Per_XLG( Value, factor, divider);
			break;

			case	eBSP_PER_FUNC_GET_ACCL:
			result = BSP_Per_XLG( Value, factor, divider);
			break;
		}
		break;
	}

	return result;
}

/**
  * @brief
  * @retval
  */
static float		BSP_Per_Temp2sComplement( int16_t Value)
{
	return( 25.0 + (Value / 256.0));
}

/**
  * @brief
  * @retval
  */
static float		BSP_Per_XLG( int16_t Value, float factor, uint16_t divider)
{
    if( divider == 0)
    	return 0.0;

    return( (Value * factor) / divider);
}

#ifdef __cplusplus
}
#endif


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : gpio.c
  * @brief          : gpio program body
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
#include "Gpio.h"
#include "..\RespCodes.h"
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
#define	BSP_GPIO_MAX_SETTINGS		20

static	uint8_t			GPIO_Data_idx = 0;
struct
{
	GPIO_TypeDef*		Port;
	uint16_t			Pin;
}GPIO_Data[BSP_GPIO_MAX_SETTINGS] = {0};

static	bool			Main_Active 		= false;
static	uint16_t		Main_Time 			= 0;
static	uint32_t		Main_Time_Target	= 0;
static	uint8_t			Main_State 			= 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief
  * @retval
  */
bool			BSP_GPIO_Init(GPIO_TypeDef* Port, uint16_t Pin)
{
	if( BSP_RespCodes_Assert_BSP((GPIO_Data_idx >= BSP_GPIO_MAX_SETTINGS), BSP_ERROR_PARAM_OVF))	return false;

	GPIO_Data[GPIO_Data_idx].Port	= Port;
	GPIO_Data[GPIO_Data_idx].Pin	= Pin;
	GPIO_Data_idx ++;

	Main_Active 		= true;
	Main_Time 			= 0;
	Main_Time_Target	= 0;
	Main_State 			= 0;

	return true;
}

/**
  * @brief
  * @retval
  */
void BSP_GPIO_MainLoop( void)
{
	if( Main_Active == false)	return;

	if( HAL_GetTick() < Main_Time_Target)	return;

	switch( Main_State)
	{
	case	0:
		return;

	case	1:
		Main_Time	=	500;
		Main_State ++;
		break;

	case	2:
		Main_State	= 1;
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
uint8_t			idx;
GPIO_TypeDef*	Port = NULL;


	for( idx = 0; idx < BSP_GPIO_MAX_SETTINGS; idx ++)
	{
		if( GPIO_Data[idx].Pin == GPIO_Pin)
		{
			Port = GPIO_Data[idx].Port;
			break;
		}
	}

	if( GPIO_Pin == GPIO_PIN_13) // PORTC, PIN 13
	{
		if( Port != NULL)
		{
			if( HAL_GPIO_ReadPin(Port, GPIO_Pin) == GPIO_PIN_SET)
				printf("Blue button pressed\n");
			else
				printf("Blue button released\n");
		}
		else
			printf("Error! Invalid Port\n");
	}
}



#ifdef __cplusplus
}
#endif


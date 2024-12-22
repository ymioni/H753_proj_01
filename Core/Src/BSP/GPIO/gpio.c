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
struct
{
	GPIO_TypeDef *		Port;
	uint16_t			Pin;
	tBSP_GPIO_Pattern	Pattern;
	uint16_t			Timer;
}GPIO_Data[eBSP_GPIO_MAX_VALUE];
//}GPIO_Data[eBSP_GPIO_MAX_VALUE]	=	{	{ .Port = GPIOB, .Pin = GPIO_PIN_14, .Pattern	= eBSP_GPIO_PATTERN_OFF, .Timer	= 0},
//										{ .Port = GPIOE, .Pin = GPIO_PIN_1,  .Pattern	= eBSP_GPIO_PATTERN_OFF, .Timer	= 0},
//									};

static	bool		Main_Active 		= false;
static	uint16_t	Main_Time 			= 0;
static	uint32_t	Main_Time_Target	= 0;
static	uint8_t		Main_State 			= 0;

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
void BSP_GPIO_Init(void)
{
	Main_Active 		= false;
	Main_Time 			= 0;
	Main_Time_Target	= 0;
	Main_State 			= 0;
}

/**
  * @brief
  * @retval
  */
void BSP_GPIO_MainStart(void)
{
	Main_Active 		= true;
	Main_Time 			= 0;
	Main_Time_Target	= 0;
	Main_State 			= 0;
}

/**
  * @brief
  * @retval
  */
void BSP_GPIO_MainStop(void)
{
	Main_Active 		= false;
	Main_Time 			= 0;
	Main_Time_Target	= 0;
	Main_State 			= 0;
}

/**
  * @brief
  * @retval
  */
void BSP_GPIO_MainLoop(void)
{
	if( Main_Active == false)	return;

	if( HAL_GetTick() < Main_Time_Target)	return;

	switch( Main_State)
	{
	case	0:
		Main_Time	=	100;
		break;

	case	1:
		Main_Time	=	500;
		break;

	default:
		Main_State	=	0;
		return;
	}

//	for( tBSP_GPIO Gpio = 0; Gpio < eBSP_GPIO_MAX_VALUE; Gpio ++)
//	{
//		HAL_GPIO_TogglePin(GPIO_Data[Gpio].Port, GPIO_Data[Gpio].Pin);
//	}

	Main_State	++;
	Main_Time_Target = HAL_GetTick() + Main_Time;
}

/**
  * @brief
  * @retval
  */
void BSP_GPIO_Start(tBSP_GPIO Gpio, tBSP_GPIO_Pattern Pattern, uint16_t Time)
{
	if( Gpio >= eBSP_GPIO_MAX_VALUE)	return;

	GPIO_Data[Gpio].Pattern	=	eBSP_GPIO_PATTERN_ON;
	GPIO_Data[Gpio].Timer		=	Time;

	HAL_GPIO_WritePin(GPIO_Data[Gpio].Port, GPIO_Data[Gpio].Pin, GPIO_PIN_SET);
}

/**
  * @brief
  * @retval
  */
void BSP_GPIO_Stop(tBSP_GPIO Gpio)
{
	if( Gpio >= eBSP_GPIO_MAX_VALUE)	return;

	GPIO_Data[Gpio].Pattern	=	eBSP_GPIO_PATTERN_OFF;
	GPIO_Data[Gpio].Timer		=	0;

	HAL_GPIO_WritePin(GPIO_Data[Gpio].Port, GPIO_Data[Gpio].Pin, GPIO_PIN_RESET);
}

/**
  * @brief
  * @retval
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if( GPIO_Pin == GPIO_PIN_13) // PORTC, PIN 13
	{
		printf("Blue button pressed\n");
	}
}


#ifdef __cplusplus
}
#endif


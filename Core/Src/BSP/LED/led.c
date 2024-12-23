/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : led.c
  * @brief          : led program body
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
#include "Led.h"
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
	tBSP_LED_Pattern	Pattern;
	uint16_t			Timer;
}LED_Data[eBSP_LED_MAX_VALUE]	=	{	{ .Port = GPIOB, .Pin = GPIO_PIN_14, .Pattern	= eBSP_LED_PATTERN_OFF, .Timer	= 0},
										{ .Port = GPIOE, .Pin = GPIO_PIN_1,  .Pattern	= eBSP_LED_PATTERN_OFF, .Timer	= 0},
										{ .Port = GPIOB, .Pin = GPIO_PIN_0,  .Pattern	= eBSP_LED_PATTERN_OFF, .Timer	= 0},
									};

static	bool		Main_Active 		= false;
static	uint16_t	Main_Time 			= 0;
static	uint32_t	Main_Time_Target	= 0;
static	uint8_t		Main_State 			= 0;
static	uint8_t		Main_Idle_Idx		= 0;

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
void BSP_LED_Init(void)
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
void BSP_LED_MainStart(void)
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
void BSP_LED_MainStop(void)
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
void BSP_LED_MainLoop(void)
{
	if( Main_Active == false)	return;

	if( HAL_GetTick() < Main_Time_Target)	return;

	switch( Main_State)
	{
	case	0:
		Main_Time	=	250;
		break;

	default:
		Main_State	=	0;
		return;
	}

	if( Main_Idle_Idx >= eBSP_LED_MAX_VALUE) Main_Idle_Idx = 0;
	HAL_GPIO_TogglePin(LED_Data[Main_Idle_Idx].Port, LED_Data[Main_Idle_Idx].Pin);
	Main_Idle_Idx ++;
	if( Main_Idle_Idx >= eBSP_LED_MAX_VALUE) Main_Idle_Idx = 0;
	HAL_GPIO_TogglePin(LED_Data[Main_Idle_Idx].Port, LED_Data[Main_Idle_Idx].Pin);

	Main_Time_Target = HAL_GetTick() + Main_Time;
}

/**
  * @brief
  * @retval
  */
void BSP_LED_Start(tBSP_LED Led, tBSP_LED_Pattern Pattern, uint16_t Time)
{
	if( Pattern == eBSP_LED_PATTERN_OFF)
	{
		BSP_LED_Stop( Led);
		return;
	}

	if( Led >= eBSP_LED_MAX_VALUE)	return;

	LED_Data[Led].Pattern	=	Pattern;
	LED_Data[Led].Timer		=	Time;

	HAL_GPIO_WritePin(LED_Data[Led].Port, LED_Data[Led].Pin, GPIO_PIN_SET);
}

/**
  * @brief
  * @retval
  */
void BSP_LED_Stop(tBSP_LED Led)
{
	if( Led >= eBSP_LED_MAX_VALUE)	return;

	LED_Data[Led].Pattern	=	eBSP_LED_PATTERN_OFF;
	LED_Data[Led].Timer		=	0;

	HAL_GPIO_WritePin(LED_Data[Led].Port, LED_Data[Led].Pin, GPIO_PIN_RESET);
}


#ifdef __cplusplus
}
#endif


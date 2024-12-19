/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : led.h
  * @brief          : Header for led.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_LED_H
#define __BSP_LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum
{
	eBSP_LED_1_LED,						//	0
	eBSP_LED_2_ORANGE,					//	1
	/***** DON'T CROSS THIS LINE *****/
	eBSP_LED_MAX_VALUE					//	2
}tBSP_LED;

typedef enum
{
	eBSP_LED_PATTERN_OFF,				//	0
	eBSP_LED_PATTERN_ON,				//	1
	eBSP_LED_PATTERN_BLINK_VERY_FAST,	//	2
	eBSP_LED_PATTERN_BLINK_FAST,		//	3
	eBSP_LED_PATTERN_BLINK_SLOW,		//	4
	eBSP_LED_PATTERN_BLINK_VERY_SLOW,	//	5
	eBSP_LED_PATTERN_BLINK_CUSTOM,		//	6 (TBD)
}tBSP_LED_Pattern;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void BSP_LED_Init(void);
void BSP_LED_MainStart(void);
void BSP_LED_MainStop(void);
void BSP_LED_MainLoop(void);
void BSP_LED_Start(tBSP_LED Led, tBSP_LED_Pattern Pattern, uint16_t Time);
void BSP_LED_Stop(tBSP_LED Led);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LED_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : gpio.h
  * @brief          : Header for gpio.c file.
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
#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

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
	eBSP_GPIO_RESULT_HAL_OK				=	HAL_OK		,
	eBSP_GPIO_RESULT_HAL_ERROR			=	HAL_ERROR	,
	eBSP_GPIO_RESULT_HAL_BUSY			=	HAL_BUSY	,
	eBSP_GPIO_RESULT_HAL_TIMEOUT		=	HAL_TIMEOUT	,
	eBSP_GPIO_RESULT_INVALID_PORT		=	-1,
	eBSP_GPIO_RESULT_NULL_HANDLE		=	-2,
	eBSP_GPIO_RESULT_NULL_CB_FUNC		=	-3,
	eBSP_GPIO_RESULT_NULL_BUFFER		=	-4,
	eBSP_GPIO_RESULT_ZERO_LEN			=	-5,
	eBSP_GPIO_RESULT_BUSY_TX			=	-6,
	eBSP_GPIO_RESULT_NA_PORT			=	-7,
	eBSP_GPIO_RESULT_SETTING_OVERFLOW	=	-8,
	/***** DON'T CROSS THIS LINE *****/
	eBSP_GPIO_RESULT_MAX_VALUE				//
}tBSP_GPIO_RESULT;

typedef	void (*Cb_Gpio)(void);
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
tBSP_GPIO_RESULT	BSP_GPIO_Init( GPIO_TypeDef* Port, uint16_t Pin);
void 				BSP_GPIO_MainLoop( void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H */

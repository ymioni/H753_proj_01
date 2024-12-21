/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usart.h
  * @brief          : Header for usart.c file.
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
#ifndef __BSP_USART_H
#define __BSP_USART_H

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
	eBSP_USART_PORT_NONE,					//	0
	eBSP_USART_PORT_1,						//	1
	eBSP_USART_PORT_2,						//	2
	eBSP_USART_PORT_3,						//	3
	eBSP_USART_PORT_4,						//	4
	eBSP_USART_PORT_5,						//	5
	eBSP_USART_PORT_6,						//	6
	eBSP_USART_PORT_7,						//	7
	eBSP_USART_PORT_8,						//	8
	/***** DON'T CROSS THIS LINE *****/
	eBSP_USART_MAX_VALUE					//	9
}tBSP_USART_PORT;

typedef	void (*Cb_UsartTxCplt)(void);
typedef	void (*Cb_UsartRxCplt)(uint8_t Byte);
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void BSP_USART_Init( tBSP_USART_PORT Port, UART_HandleTypeDef *huart);
void BSP_USART_MainLoop( void);
int  BSP_USART_Send( tBSP_USART_PORT Port, Cb_UsartTxCplt CbFunc, void *buf, uint16_t len);
int	 BSP_USART_Receive(tBSP_USART_PORT Port, Cb_UsartRxCplt CbFunc);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_USART_H */

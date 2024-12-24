/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : RespCodes.h
  * @brief          : Header for RespCodes.c file.
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
#ifndef __BSP_RESP_H
#define __BSP_RESP_H

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
typedef	enum
{
	eBSP_RESP_CODE_BSP_OK		= 0,
	eBSP_RESP_CODE_BSP_ERR		= -1,
	eBSP_RESP_CODE_HAL_ERR		= -2,
	eBSP_RESP_CODE_DMA_ERR		= -3,
	eBSP_RESP_CODE_FLASH_ERR	= -4,
	eBSP_RESP_CODE_I2C_ERR		= -5,
	eBSP_RESP_CODE_MDMA_ERR		= -6,
	eBSP_RESP_CODE_SPI_ERR		= -7,
	eBSP_RESP_CODE_UART_ERR		= -8,
	/***** DON'T CROSS THIS LINE *****/
	eBSP_RESP_CODE_MAX_VALUE
}tBSP_RESP_CODES;

typedef	struct
{
	tBSP_RESP_CODES		BSP_ErrCode;
	uint32_t			ErrCode;
}tBSP_RESP_INFO;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */
void 				BSP_RespCodes_Init(void);
tBSP_RESP_INFO*		BSP_RespCodes_GetErr(void);
bool				BSP_RespCodes_Assert_BSP(bool Condition, uint32_t RespCode);
bool				BSP_RespCodes_Assert_HAL(bool Condition, tBSP_RESP_CODES RespCode, HAL_StatusTypeDef HAL_Status, void	*handle);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */
#define  BSP_ERROR_NONE             	(0x00000000U)    /*!< No error                */
#define  BSP_ERROR_HANDLE_ERR			(0x00000001U)    /*!< */
#define  BSP_ERROR_PARAM_NULL			(0x00000002U)    /*!< */
#define  BSP_ERROR_PARAM_ZERO			(0x00000004U)    /*!< */
#define  BSP_ERROR_PARAM_OVF			(0x00000008U)    /*!< */
#define  BSP_ERROR_PARAM_NA				(0x00000010U)    /*!< */
#define	 BSP_ERROR_BUSY					(0x00000020U)    /*!< */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __BSP_RESP_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : RespCodes.c
  * @brief          : RespCodes program body
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
#include "RespCodes.h"
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
static	tBSP_RESP_INFO	RespCodes = {	.BSP_ErrCode = 0,
										.ErrCode = 0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
static uint32_t		BSP_RespCodes_GetError(tBSP_RESP_CODES RespCode, void	*handle);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief
  * @retval
  */
void 				BSP_RespCodes_Init(void)
{
	RespCodes.BSP_ErrCode = 0;
	RespCodes.ErrCode = 0;
}

/**
  * @brief
  * @retval
  */
tBSP_RESP_INFO*		BSP_RespCodes_GetErr(void)
{
	return &RespCodes;
}

/**
  * @brief
  * @retval
  */
bool				BSP_RespCodes_Assert_BSP(bool Condition, uint32_t RespCode)
{
	if( Condition == true)
	{
		RespCodes.BSP_ErrCode = eBSP_RESP_CODE_BSP_ERR;
		RespCodes.ErrCode = RespCode;
		return true;
	}

	return false;
}

/**
  * @brief
  * @retval
  */
bool				BSP_RespCodes_Assert_HAL(bool Condition, tBSP_RESP_CODES RespCode, HAL_StatusTypeDef HAL_Status, void	*handle)
{
	if( Condition == true)
	{
		RespCodes.BSP_ErrCode = RespCode;
		if( RespCode == eBSP_RESP_CODE_HAL_ERR)	 // HAL_ERROR, HAL_BUSY, HAL_TIMEOUT
		{
			RespCodes.ErrCode = HAL_Status;
		}
		else
		{
			if( handle == NULL)
			{
				RespCodes.BSP_ErrCode = eBSP_RESP_CODE_BSP_ERR;
				RespCodes.ErrCode = BSP_ERROR_HANDLE_ERR;
			}
			else
			{
				RespCodes.ErrCode = BSP_RespCodes_GetError(RespCode, handle);
			}
		}

		return true;
	}

	return false;
}

/**
  * @brief
  * @retval
  */
static uint32_t		BSP_RespCodes_GetError(tBSP_RESP_CODES RespCode, void	*handle)
{
	switch(RespCode)
	{
	case	eBSP_RESP_CODE_DMA_ERR		:return HAL_DMA_GetError((DMA_HandleTypeDef *) handle);
	case	eBSP_RESP_CODE_FLASH_ERR	:return HAL_FLASH_GetError();
	case	eBSP_RESP_CODE_I2C_ERR		:return HAL_I2C_GetError((I2C_HandleTypeDef *) handle);
	case	eBSP_RESP_CODE_MDMA_ERR		:return HAL_MDMA_GetError((MDMA_HandleTypeDef *) handle);
	case	eBSP_RESP_CODE_SPI_ERR		:return (int32_t)-1;	// TBD
	case	eBSP_RESP_CODE_UART_ERR		:return HAL_UART_GetError((UART_HandleTypeDef *) handle);
	default:	return 0;
	}
}

#ifdef __cplusplus
}
#endif


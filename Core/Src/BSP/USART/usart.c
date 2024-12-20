/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usart.c
  * @brief          : usart program body
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
#include "Usart.h"
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
	UART_HandleTypeDef*	Handle;
	USART_TypeDef*		Port;
	Cb_UsartTxCplt		CbFunc_TxCplt;
}USART_Data[eBSP_USART_MAX_VALUE] = {0};

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
void BSP_USART_Init(tBSP_USART_PORT Port, UART_HandleTypeDef *huart)
{
	if( Port >= eBSP_USART_MAX_VALUE)	return;
	if( huart == NULL)	return;

	USART_Data[Port].Handle = huart;
	switch( Port)
	{
		case	eBSP_USART_PORT_1:	USART_Data[Port].Port = USART1;	break;
		case	eBSP_USART_PORT_2:	USART_Data[Port].Port = USART2;	break;
		case	eBSP_USART_PORT_3:	USART_Data[Port].Port = USART3;	break;
		case	eBSP_USART_PORT_4:	/*USART_Data[Port].Port = USART4;*/	break;
		case	eBSP_USART_PORT_5:	/*USART_Data[Port].Port = USART5;*/	break;
		case	eBSP_USART_PORT_6:	USART_Data[Port].Port = USART6;	break;
		case	eBSP_USART_PORT_7:	/*USART_Data[Port].Port = USART7;*/	break;
		case	eBSP_USART_PORT_8:	/*USART_Data[Port].Port = USART8;*/	break;
		default:	return;
	}
}

/**
  * @brief
  * @retval
  */
int BSP_USART_Send(tBSP_USART_PORT Port, Cb_UsartTxCplt CbFunc, void *buf, uint16_t len)
{
	if( Port >= eBSP_USART_MAX_VALUE)		return -1;
	if( CbFunc == NULL)						return -2;
	if( buf == NULL)						return -3;
	if( len == 0)							return -4;
	if( USART_Data[Port].Handle == NULL)	return -5;

	USART_Data[Port].CbFunc_TxCplt = CbFunc;
	HAL_StatusTypeDef	result =	HAL_UART_Transmit_IT(USART_Data[Port].Handle, buf, len);
    return( result);
}

/**
  * @brief
  * @retval
  */
void BSP_USART_Receive(void)
{
}

// Callback for TX complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_Data[eBSP_USART_PORT_3].Port)
    {
    	if( USART_Data[eBSP_USART_PORT_3].CbFunc_TxCplt)
    		USART_Data[eBSP_USART_PORT_3].CbFunc_TxCplt();
    }
}

// Callback for Error
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_Data[eBSP_USART_PORT_3].Port)
    {
    }
}

#ifdef __cplusplus
}
#endif


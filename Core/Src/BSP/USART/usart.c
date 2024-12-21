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
	Cb_UsartRxCplt		CbFunc_RxCplt;
	bool				TxBusy;
}USART_Data[eBSP_USART_MAX_VALUE] = {0};

static	bool			Main_Active 		= false;
static	uint16_t		Main_Time 			= 0;
static	uint32_t		Main_Time_Target	= 0;
static	uint8_t			Main_State 			= 0;
static	uint8_t			Main_Buf[32];
static	uint8_t			Main_BufLen			= 0;
static	tBSP_USART_PORT	Main_Port			= eBSP_USART_PORT_NONE;
static  uint8_t			Main_RxByte			= 0;
static  uint32_t		Main_RxCnt			= 0;

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
tBSP_USART_RESULT BSP_USART_Init(tBSP_USART_PORT Port, UART_HandleTypeDef *huart)
{
	if( Port >= eBSP_USART_MAX_VALUE)		return eBSP_USART_RESULT_INVALID_PORT;
	if( huart == NULL)						return eBSP_USART_RESULT_NULL_HANDLE;

	switch( Port)
	{
		case	eBSP_USART_PORT_1:	USART_Data[Port].Port = USART1;	break;
		case	eBSP_USART_PORT_2:	USART_Data[Port].Port = USART2;	break;
		case	eBSP_USART_PORT_3:	USART_Data[Port].Port = USART3;	break;
		case	eBSP_USART_PORT_4:									return eBSP_USART_RESULT_NA_PORT;
		case	eBSP_USART_PORT_5:									return eBSP_USART_RESULT_NA_PORT;
		case	eBSP_USART_PORT_6:	USART_Data[Port].Port = USART6;	break;
		case	eBSP_USART_PORT_7:									return eBSP_USART_RESULT_NA_PORT;
		case	eBSP_USART_PORT_8:									return eBSP_USART_RESULT_NA_PORT;
		default:													return eBSP_USART_RESULT_INVALID_PORT;
	}
	USART_Data[Port].Handle = huart;

	Main_Active 		= true;
	Main_Time 			= 0;
	Main_Time_Target	= 0;
	Main_State 			= 0;
	Main_Port			= eBSP_USART_PORT_NONE;

	return eBSP_USART_RESULT_HAL_OK;
}

/**
  * @brief
  * @retval
  */
void BSP_USART_MainLoop( void)
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
		HAL_UART_Transmit_IT(USART_Data[Main_Port].Handle, Main_Buf, Main_BufLen);
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
tBSP_USART_RESULT BSP_USART_Send(tBSP_USART_PORT Port, Cb_UsartTxCplt CbFunc, void *buf, uint16_t len)
{
	if( Port >= eBSP_USART_MAX_VALUE)		return eBSP_USART_RESULT_INVALID_PORT;
	if( USART_Data[Port].Handle == NULL)	return eBSP_USART_RESULT_NULL_HANDLE;
	if( CbFunc == NULL)						return eBSP_USART_RESULT_NULL_CB_FUNC;
	if( buf == NULL)						return eBSP_USART_RESULT_NULL_BUFFER;
	if( len == 0)							return eBSP_USART_RESULT_ZERO_LEN;
	if( USART_Data[Port].TxBusy == true)	return eBSP_USART_RESULT_BUSY_TX;

	memset( Main_Buf, 0, sizeof( Main_Buf));
	memcpy( Main_Buf, buf, len);
	Main_BufLen	=	len;
	Main_State	=	0;
	Main_Time	=	0;
	Main_Port	=	Port;

	USART_Data[Port].TxBusy	=	true;
	USART_Data[Port].CbFunc_TxCplt = CbFunc;
	HAL_StatusTypeDef	result =	HAL_UART_Transmit_IT(USART_Data[Port].Handle, buf, len);
    return( result);
}

/**
  * @brief
  * @retval
  */
tBSP_USART_RESULT BSP_USART_Receive(tBSP_USART_PORT Port, Cb_UsartRxCplt CbFunc)
{
	if( Port >= eBSP_USART_MAX_VALUE)		return eBSP_USART_RESULT_INVALID_PORT;
	if( USART_Data[Port].Handle == NULL)	return eBSP_USART_RESULT_NULL_HANDLE;
	if( CbFunc == NULL)						return eBSP_USART_RESULT_NULL_CB_FUNC;

	USART_Data[Port].CbFunc_RxCplt = CbFunc;
	HAL_StatusTypeDef	result =	HAL_UART_Receive_IT(USART_Data[Port].Handle, &Main_RxByte, 1);
    return( result);
}

// Callback for TX complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	tBSP_USART_PORT	Port;

    if (huart->Instance == USART_Data[eBSP_USART_PORT_3].Port)
    {
    	Port = eBSP_USART_PORT_3;
    	if( USART_Data[Port].CbFunc_TxCplt)
    		USART_Data[Port].CbFunc_TxCplt();
    	USART_Data[Port].TxBusy	=	false;
    }
}

// Callback for RX complete
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    Main_RxCnt ++;

    if (huart->Instance == USART_Data[eBSP_USART_PORT_3].Port)
    {
    	if( USART_Data[eBSP_USART_PORT_3].CbFunc_RxCplt)
    	{
    		USART_Data[eBSP_USART_PORT_3].CbFunc_RxCplt(Main_RxByte);
    		HAL_UART_Receive_IT(USART_Data[eBSP_USART_PORT_3].Handle, &Main_RxByte, 1);
    	}
    }
}

// Callback for Error
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    __HAL_UART_FLUSH_DRREGISTER(huart);

    if (huart->Instance == USART_Data[eBSP_USART_PORT_3].Port)
    {
		HAL_UART_Receive_IT(USART_Data[eBSP_USART_PORT_3].Handle, &Main_RxByte, 1);
    }
}

#ifdef __cplusplus
}
#endif


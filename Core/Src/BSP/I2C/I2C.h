/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : I2C.h
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"

/* USER CODE END Includes */
#include "..\PER\Peripherals.h"

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef	void	(*BSP_I2C_Cb_TxDone)(bool result);	//	false in case of failure
typedef	void	(*BSP_I2C_Cb_RxDone)(bool result);	//	false in case of failure

typedef	struct
{
	I2C_HandleTypeDef*		handle;
	uint16_t 				Address;
	tBSP_PER_Target			Device;
	uint8_t*				pData;
	uint16_t 				Size;
	uint16_t				Timeout;
	BSP_I2C_Cb_TxDone		Cb_TxDone;
	BSP_I2C_Cb_RxDone		Cb_RxDone;
}tBSP_I2C_TxRx;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
bool			BSP_I2C_Init( I2C_HandleTypeDef *handle);
void 			BSP_I2C_MainLoop( void);
bool			BSP_I2C_Cmd(I2C_HandleTypeDef *handle, tBSP_PER_DataCmd *cmd, tBSP_PER_DataResp *resp);

bool			BSP_I2C_Transmit_IT(tBSP_I2C_TxRx*	BSP_I2C_TxRx);
bool			BSP_I2C_Receive_IT(tBSP_I2C_TxRx*	BSP_I2C_TxRx);
void			HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *handle);
void			HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *handle);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


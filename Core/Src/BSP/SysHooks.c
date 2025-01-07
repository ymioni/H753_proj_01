
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : SysHooks.c
  * @brief          : SysHooks program body
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
#include "cmsis_os.h"
#include "SysHooks.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief
  * @retval
  */
void 				vApplicationIdleHook(void)
{
	printf("ERR! vApplicationIdleHook\n");
	ulTaskNotifyTake(pdTRUE, osWaitForever);
}

/**
  * @brief
  * @retval
  */
void 				vApplicationTickHook(void)
{
	printf("ERR! vApplicationTickHook\n");
	ulTaskNotifyTake(pdTRUE, osWaitForever);
}

/**
  * @brief
  * @retval
  */
void 				vApplicationMallocFailedHook(void)
{
	printf("ERR! vApplicationMallocFailedHook\n");
	ulTaskNotifyTake(pdTRUE, osWaitForever);
}

/**
  * @brief
  * @retval
  */
void 				vApplicationDaemonTaskStartupHook(void)
{
	printf("ERR! vApplicationDaemonTaskStartupHook\n");
	ulTaskNotifyTake(pdTRUE, osWaitForever);
}

/**
  * @brief
  * @retval
  */
void 				vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
	printf("ERR! vApplicationStackOverflowHook (%s)\n", pcTaskName);
	ulTaskNotifyTake(pdTRUE, osWaitForever);
}

/* USER CODE END 0 */

#ifdef __cplusplus
}
#endif


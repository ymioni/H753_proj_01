/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : I2C.c
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include "..\PER\Peripherals.h"
#include "..\Util\Util.h"
#include "..\RespCodes.h"
#include "task_Sensors.h"
#include "SHT40.h"
#include "STTS22.h"
#include "LPS22D.h"
#include "LIS2MDL.h"
#include "LSM6DSV.h"
#include "LSM6DSO.h"
#include "LIS2DUX.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
	tBSP_PER_Target		target;
	tBSP_PER_Func		func;
	int16_t				arg1;
	int16_t				arg2;
	int16_t				arg3;
} tQ_Sensor_Cmd;

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
	I2C_HandleTypeDef*		hI2C;
} Main_Info	= {	.hI2C	= NULL};

struct
{
	void*					handle;
	bool					err_Q_Lvl;
}Main_Targets[eBSP_PER_MAX_VALUE_TARGET] = {0};

static	osMessageQueueId_t 			Main_Q;
static	const osMessageQueueAttr_t	Q_attributes	= {	.name = "Q_Sensors"};
static	tQ_Sensor_Cmd				Main_Q_Cmd;

static	osTimerId_t					Main_Timer_idle;
static	bool						Main_Pause = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static	void		BSP_Sensors_InitSensors( void);
static	bool		BSP_Sensors_TxCmd2Sensor( tQ_Sensor_Cmd	*cmd);
static	void		BSP_Sensors_Cb_GetData( tBSP_PER_DataResp* data);
static	void		BSP_Sensors_Cb_Timer( void *argument);
static	bool		BSP_Sensors_Cb_Timer_SetData( tBSP_PER_Target target, tBSP_PER_Func function, uint16_t arg1, uint16_t arg2, uint16_t arg3);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * @brief
  * @retval
  */
void				BSP_Sensors_Init( I2C_HandleTypeDef *handle)
{
	Main_Info.hI2C		= handle;

	memset(Main_Targets, 0, sizeof(Main_Targets));

	Main_Targets[eBSP_PER_TARGET_SHT40A].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_STTS22].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LPS22D].handle		= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LIS2MDL].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LSM6DSV].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LSM6DSO].handle	= Main_Info.hI2C;
	Main_Targets[eBSP_PER_TARGET_LIS2DUX].handle	= Main_Info.hI2C;

	Main_Q	= osMessageQueueNew(48, sizeof(tQ_Sensor_Cmd), &Q_attributes);

	BSP_Sensors_InitSensors();
	BSP_Sensors_Cb_Timer(NULL);	//	MUST call this BEFORE calling osTimerStart() (it's a timer's Cb function)

	Main_Timer_idle	=	osTimerNew( BSP_Sensors_Cb_Timer, osTimerPeriodic, NULL, NULL);
	osTimerStart( Main_Timer_idle, pdMS_TO_TICKS(100));
}

/**
  * @brief
  * @retval
  */
void 				task_Sensors( void *arguments)
{
	static tQ_Sensor_Cmd	Cmd = {0};

	while(1)
	{
		osDelay(1);

		osMessageQueueGet(Main_Q, &Cmd, NULL, osWaitForever);

#ifdef	MY_DEBUG
	val2[4]	++;
#endif

		BSP_Sensors_TxCmd2Sensor(&Cmd);
	}
}

/**
  * @brief
  * @retval
  */
bool				BSP_Sensors_Cmd( tBSP_PER_DataCmd *Cmd, bool FromISR)
{
	Main_Q_Cmd.target	= Cmd->Target;
	Main_Q_Cmd.func		= Cmd->Function;

	if((Cmd->Target == eBSP_PER_TARGET_VOID) && (Cmd->Function == eBSP_PER_FUNC_VOID))
		Main_Pause ^= true;

	if( Main_Pause)
		return false;

	if( (Cmd->Function == eBSP_PER_FUNC_GET_REG) || (Cmd->Function == eBSP_PER_FUNC_SET_REG))
	{
		Main_Q_Cmd.arg1		= Cmd->Reg_addr;
		Main_Q_Cmd.arg2		= Cmd->Reg_data;
		Main_Q_Cmd.arg3		= (bool)(Cmd->Function == eBSP_PER_FUNC_SET_REG);
	}

	if( Cmd->Function == eBSP_PER_FUNC_SPECIAL_1)
	{
		Main_Q_Cmd.arg1		= Cmd->idx;
	}

	switch( Main_Q_Cmd.target)
	{
	case	eBSP_PER_TARGET_SHT40A:
		switch( Main_Q_Cmd.func)
		{
		case	eBSP_PER_FUNC_TEMP:
		case	eBSP_PER_FUNC_RH:
		case	eBSP_PER_FUNC_TEMP_RH:
			Main_Q_Cmd.arg1	= Cmd->Precision;
			break;
		}
		break;

	case	eBSP_PER_TARGET_STTS22:
		switch( Main_Q_Cmd.func)
		{
		case	eBSP_PER_FUNC_SET_CTRL:
			Main_Q_Cmd.arg1	= Cmd->Control;
			break;
		}
		break;

	case	eBSP_PER_TARGET_LIS2MDL:
		switch( Main_Q_Cmd.func)
		{
		case	eBSP_PER_FUNC_SET_CTRL:
			Main_Q_Cmd.arg1	= Cmd->Control;
			Main_Q_Cmd.arg2	= Cmd->idx;
			break;
		}
		break;
	}

	// no target or no function
	if((Cmd->Target == eBSP_PER_TARGET_VOID) || (Cmd->Function == eBSP_PER_FUNC_VOID))
	{
#ifdef	MY_DEBUG
		cnt3er[2]	++;
#endif
		return false;
	}
#ifdef	MY_DEBUG
	cnt3ok[2]	++;
#endif

	osStatus_t	status;
	if( FromISR)
	{
		status	= osMessageQueuePut(Main_Q, &Main_Q_Cmd, 0, 0); // ISR! timeout MUST be 0

#ifdef	MY_DEBUG
		if( status == osOK)	val2[2]	++;
		else				val2[3]	++;
#endif
	}
	else
	{
		status	= osMessageQueuePut(Main_Q, &Main_Q_Cmd, 0, osWaitForever);

#ifdef	MY_DEBUG
		if( status == osOK)	val2[0]	++;
		else				val2[1]	++;
#endif
	}

	return( status == osOK);
}

/**
  * @brief
  * @retval
  */
void				BSP_Sensors_SetErr( tBSP_PER_Target Source, tBSP_SENS_ErrCode ErrCode, bool Set)
{
	if( Source >= eBSP_PER_MAX_VALUE_TARGET)	return;
	if( ErrCode >= eBSP_SENS_MAX_VALUE_ERR)		return;

	switch( ErrCode)
	{
	case	eBSP_SENS_ERR_Q_LVL:
		Main_Targets[Source].err_Q_Lvl = Set;
		break;
	}
}

/**
  * @brief
  * @retval
  */
static	void		BSP_Sensors_InitSensors( void)
{
	BSP_SHT40_Init	(Main_Targets[eBSP_PER_TARGET_SHT40A].handle, 	BSP_Sensors_Cb_GetData);
	BSP_STTS22_Init	(Main_Targets[eBSP_PER_TARGET_STTS22].handle,	BSP_Sensors_Cb_GetData);
	BSP_LPS22D_Init	(Main_Targets[eBSP_PER_TARGET_LPS22D].handle, 	BSP_Sensors_Cb_GetData);
	BSP_LIS2MDL_Init(Main_Targets[eBSP_PER_TARGET_LIS2MDL].handle, 	BSP_Sensors_Cb_GetData);
	BSP_LSM6DSV_Init(Main_Targets[eBSP_PER_TARGET_LSM6DSV].handle, 	BSP_Sensors_Cb_GetData);
	BSP_LSM6DSO_Init(Main_Targets[eBSP_PER_TARGET_LSM6DSO].handle, 	BSP_Sensors_Cb_GetData);
	BSP_LIS2DUX_Init(Main_Targets[eBSP_PER_TARGET_LIS2DUX].handle, 	BSP_Sensors_Cb_GetData);
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_Sensors_TxCmd2Sensor( tQ_Sensor_Cmd	*cmd)
{
	tBSP_PER_DataCmd	Cmd = {0};

	if( Main_Targets[cmd->target].err_Q_Lvl == true)
	{
#ifdef	MY_DEBUG
		cnt3er[0]	++;
#endif
		return false;
	}
#ifdef	MY_DEBUG
	cnt3ok[0]	++;
#endif

	Cmd.Target		=	cmd->target;
	Cmd.Function	=	cmd->func;

	if( (cmd->func == eBSP_PER_FUNC_GET_REG) || (cmd->func == eBSP_PER_FUNC_SET_REG))
	{
		Cmd.Reg_addr	= cmd->arg1;
		Cmd.Reg_data	= cmd->arg2;
		Cmd.Reg_set		= cmd->arg3;
	}

	if( cmd->func == eBSP_PER_FUNC_SPECIAL_1)
	{
		Cmd.idx			= cmd->arg1;
	}

	switch(cmd->target)
	{
	case	eBSP_PER_TARGET_SHT40A:
		switch(cmd->func)
		{
		case	eBSP_PER_FUNC_TEMP:
		case	eBSP_PER_FUNC_RH:
		case	eBSP_PER_FUNC_TEMP_RH:
			Cmd.Precision	= cmd->arg1;
			break;
		}

		BSP_SHT40_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_STTS22:
		switch(cmd->func)
		{
		case	eBSP_PER_FUNC_SET_CTRL:
			Cmd.Control	= cmd->arg1;
			break;
		}

		BSP_STTS22_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_LPS22D:
		BSP_LPS22D_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_LIS2MDL:
		switch(cmd->func)
		{
		case	eBSP_PER_FUNC_SET_CTRL:
			Cmd.Control	= cmd->arg1;
			Cmd.idx		= cmd->arg2;
			break;
		}

		BSP_LIS2MDL_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_LSM6DSV:
		BSP_LSM6DSV_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_LSM6DSO:
		BSP_LSM6DSO_Cmd(&Cmd);
		break;

	case	eBSP_PER_TARGET_LIS2DUX:
		BSP_LIS2DUX_Cmd(&Cmd);
		break;
	}

	return true;
}

/**
  * @brief
  * @retval
  */
static	void		BSP_Sensors_Cb_GetData( tBSP_PER_DataResp* data)
{
	printf("BSP_Sensors_Cb_GetData\n");
}

/**
  * @brief
  * @retval
  */
static	void		BSP_Sensors_Cb_Timer( void *argument)
{
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_SHT40A,	eBSP_PER_FUNC_TEMP_RH,	eBSP_PER_PRCSN_HIGH, 0, 0);
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_STTS22,	eBSP_PER_FUNC_TEMP_RH,	0, 0, 0);
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_LPS22D,	eBSP_PER_FUNC_GET_SN,	0, 0, 0);
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_LIS2MDL,	eBSP_PER_FUNC_TEMP_RH,	0, 0, 0);
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_LSM6DSV,	eBSP_PER_FUNC_TEMP_RH,	0, 0, 0);
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_LIS2DUX,	eBSP_PER_FUNC_GET_SN,	0, 0, 0);
	BSP_Sensors_Cb_Timer_SetData( eBSP_PER_TARGET_LSM6DSO,	eBSP_PER_FUNC_TEMP_RH,	0, 0, 0);
}

/**
  * @brief
  * @retval
  */
static	bool		BSP_Sensors_Cb_Timer_SetData( tBSP_PER_Target target, tBSP_PER_Func function, uint16_t arg1, uint16_t arg2, uint16_t arg3)
{
	tBSP_PER_DataCmd	Cmd	=	{	.Target		=	target,
									.Function	=	function};
	if( Main_Pause)
		return false;

	if( Main_Targets[target].err_Q_Lvl == true)
	{
#ifdef	MY_DEBUG
		cnt3er[1]	++;
#endif
		return false;
	}
#ifdef	MY_DEBUG
	cnt3ok[1]	++;
#endif

	if( (function == eBSP_PER_FUNC_GET_REG) || (function == eBSP_PER_FUNC_SET_REG))
	{
		Cmd.Reg_addr	= arg1;
		Cmd.Reg_data	= arg2;
		Cmd.Reg_set		= arg3;
	}

	if( (target == eBSP_PER_TARGET_SHT40A) && (function == eBSP_PER_FUNC_TEMP_RH))
		Cmd.Precision	= arg1;

	BSP_Sensors_Cmd( &Cmd, false);

	return true;
}

/* USER CODE END 4 */

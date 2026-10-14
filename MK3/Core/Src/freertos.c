/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "tetris_app.h"
/* USER CODE END Includes */
osThreadId InputTaskHandle, GameTaskHandle, DisplayTaskHandle, AudioTaskHandle;
void StartInputTask(void const *argument);
void StartGameTask(void const *argument);
void StartDisplayTask(void const *argument);
void StartAudioTask(void const *argument);
/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t idleTCB;
static StackType_t idleStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory(StaticTask_t **tcb, StackType_t **stack, uint32_t *size)
{
  *tcb = &idleTCB; *stack = idleStack; *size = configMINIMAL_STACK_SIZE;
}
/* USER CODE END GET_IDLE_TASK_MEMORY */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  TetrisApp_Init();
  /* USER CODE END Init */
  osThreadDef(InputTask, StartInputTask, osPriorityAboveNormal, 0, 384);
  InputTaskHandle = osThreadCreate(osThread(InputTask), NULL);
  osThreadDef(GameTask, StartGameTask, osPriorityNormal, 0, 768);
  GameTaskHandle = osThreadCreate(osThread(GameTask), NULL);
  osThreadDef(DisplayTask, StartDisplayTask, osPriorityBelowNormal, 0, 768);
  DisplayTaskHandle = osThreadCreate(osThread(DisplayTask), NULL);
  osThreadDef(AudioTask, StartAudioTask, osPriorityNormal, 0, 384);
  AudioTaskHandle = osThreadCreate(osThread(AudioTask), NULL);
  /* USER CODE BEGIN RTOS_THREADS */
  configASSERT(InputTaskHandle && GameTaskHandle && DisplayTaskHandle && AudioTaskHandle);
  /* USER CODE END RTOS_THREADS */
}
void StartInputTask(void const *argument)
{
  /* USER CODE BEGIN StartInputTask */
  TetrisApp_InputTask(argument);
  /* USER CODE END StartInputTask */
}
void StartGameTask(void const *argument)
{
  /* USER CODE BEGIN StartGameTask */
  TetrisApp_GameTask(argument);
  /* USER CODE END StartGameTask */
}
void StartDisplayTask(void const *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  TetrisApp_DisplayTask(argument);
  /* USER CODE END StartDisplayTask */
}

void StartAudioTask(void const *argument)
{
  /* USER CODE BEGIN StartAudioTask */
  TetrisApp_AudioTask(argument);
  /* USER CODE END StartAudioTask */
}

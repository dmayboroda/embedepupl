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
 * Copyright (c) 2017 STMicroelectronics International N.V.
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "fatfs.h"
#include "lcd.h"
#include "waveplayer.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId soundLoopHandle;
osMessageQId SDQueueHandle;

/* USER CODE BEGIN Variables */
osMutexDef (sd_mutex);    // Declare mutex
osMutexId  (sd_mutex_id); // Mutex ID
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void SoundTask(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
void DrawSprite(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		const char * path);
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */
	//BSP_SD_Init();
	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	sd_mutex_id = osMutexCreate(osMutex(sd_mutex));
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 4096);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* definition and creation of soundLoop */
	osThreadDef(soundLoop, SoundTask, osPriorityBelowNormal, 0, 4096);
	soundLoopHandle = osThreadCreate(osThread(soundLoop), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Create the queue(s) */
	/* definition and creation of SDQueue */
	osMessageQDef(SDQueue, 8, uint16_t);
	SDQueueHandle = osMessageCreate(osMessageQ(SDQueue), NULL);

	/* USER CODE BEGIN RTOS_QUEUES */
	BSP_SD_Init();
	MX_FATFS_Init();
	/* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument) {
	/* init code for FATFS */


	/* USER CODE BEGIN StartDefaultTask */
	const uint32_t spriteDelay = 50;

	uint32_t xMove = 20;

	for (;;) {
//		DrawSprite(0, 0, 320, 240, "hal_9000.raw");
//		osDelay(5000);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "run0.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "run1.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "run2.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "run3.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "run4.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "run5.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "idle0.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "idle1.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "idle2.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "idle3.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "jump0.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "jump1.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "grab0.raw");
		osDelay(spriteDelay);
		DrawSprite(0, 0, 320, 240, "screen.raw");
		xMove += 5;
		DrawSprite(xMove, 120, 40, 40, "hurt0.raw");
		osDelay(spriteDelay);
		if (xMove > 200)
			xMove = 20;
	}
	/* USER CODE END StartDefaultTask */
}

/* SoundTask function */
void SoundTask(void const * argument) {
	/* USER CODE BEGIN SoundTask */
	retSD = f_mount(&SDFatFS, SDPath, 1);
	/* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	while (1) {
		osDelay(1000);
		AUDIO_PLAYER_Start("hal_9000.wav");
		for (;;) {
			if (AudioState == AUDIO_STATE_STOP)
				break;

			AUDIO_PLAYER_Process();
			osDelay(1);
		}

	}
	/* USER CODE END SoundTask */
}

/* USER CODE BEGIN Application */

void DrawSprite(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
		const char * path) {

	RTRY:
	retSD = f_open(&SDFile, path, FA_OPEN_EXISTING | FA_READ);
	if (retSD == FR_OK) {

		LCD_DrawRAWFromFile(x, y, w, h, (w != 320), &SDFile);
		/* Close file */
		f_close(&SDFile);
	} else {
		osDelay(1);
		goto RTRY;
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

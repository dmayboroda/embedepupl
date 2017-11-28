/**
 ******************************************************************************
 * @file    Audio/Audio_playback_and_record/Src/waveplayer.c
 * @author  MCD Application Team
 * @brief   This file provides the Audio Out (playback) interface API
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
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
#include "waveplayer.h"
#include "dma.h"
#include "dac.h"
#include "tim.h"
#include "lcd.h"
#include "string.h"

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static AUDIO_OUT_BufferTypeDef BufferCtl;
AUDIO_PLAYBACK_StateTypeDef AudioState;
static int16_t FilePos = 0;
static __IO uint32_t uwVolume = 70;

WAVE_FormatTypeDef WaveFormat;
FIL WavFile;

/* Private function prototypes -----------------------------------------------*/
static AUDIO_ErrorTypeDef GetFileInfo(char *path, WAVE_FormatTypeDef *info);

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Starts Audio streaming.
 * @param  idx: File index
 * @retval Audio error
 */
AUDIO_ErrorTypeDef AUDIO_PLAYER_Start(char *path) {
	uint32_t bytesread;
	if (AudioState == AUDIO_STATE_PLAY)
		return AUDIO_ERROR_NONE;
	f_close(&WavFile);

	GetFileInfo(path, &WaveFormat);

	memset(&BufferCtl.buff[0], 0, AUDIO_OUT_BUFFER_SIZE);
	/*Adjust the Audio frequency */
	// PlayerInit(WaveFormat.SampleRate);
	BufferCtl.state = BUFFER_OFFSET_NONE;
	BufferCtl.fptr = 0;

	/* Fill whole buffer at first time */
	if (f_read(&WavFile, &BufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE,
			(void *) &bytesread) == FR_OK) {
		AudioState = AUDIO_STATE_PLAY;
		{
			if (bytesread != 0) {

				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, &BufferCtl.buff[0],
				AUDIO_OUT_BUFFER_SIZE / 2, DAC_ALIGN_8B_R);
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, &BufferCtl.buff[0],
				AUDIO_OUT_BUFFER_SIZE / 2, DAC_ALIGN_8B_R);
				HAL_TIM_Base_Start(&htim6);
				BufferCtl.fptr += bytesread;
				return AUDIO_ERROR_NONE;
			}
		}
	}

	return AUDIO_ERROR_IO;
}

/**
 * @brief  Manages Audio process.
 * @param  None
 * @retval Audio error
 */
AUDIO_ErrorTypeDef AUDIO_PLAYER_Process(void) {
	uint32_t bytesread;
	AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
	static uint32_t prev_elapsed_time = 0xFFFFFFFF;

	switch (AudioState) {
	case AUDIO_STATE_PLAY:
		if (BufferCtl.fptr >= WaveFormat.FileSize - sizeof(WaveFormat)) {
			AudioState = AUDIO_STATE_IDLE;
		}

		if (BufferCtl.state == BUFFER_OFFSET_HALF) {
//			memset(&BufferCtl.buff[0], 0, AUDIO_OUT_BUFFER_SIZE / 2);
			if (f_read(&WavFile, &BufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE / 2,
					(void *) &bytesread) != FR_OK) {
				return AUDIO_ERROR_IO;
			}

			BufferCtl.state = BUFFER_OFFSET_NONE;
			BufferCtl.fptr += bytesread;

			if (bytesread == 0)
				AudioState = AUDIO_STATE_IDLE;
		}

		if (BufferCtl.state == BUFFER_OFFSET_FULL) {
//			memset(&BufferCtl.buff[AUDIO_OUT_BUFFER_SIZE / 2], 0, AUDIO_OUT_BUFFER_SIZE / 2);
			if (f_read(&WavFile, &BufferCtl.buff[AUDIO_OUT_BUFFER_SIZE / 2],
			AUDIO_OUT_BUFFER_SIZE / 2, (void *) &bytesread) != FR_OK) {
				return AUDIO_ERROR_IO;
			}

			BufferCtl.state = BUFFER_OFFSET_NONE;
			BufferCtl.fptr += bytesread;

			if (bytesread == 0)
				AudioState = AUDIO_STATE_IDLE;
		}
		break;

	case AUDIO_STATE_STOP:

		AUDIO_PLAYER_Stop();
		break;

	case AUDIO_STATE_WAIT:
	case AUDIO_STATE_IDLE:
	case AUDIO_STATE_INIT:
	default:
		/* Do Nothing */
		break;
	}
	return audio_error;
}

/**
 * @brief  Stops Audio streaming.
 * @param  None
 * @retval Audio error
 */
AUDIO_ErrorTypeDef AUDIO_PLAYER_Stop(void) {
	AudioState = AUDIO_STATE_STOP;
	FilePos = 0;

	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_TIM_Base_Stop(&htim6);

//	f_close(&WavFile);
	return AUDIO_ERROR_NONE;
}

/**
 * @brief  Calculates the remaining file size and new position of the pointer.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	if (AudioState == AUDIO_STATE_IDLE) {
		AUDIO_PLAYER_Stop();
	}

	if (AudioState == AUDIO_STATE_PLAY) {
		BufferCtl.state = BUFFER_OFFSET_FULL;
	}
}

/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	if (AudioState == AUDIO_STATE_IDLE) {
		AUDIO_PLAYER_Stop();
	}

	if (AudioState == AUDIO_STATE_PLAY) {
		BufferCtl.state = BUFFER_OFFSET_HALF;
	}
}
/*******************************************************************************
 Static Functions
 *******************************************************************************/

/**
 * @brief  Gets the file info.
 * @param  file_idx: File index
 * @param  info: Pointer to WAV file info
 * @retval Audio error
 */
static AUDIO_ErrorTypeDef GetFileInfo(char *path, WAVE_FormatTypeDef *info) {
	uint32_t bytesread;
	uint32_t duration;

	if (f_open(&WavFile, path, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
		/* Fill the buffer to Send */
		if (f_read(&WavFile, info, sizeof(WaveFormat), (void *) &bytesread)
				== FR_OK) {
			return AUDIO_ERROR_NONE;
		}
		f_close(&WavFile);
	}
	return AUDIO_ERROR_IO;
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
	BSP_AUDIO_OUT_HalfTransfer_CallBack();
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
	BSP_AUDIO_OUT_TransferComplete_CallBack();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

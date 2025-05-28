/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "LCD.h"
#include <stdlib.h>
#include "TIMER.h"
#include "Keypad.h"
#include <string.h>
#include <stdio.h>

// flags
#define UART_REC 0x0001
#define BUFF_FULL 0x0002
#define LINE_DONE 0x0004

//flag for main
#define POUND_DETECT 0x0008

#define BUFFSIZE 11
#define NUMFIFOITEMS 5

// structure for fifo implementation
typedef struct
{
   char buffer[BUFFSIZE];
   unsigned short nextBuf;
} fifo_item_struct_t;

// init the fifo struct
fifo_item_struct_t buff_fifo[NUMFIFOITEMS] = {
	{"", 1},
	{"", 2},
	{"", 3},
	{"", 4},
	{"", 0},
};

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

unsigned short flags;
char Buffer[1];			// for receiving and send single byte use

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);

void UART_SEND(UART_HandleTypeDef*, char[]);
int get_line(char[], int);

// timer interrupt callback
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	TIMER2_HANDLE();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	flags |= UART_REC;
}

int main(void)
{
	fifo_item_struct_t *curItem;

	char ClearScreen[] = { 0x1B, '[', '2' , 'J',0 }; 	// Clear the screen
	char CursorHome[] = { 0x1B, '[' , 'H' , 0 }; 	// Home the cursor
	char *initMsg = "Enter Phone Numbers\r\n";
	//char *endMsg = "End of FIFO";

	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_USART2_UART_Init();
	HAL_UART_Receive_IT(&huart2, (uint8_t *)Buffer, 1);
	HAL_TIM_Base_Start_IT(&htim2);

	initKeypad();
	LcdInit();

	UART_SEND(&huart2, ClearScreen);	// clear screen with the phrase: ESC, [ 2 J NUL
	UART_SEND(&huart2, CursorHome);	// move cursor position home with the phrase: ESC, [, H, NUL
	UART_SEND(&huart2, initMsg);

	while (1)
	{
		curItem = &buff_fifo[0];

		// first collect phone numbers into fifo
		for (int j = 0; j < NUMFIFOITEMS; j++) {
			get_line(curItem->buffer, BUFFSIZE);
			UART_SEND(&huart2, "\r\n");
			curItem = &buff_fifo[curItem->nextBuf];
		}
		UART_SEND(&huart2, "Buffer full!\r\n");

		// display the first one
		LcdGoto(0, 0);
		LcdPutS(curItem->buffer);

		// next display one at a time until end reached
		while (curItem->nextBuf != 0) {
			if (sTimer[KEY_SCAN_TIMER] == 0)
			{
				Keypadscan();
				flags |= KeyProcess();
				sTimer[KEY_SCAN_TIMER] = KEY_SCAN_TIME;
			}
			if (flags & POUND_DETECT) {
				// midterm change: check if next to last and if so print both
				if ((curItem->nextBuf) == (NUMFIFOITEMS-2)) {
					curItem = &buff_fifo[curItem->nextBuf];
					LcdGoto(0, 0);
					LcdPutS(curItem->buffer);
					curItem = &buff_fifo[curItem->nextBuf];
					LcdGoto(1, 0);
					LcdPutS(curItem->buffer);
				} else {
					curItem = &buff_fifo[curItem->nextBuf];
					LcdGoto(0, 0);
					LcdPutS(curItem->buffer);
				}
				flags &= ~POUND_DETECT;
			}
		}

		// now display end since on last:
		//LcdGoto(1, 0);
		//LcdPutS(endMsg);

		// clear and reset everything when done
		while (1) {
			if (sTimer[KEY_SCAN_TIMER] == 0)
			{
				Keypadscan();
				flags |= KeyProcess();
				sTimer[KEY_SCAN_TIMER] = KEY_SCAN_TIME;
			}
			if (flags & POUND_DETECT) {
				LcdInit();
				UART_SEND(&huart2, ClearScreen);	// clear screen with the phrase: ESC, [ 2 J NUL
				UART_SEND(&huart2, CursorHome);	// move cursor position home with the phrase: ESC, [, H, NUL
				UART_SEND(&huart2, initMsg);
				flags &= ~POUND_DETECT;
				break;
			}
		}
	}
}


// for getting the next phone number
int get_line(char line[], int max)
{

	int i=0;	// Index record to save each char

	while (!(flags & LINE_DONE) && (i < max-1))		// Once in this function, it'll get char till '\r'
	{
	  if (flags & UART_REC)		// Only work when UART interrupt detected
	  {
		 flags &= ~UART_REC;	// clear the flag
		 HAL_UART_Receive_IT(&huart2, (uint8_t *)Buffer, 1);
	     HAL_UART_Transmit(&huart2, (uint8_t *)Buffer, 1, HAL_MAX_DELAY);  // Echo back

		 if (Buffer[0] != '\r') // if not carriage return, save to array
		   line[i++] = Buffer[0];
		 else
		   flags |= LINE_DONE;
	  }
	}

	flags &= ~LINE_DONE;
	line[i] = '\0'; 				// terminate with null
	return i;
}

// send string to UART
void UART_SEND(UART_HandleTypeDef *huart, char buffer[])
{
    HAL_UART_Transmit(huart, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 3999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim2.Init.Period = 19;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8
                          |GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA8
                           PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_8
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

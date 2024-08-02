/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
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
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAX_CELLS 2
#define MAX_TEMPS 5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char buffer[256] = {0};
char trial[265];
char tempBuffer[256] = {0}; // Temporary buffer to store a complete message
int bufferIndex = 0; // Current index in the temporary buffer
double cellVoltages[MAX_CELLS] = {0}; // Assuming a maximum of 10 cells
double tempValues[MAX_TEMPS] = {0};
float min_cell, max_cell, min_temp, max_temp;
int pri_current, pri_current_fa, sec_current;
int string;

char received_char;


char buffer1[100];

int P0DE7_flag = 0;
int P1C01_flag = 0;
int P0DE6_flag = 0;
int P1C00_flag = 0;
int P1A9B_flag = 0;
int P0A7E_flag = 0;
int P1A9A_flag = 0;
int P0CA7_flag = 0;

clock_t start_time;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void process_message(void) {
    char* token = strtok(tempBuffer, " ");
    int index = 0;
    double value = 0;

    while (token != NULL) {
        if (sscanf(token, "Cell%d:%lf", &index, &value) == 2) {
            if (index >= 0 && index < MAX_CELLS) {
                cellVoltages[index] = value;
                sprintf(buffer, "Updated Cell%d: %.2lf\n", index, value);
                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
            }
        } else if (sscanf(token, "Temp%d:%lf", &index, &value) == 2) {
            if (index >= 0 && index < MAX_TEMPS) {
                tempValues[index] = value;
                sprintf(buffer, "Updated Temp%d: %.2lf\n", index, value);
                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
            }
        } else if (sscanf(token, "MinCell:%lf", &value) == 1) {
        		min_cell = value;
                sprintf(buffer, "Updated min_cell %lf\n", value);
                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        } else if (sscanf(token, "MaxCell:%lf", &value) == 1) {
    			max_cell = value;
    			sprintf(buffer, "Updated max_cell %lf\n", value);
    			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        }else if (sscanf(token, "MinTemp:%lf", &value) == 1) {
    			min_temp = value;
    			sprintf(buffer, "Updated min_temp %lf\n", value);
    			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        } else if (sscanf(token, "MaxTemp:%lf", &value) == 1) {
				max_temp = value;
				sprintf(buffer, "Updated max_temp %lf\n", value);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        } else if (sscanf(token, "$0314_%d\n", &value) == 1) {
            	string = value;
            	P1C01_flag = 0;
            	P1C00_flag = 0;
            	P0A7E_flag = 0;
            	sprintf(buffer, "P1C01, P0A7E, and P1C00 flag reset manually\n");
            	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        } else if (sscanf(token, "$0321_%d\n", &value) == 1) {
        		string = value;
        		P0CA7_flag = 0;
        		sprintf(buffer, "P0CA7 flag reset manually\n");
        		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        }else {
            	sprintf(buffer, "Failed to parse token: %s\n", token);
            	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        }
        token = strtok(NULL, " ");
    }
    memset(tempBuffer, 0, sizeof(tempBuffer));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		if (bufferIndex < sizeof(tempBuffer) - 1){
			tempBuffer[bufferIndex++] = received_char;
			tempBuffer[bufferIndex] = '\0';

			if (received_char == '\n') {
				process_message();
				bufferIndex = 0;
			}
		}else {
			bufferIndex = 0;
		}

		HAL_UART_Receive_IT(&huart2, (uint8_t *)&received_char, 1);
	}
}

void P0DE7(void) {
	int P0DE7_condition_met = 0;


		if(max_cell >= 4.25){
			P0DE7_condition_met = 1;
		}

	static uint32_t start_time;
	static uint8_t P0DE7_flag = 0;
	char buffer[100];

	if(P0DE7_condition_met){
		if(!P0DE7_flag){
			start_time = HAL_GetTick();
			P0DE7_flag = 1;
			sprintf(buffer, "P0DE7 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 3){
				 HAL_UART_Transmit(&huart2, (uint8_t*)"P0DE7\n", 6, 100);
			 }
		}
	}else{
		int all_cells_below_threshold = 1;
			if(max_cell < 4.23){
				all_cells_below_threshold = 0;
			}

		if(all_cells_below_threshold){
			P0DE7_flag = 0;
            sprintf(buffer, "All cells below threshold, P0DE7 reset.\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}
	}
}

void P1C01(void){
	int P1C01_condition_met = 0;

			if(max_cell >= 4.35){
				P1C01_condition_met = 1;
			}

		static uint32_t start_time;
		char buffer[100];

		if(P1C01_condition_met){
			if(!P1C01_flag){
				start_time = HAL_GetTick();
				P1C01_flag = 1;
				sprintf(buffer, "P1C01 condition met, Start_time set: %.2f s\n", (float)start_time/1000.00);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			}else{

			     uint32_t current_time = HAL_GetTick();
				 float elapsed_time = (float)(current_time - start_time) / 1000.0;

			     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

				 if(elapsed_time >= 4){
					 HAL_UART_Transmit(&huart2, (uint8_t*)"P1C01\n", 6, 100);
				 }
			}
		}
}
void P0DE6(void) {
	int P0DE6_condition_met = 0;

		if(min_cell <= 2.8){
			P0DE6_condition_met = 1;
		}
	static uint32_t start_time;
	static uint8_t P0DE6_flag = 0;
	char buffer[100];

	if(P0DE6_condition_met){
		if(!P0DE6_flag){
			start_time = HAL_GetTick();
			P0DE6_flag = 1;
			sprintf(buffer, "P0DE6 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 4){
				 HAL_UART_Transmit(&huart2, (uint8_t*)"P0DE6\n", 6, 100);
			 }
		}
	}else{
		int all_cells_below_threshold = 1;
			if(min_cell < 2.9){
				all_cells_below_threshold = 0;
			}
		if(all_cells_below_threshold){
			P0DE6_flag = 0;
            sprintf(buffer, "All cells below threshold, P0DE6 reset.\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}
	}
}

void P1C00(void) {
	int P1C00_condition_met = 0;

		if(min_cell <= 1.7){
			P1C00_condition_met = 1;
		}
	static uint32_t start_time;
	char buffer[100];

	if(P1C00_condition_met){
		if(!P1C00_flag){
			start_time = HAL_GetTick();
			P1C00_flag = 1;
			sprintf(buffer, "P1C00 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 3){
				 HAL_UART_Transmit(&huart2, (uint8_t*)"P1C00\n", 6, 100);
			 }
		}
	}
}

void P1A9B(void) {
	int P1A9B_condition_met = 0;

		if(max_temp >= 58){
			P1A9B_condition_met = 1;
		}

	static uint32_t start_time;
	static uint8_t P1A9B_flag = 0;
	char buffer[100];

	if(P1A9B_condition_met){
		if(!P1A9B_flag){
			start_time = HAL_GetTick();
			P1A9B_flag = 1;
			sprintf(buffer, "P1A9B condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 3){
				 HAL_UART_Transmit(&huart2, (uint8_t*)"P1A9B\n", 6, 100);
			 }
		}
	}else{
		int all_cells_below_threshold = 1;
			if(max_temp < 55){
				all_cells_below_threshold = 0;
			}
		if(all_cells_below_threshold){
			P1A9B_flag = 0;
            sprintf(buffer, "All cells below threshold, P1A9B reset.\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}
	}
}

void P0A7E(void) {
	int P0A7E_condition_met = 0;

		if(max_temp >= 65){
			P0A7E_condition_met = 1;
		}

	static uint32_t start_time;
	char buffer[100];

	if(P0A7E_condition_met){
		if(!P0A7E_flag){
			start_time = HAL_GetTick();
			P0A7E_flag = 1;
			sprintf(buffer, "P0A7E condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 4){
				 HAL_UART_Transmit(&huart2, (uint8_t*)"P0A7E\n", 6, 100);
			 }
		}
	}
}

void P0CA7(void)
{
	int P0CA7_condition_met = 0;

	static uint32_t start_time;
	static uint8_t P0CA7_flag = 0;
	char buffer[100]; 
		
	if((pri_current >= 1350 || pri_current <= -1350) || (pri_current_fa = 1 && sec_current >= 1350))
	{
		P0CA7_condition_met = 1;
	}

		if(P0CA7_condition_met){
			if(!P0CA7_flag){
				//first time entering the condition.
				start_time = HAL_GetTick();
				P0CA7_flag = 1;
				sprintf(buffer, "P0CA7 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			}else{

			     uint32_t current_time = HAL_GetTick();
				 float elapsed_time = (float)(current_time - start_time) / 1000.0;

			     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

				 if(elapsed_time >= 0.5){
					 HAL_UART_Transmit(&huart2, (uint8_t*)"P0CA7\n", 6, 100);
				 }
			}
		}else{
			int all_cells_below_threshold = 1;
				if(max_cell < 4.23){
					all_cells_below_threshold = 0;
				}

			if(all_cells_below_threshold){
				P0CA7_flag = 0;
	            sprintf(buffer, "All cells below threshold, P0CA7 reset.\n");
	            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			}
		}
}






void display_values(void) {
	char msg[64];

	//Display cell voltages
	for (int i = 0; i < MAX_CELLS; i++){
		sprintf(msg,"Cell%d:%.2lf\n\r", i, cellVoltages[i]);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
	}
	// Display temp values
	for (int i = 0; i < MAX_TEMPS; i++){
		sprintf(msg, "Temp%d:%.2lf\n\r", i, tempValues[i]);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, (uint8_t *)&received_char, 1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  // Start PWM Output
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      display_values();

      //PWM_Signal();
      P0DE7();
      P1C01();
      P0DE6();
      P1C00();
      P1A9B();
      P0A7E();
      HAL_Delay(1000);

  }
  /* USER CODE END 3 */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
      Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 16;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 10;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 11;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 12;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 13;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 14;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 15;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = 16;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 255;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 4749;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
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
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD10 PD11 PD12 PD13
                           PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

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

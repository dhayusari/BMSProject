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
#include <math.h>
#include <time.h>
#include <stdbool.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


#define TXBUFLEN (225);
#define MAX_CELLS 200
#define MAX_TEMPS 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile uint16_t adcResultsDMA[16]; //results of the potentiometer
const int adcChannelCount = sizeof(adcResultsDMA)/ sizeof(adcResultsDMA[0]);
volatile int adcConversionCplt = 0; //set by callback

//volatile uint16_t tempResultsDMA[2]; //results of the potentiometer
//const int tempChannelCount = sizeof(tempResultsDMA)/ sizeof(tempResultsDMA[0]);
//volatile int tempConversionCplt = 0; //set by callback

char buffer[256] = {0};
char tempBuffer[256] = {0}; // Temporary buffer to store a complete message
int bufferIndex = 0; // Current index in the temporary buffer
double cellVoltages[MAX_CELLS] = {0}; // Assuming a maximum of 10 cells
double tempValues[MAX_TEMPS] = {0};
char received_char;

float min_cell, max_cell, min_temp, max_temp;
float pri_current, pri_current_fa, sec_current;
int string;



char buffer1[100];

int P0CA7_flag = 0;
int P0DE7_flag = 0;
int P1C01_flag = 0;
int P0DE6_flag = 0;
int P1C00_flag = 0;
int P1A9B_flag = 0;
int P0A7E_flag = 0;
int P1A9A_flag = 0;

//NOT CHECKED:
int P29FF_flag = 0;
int P0A0D_flag = 0;
int P0A0C_flag = 0;
int P0A0B_flag = 0;



int P0CA7_demature = 0;

clock_t start_time;
//#define VCC 5;
//#define R0 10000;
//#define B 3977;
//#define R_S 10000;

char txBuf[225];
char tempBuf[50];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

void print_char(uint32_t num_var);
void ManualDelay(volatile uint32_t nCount);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char PWMbuf[50];
uint32_t rising_edge1 = 0, rising_edge2 = 0, falling_edge = 0;
uint32_t period = 0, high_time = 0;
uint32_t freq = 0, duty_cycle = 0;
uint8_t first_capture = 0;
uint8_t msgSent = 0;
uint8_t counter = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    if (htim->Instance == TIM3)
    {
    	counter++;
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            if (first_capture == 0)
            {
                // First rising edge capture
                rising_edge1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                first_capture = 1;
            }
            else if (first_capture == 1)
            {
                // Second rising edge capture
                rising_edge2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                period = (rising_edge2 >= rising_edge1) ? (rising_edge2 - rising_edge1) : ((0xFFFF - rising_edge1) + rising_edge2 + 1);
                freq = SystemCoreClock / ((htim->Instance->PSC + 1) * period);
                first_capture = 0;

                // Calculate duty cycle
                high_time = (falling_edge >= rising_edge1) ? (falling_edge - rising_edge1) : ((0xFFFF - rising_edge1) + falling_edge + 1);
                duty_cycle = (100 * high_time) / period;


                if((freq >=82 && freq <= 94) && (duty_cycle >= 47 && duty_cycle <= 53))
                {
                	snprintf(PWMbuf, 50, "\n\rPWM Connected.\n\r");
					 //snprintf(PWMbuf, 50, "\n\rFreq: %lu Hz, Duty Cycle: %lu%%\n\r", freq, duty_cycle);
					 HAL_UART_Transmit(&huart2, (uint8_t *)PWMbuf, sizeof(PWMbuf), HAL_MAX_DELAY);

					 snprintf(PWMbuf, 50, "Duty Cycle: %d", duty_cycle);
					 HAL_UART_Transmit(&huart2, (uint8_t *)PWMbuf, sizeof(PWMbuf), HAL_MAX_DELAY);

					 snprintf(PWMbuf, 50, "Freq: %d", freq);
					 HAL_UART_Transmit(&huart2, (uint8_t *)PWMbuf, sizeof(PWMbuf), HAL_MAX_DELAY);

					 //msgSent = 1;

                }
                else
                {
                	snprintf(PWMbuf, 50, "\n\rPWM Disconnected.\n\r");
                	//snprintf(PWMbuf,50 , "\r\n PWM SIGNAL DOES NOT MATCH. DTC \n\r");
					HAL_UART_Transmit(&huart2, (uint8_t *)PWMbuf, sizeof(PWMbuf), HAL_MAX_DELAY);
					//msgSent = 1;
                }
//                else if(msgSent == 1 && counter > 100)
//                {
//                	//msgSent will be 1
//                	msgSent = 0;
//                }
                //if((freq <82 && freq > 94) && (duty_cycle < 47 && duty_cycle > 53))
            }
        }
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
            // Falling edge capture
            falling_edge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        }
    }
    else
    {
    	freq = 0;
    	duty_cycle = 0;

    }
}

//uint32_t freq, duty_cycle;
//uint32_t capture_val;
//
//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
//{
//	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//	{
//		//printf("instance: %s\n", htim->Instance);
//
//		capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
//		if(capture_val)
//		{
//			freq = SystemCoreClock / (capture_val);
//			duty_cycle = 100 * HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2) / capture_val;
//			snprintf(PWMbuf, 50, "\n freq: %lu duty cycle: %lu\n capture val: %lu \n\r", freq,duty_cycle, capture_val);
//			HAL_UART_Transmit(&huart2, (uint8_t *)PWMbuf, sizeof(PWMbuf), HAL_MAX_DELAY);
//		}
//		else
//		{
//			duty_cycle = 0;
//			freq = 0;
//		}
//	}
//}


void process_message(void) {
    char* token = strtok(tempBuffer, " ");
    int index = 0;
    double value = 0;
    int relay_num = 0;
    int relay_state = 0;
    int pc_state = 0;


    while (token != NULL) {
        if (sscanf(token, "Cell%d:%lf", &index, &value) == 2) {
            if (index >= 0 && index < MAX_CELLS) {
                cellVoltages[index] = value;

                snprintf(buffer, 256, "\nUpdated Cell%d: %.2lf\n", index, value);
                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
            }
        } else if (sscanf(token, "Temp%d:%lf", &index, &value) == 2) {
            if (index >= 0 && index < MAX_TEMPS) {
                tempValues[index] = value;
                snprintf(buffer, 256, "\nUpdated Temp%d: %.2lf\n", index, value);
                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) +1, HAL_MAX_DELAY);
            }
        }

		else if (sscanf(token, "Relay%d:%d", &relay_num, &relay_state) == 2) {
            switch (relay_num) {
                case 1:
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, relay_state ? 1 : 0);
                    snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", relay_num, relay_state);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);

                    break;
                case 2:
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, relay_state ? 1 : 0);
                    snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", relay_num, relay_state);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
                    break;
                case 3:
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, relay_state ? 1 : 0);
                    snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", relay_num, relay_state);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
                    break;
                case 4:
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, relay_state ? 1 : 0);
                    snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", relay_num, relay_state);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
                    break;
                case 5:
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, relay_state ? 1 : 0);
                    snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", relay_num, relay_state);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
                    break;
                default:
                    snprintf(buffer, 256, "Invalid relay number: %d\n", relay_num);
                    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) +1, HAL_MAX_DELAY);
                    break;
            }
        }
		else if (sscanf(token, "Precharge:%d", &pc_state) == 1)
		{
			// 2 then 3 then 1 then open 2 then open 3.
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, 0); //close 2
            snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", 2, 1);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
            ManualDelay(1000000); // Adjust the count value as needed

            //HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, 0); //close 3
            snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", 3, 1);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
            ManualDelay(1000000);
            //HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, 0); //close 1
            snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", 1, 1);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
            ManualDelay(1000000);
            //HAL_Delay(10);

			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, 1); //open 2
            snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", 2, 0);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
            ManualDelay(1000000);
            //HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, 1); //open 3
            snprintf(buffer, 256, "\nUpdated Relay %d: %d\n", 3, 0);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);

			snprintf(buffer, 256,"\nPrecharge Done\n");
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);

		}
		else if (sscanf(token, "MinCell:%lf", &value) == 1) {
		        		min_cell = value;
		                sprintf(buffer, "Updated min_cell %lf\n", value);
		                HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        }
		else if (sscanf(token, "MaxCell:%lf", &value) == 1) {
		    			max_cell = value;
		    			sprintf(buffer, "Updated max_cell %lf\n", value);
		    			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        }
		else if (sscanf(token, "MinTemp:%lf", &value) == 1) {
		    			min_temp = value;
		    			sprintf(buffer, "Updated min_temp %lf\n", value);
		    			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        }
		else if (sscanf(token, "MaxTemp:%lf", &value) == 1) {
						max_temp = value;
						sprintf(buffer, "Updated max_temp %lf\n", value);
						HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        }
		else if (sscanf(token, "$0314_%lf\n", &value) == 1) {
		            	string = value;
		            	P1C01_flag = 0;
		            	P1C00_flag = 0;
		            	P0A7E_flag = 0;
		            	P29FF_flag = 0;
		        		sprintf(buffer, "DTC P1C01 Demature\n");
		        		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        		sprintf(buffer, "DTC P1C00 Demature\n");
		        		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        		sprintf(buffer, "DTC P0A7E Demature\n");
		        		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        		sprintf(buffer, "DTC P29FF Demature\n");
		        		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

//		        		sprintf(buffer, "DTC P1C01 Demature, DTC P0A7E Demature, and DTC P1C00 Demature\n");
//		            	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        }
		else if (sscanf(token, "$0321_%lf\n", &value) == 1) {
		        		string = value;
		        		P0CA7_flag = 0;
		        		sprintf(buffer, "DTC P0CA7 Demature\n");
		        		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		        }
		else if (sscanf(token, "HV_current_Pri:%lf", &value) == 1) {
				pri_current = value;
				sprintf(buffer, "Updated HV_current_Pri %lf\n", pri_current);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}
		else if (sscanf(token, "HV_current_Pri_FA:%lf", &value) == 1) {
				pri_current_fa = value;
				sprintf(buffer, "Updated HV_current_Pri_FA %lf\n", pri_current_fa);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}
		else if (sscanf(token, "HV_current_Sec:%lf", &value) == 1) {
				sec_current = value;
				sprintf(buffer, "Updated HV_current_Sec %lf\n", sec_current);
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}

//		else if (sscanf(token, "$0314_%lf\n", &value) == 1) {
//				string = value;
//				P1C01_flag = 0;
//				P1C00_flag = 0;
//				P0A7E_flag = 0;
//				sprintf(buffer, "P1C01, P0A7E, and P1C00 flag reset manually\n");
//				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
//		}
//		else if (sscanf(token, "$0321_%lf\n", &value) == 1) {
//				string = value;
//				P0CA7_flag = 0;
//				sprintf(buffer, "P0CA7 demature is set manually\n");
//				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
//		}
        else {
            snprintf(buffer, 256, "Failed to parse token: %s\n", token);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer) + 1, HAL_MAX_DELAY);
        }
        token = strtok(NULL, " ");
    }

    memset(tempBuffer, 0, sizeof(tempBuffer));
}

void ManualDelay(volatile uint32_t nCount)
{
    while(nCount--)
    {
        __asm("nop"); // no operation, prevents optimization
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART2) {
        if (bufferIndex < sizeof(tempBuffer) - 1) {
            tempBuffer[bufferIndex++] = received_char;
            tempBuffer[bufferIndex] = '\0';

            if (received_char == '\n') {
                process_message();
                bufferIndex = 0;
            }
        } else {
            // Buffer overflow case
            bufferIndex = 0;
            sprintf(buffer, "Buffer overflow detected\n");
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        }

        HAL_UART_Receive_IT(&huart2, (uint8_t *)&received_char, 1);
    }
}
//
//void P0CA7(void)
//{
//	int P0CA7_condition_met = 0;
//
//	if(pri_current >= 1350 || pri_current <= -1350){
//		P0CA7_condition_met = 1;
//	}
//
//		static uint32_t start_time;
//		static uint8_t P0CA7_flag = 0; //static: it initializes only once. then retains its value for all future func calls.
//		char buffer[100];
//
//		if(P0CA7_condition_met){
//			if(!P0CA7_flag){
//				//first time entering the condition.
//				start_time = HAL_GetTick();
//				P0CA7_flag = 1;
//				sprintf(buffer, "P0CA7 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
//				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
//			}else{
//
//			     uint32_t current_time = HAL_GetTick();
//				 float elapsed_time = (float)(current_time - start_time) / 1000.0;
//
//			     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
//				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
//
//				 if(elapsed_time >= 0.5){
//					 HAL_UART_Transmit(&huart2, (uint8_t*)"P0CA7\n", 6, 100);
//				 }
//			}
//		}
//		else{
//			 HAL_UART_Transmit(&huart2, (uint8_t*)"P0CA7 DEMATURE.\n", sizeof(buffer) - 1, 100);
//
//		}
//
//}

void P0DE7(void) {
	int P0DE7_condition_met = 0;


		if(max_cell >= 4.25){
			P0DE7_condition_met = 1;
		}

	static uint32_t start_time;
	static uint8_t P0DE7_flag = 0;
	char buffere7[100];
	static int test_flag = 0;

	if(P0DE7_condition_met){
		test_flag = 1;
		if(!P0DE7_flag){
			start_time = HAL_GetTick();
			P0DE7_flag = 1;
			sprintf(buffere7, "P0DE7 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffere7, strlen(buffere7), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffere7, "Elapsed Time P0DE7: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);



			 if(elapsed_time >= 3){
				 sprintf(buffere7, "DTC P0DE7 Mature\n");
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffere7, strlen(buffere7), 100);
			 }
		}
	}else{
		int all_cells_below_threshold = 1;
			if(max_cell < 4.23){
				all_cells_below_threshold = 0;
			}

		if(!all_cells_below_threshold){
			P0DE7_flag = 0;
			if(test_flag == 1)
			{
				sprintf(buffer, "DTC P0DE7 Demature.\n");
				test_flag = 0;
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			}
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
					 sprintf(buffer, "DTC P1C01 Mature\n");
					 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
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
	static int test_flag = 0;


	if(P0DE6_condition_met){
		test_flag = 1;
		if(!P0DE6_flag){
			start_time = HAL_GetTick();
			P0DE6_flag = 1;
			sprintf(buffer, "\nP0DE6 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "\nElapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 4){
				 sprintf(buffer, "\nDTC P0DE6 Mature.\n");
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			 }
		}
	}else{
		int all_cells_below_threshold = 1;
			if(min_cell < 2.9){
				all_cells_below_threshold = 0;
			}
		if(!all_cells_below_threshold){
			P0DE6_flag = 0;
			if(test_flag == 1)
			{
				sprintf(buffer, "\nDTC P0DE6 Demature.\n");
				test_flag = 0;
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			}
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
			sprintf(buffer, "\nP1C00 condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
		}else{

		     uint32_t current_time = HAL_GetTick();
			 float elapsed_time = (float)(current_time - start_time) / 1000.0;

		     sprintf(buffer, "\nElapsed Time: %.2f seconds\n", elapsed_time);
			 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

			 if(elapsed_time >= 3){
				 sprintf(buffer, "\nDTC P1C00 Mature\n");
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
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
	static int test_flag = 0;

	if(P1A9B_condition_met){
		 test_flag = 1;
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
				 sprintf(buffer, "\nDTC P1A9B Mature\n");
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			 }
		}
	}else{
		int all_cells_below_threshold = 1;
			if(max_temp < 55){
				all_cells_below_threshold = 0;
			}
		if(!all_cells_below_threshold){
			P1A9B_flag = 0;
			if(test_flag == 1)
			{
				sprintf(buffer, "DTC P1A9B Demature.\n");
				HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
				test_flag = 0;
			}
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
				 sprintf(buffer, "DTC P0A7E Mature\n");
				 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
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
	static int test_flag = 0;


	if((pri_current >= 1350 || pri_current <= -1350) || (pri_current_fa = 1 && sec_current >= 1350))
	{
		P0CA7_condition_met = 1;
	}

		if(P0CA7_condition_met){
			test_flag = 1;
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
				 if(test_flag == 1)
				 {
					 if(elapsed_time >= 0.5){
						 sprintf(buffer, "DTC P0CA7 Mature\n");
						 test_flag = 0;
						 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
					 }

				 }
			}
		}
}


void P0A0C (void)
{
	int P0A0C_condition_met = 0;

		static uint32_t start_time;
		static uint8_t P0A0C_flag = 0;
		char buffer[100];
		static int test_flag = 0;


		if(duty_cycle == 0)
		{
			P0A0C_condition_met = 1;
		}

			if(P0A0C_condition_met){
				test_flag = 1;
				if(!P0A0C_flag){
					//first time entering the condition.
					start_time = HAL_GetTick();
					P0A0C_flag = 1;
					sprintf(buffer, "P0A0C condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
					HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
				}else{

				     uint32_t current_time = HAL_GetTick();
					 float elapsed_time = (float)(current_time - start_time) / 1000.0;

				     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
					 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

					 if(elapsed_time >= 0.5){
						 sprintf(buffer, "DTC P0A0C Mature\n");
						 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
					 }
				}
			}
			else{
					int all_cells_below_threshold = 1;
						if(duty_cycle == 50){
							all_cells_below_threshold = 0;
						}
					if(!all_cells_below_threshold){
						P0A0C_flag = 0;
						if(test_flag == 1)
						{

							sprintf(buffer, "DTC P0A0C Demature.\n");
							test_flag = 0;
							HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
						}
					}
				}
}

void P29FF(void) {
    int P29FF_condition_met = 0;
    static uint32_t start_time;
    static uint8_t P29FF_flag = 0;
    char buffer[256];
    int size = MAX_TEMPS;

    if (min_cell < 2.1){
        for (int i = 0; i < size - 1; i++){
            for (int j = i + 1; j < size; j++)
            {
                if (fabs(tempValues[i] - tempValues[j]) == 2)
                {
                    P29FF_condition_met = 1;
                    break; // Exit the inner loop if condition is met
                }
            }
            if (P29FF_condition_met)
            {
                break; // Exit the outer loop if condition is met
            }
        }
    }
    if (min_cell < 2.1) {
            for (int k = 0; k < size; k += 2) {
                // Check the temperature difference within the same module
                if (fabs(tempValues[k] - tempValues[k + 1]) >= 15) {
                    P29FF_condition_met = 1;
                    break; // Exit the loop if condition is met
                }
            }
        }
    if (P29FF_condition_met)
    {
        if (!P29FF_flag)
        {
            start_time = HAL_GetTick();
            P29FF_flag = 1;
            sprintf(buffer, "P29FF condition met, Start_time set: %.2f s\n", (float)start_time / 1000.0);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
        }

        else
        {
            uint32_t current_time = HAL_GetTick();
            float elapsed_time = (float)(current_time - start_time) / 1000.0;

            sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
            HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

            if (elapsed_time >= 600)
            {
                HAL_UART_Transmit(&huart2, (uint8_t*)"DTC P29FF Mature\n", 17, 100);
            }
        }
    }

    else
    {
        P29FF_flag = 0;
    }
}


void P0A0B (void)
{
	int P0A0B_condition_met = 0;

		static uint32_t start_time;
		static uint8_t P0A0B_flag = 0;
		char buffer[100];
		static int test_flag = 0;

		if(freq < 83 || duty_cycle > 55)
		{
			P0A0B_condition_met = 1;
		}

			if(P0A0B_condition_met){
				test_flag = 1;
				if(!P0A0B_flag){
					//first time entering the condition.
					start_time = HAL_GetTick();
					P0A0B_flag = 1;
					sprintf(buffer, "P0A0B condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
					HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
				}else{

				     uint32_t current_time = HAL_GetTick();
					 float elapsed_time = (float)(current_time - start_time) / 1000.0;

				     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
					 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

					 if(elapsed_time >= 0.5){
						 HAL_UART_Transmit(&huart2, (uint8_t*)"P0A0B\n", 6, 100);
					 }
				}
			}
			else{
					int all_cells_below_threshold = 1;
						if(freq == 88 && duty_cycle == 50){
							all_cells_below_threshold = 0;
						}
					if(all_cells_below_threshold){
						P0A0B_flag = 0;
						if(test_flag == 1)
						{
							sprintf(buffer, "All cells below threshold, P0A0B reset.\n");
							test_flag = 0;
							HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
						}
					}
				}
}

void P0A0C (void)
{
	int P0A0C_condition_met = 0;

		static uint32_t start_time;
		static uint8_t P0A0C_flag = 0;
		char buffer[100];

		static int test_flag = 0;
		if(duty_cycle == 0)
		{
			P0A0C_condition_met = 1;
		}

			if(P0A0C_condition_met){
				test_flag = 1;
				if(!P0A0C_flag){
					//first time entering the condition.
					start_time = HAL_GetTick();
					P0A0C_flag = 1;
					sprintf(buffer, "P0A0C condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
					HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
				}else{

				     uint32_t current_time = HAL_GetTick();
					 float elapsed_time = (float)(current_time - start_time) / 1000.0;

				     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
					 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

					 if(elapsed_time >= 0.5){
						 HAL_UART_Transmit(&huart2, (uint8_t*)"P0A0C\n", 6, 100);
					 }
				}
			}
			else{
					int all_cells_below_threshold = 1;
						if(duty_cycle == 50){
							all_cells_below_threshold = 0;
						}
					if(all_cells_below_threshold){
						P0A0C_flag = 0;
						if(test_flag == 1)
						{
							sprintf(buffer, "All cells below threshold, P0A0C reset.\n");
							test_flag = 0;
							HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

						}
					}
				}
}

void P0A0D (void)
{
	int P0A0D_condition_met = 0;

		static uint32_t start_time;
		static uint8_t P0A0D_flag = 0;
		char buffer[100];
		static int test_flag = 0;
		if(duty_cycle == 100)
		{
			P0A0D_condition_met = 1;
		}

			if(P0A0D_condition_met){
				test_flag = 1;
				if(!P0A0D_flag){
					//first time entering the condition.
					start_time = HAL_GetTick();
					P0A0D_flag = 1;
					sprintf(buffer, "P0A0D condition met, Start_time set: %.2f s\n", (float)start_time/1000.0);
					HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
				}else{

				     uint32_t current_time = HAL_GetTick();
					 float elapsed_time = (float)(current_time - start_time) / 1000.0;

				     sprintf(buffer, "Elapsed Time: %.2f seconds\n", elapsed_time);
					 HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

					 if(elapsed_time >= 0.5){
						 HAL_UART_Transmit(&huart2, (uint8_t*)"P0A0D\n", 6, 100);
					 }
				}
			}
			else{
					int all_cells_below_threshold = 1;
						if(duty_cycle == 50){
							all_cells_below_threshold = 0;
						}
					if(all_cells_below_threshold){
						P0A0D_flag = 0;
						if(test_flag == 1)
						{
							sprintf(buffer, "All cells below threshold, P1A9B reset.\n");
							test_flag = 0;
							HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);

						}
					}
				}
}


void display_values(void) {
    char msg[64];

    for (int i = 0; i < MAX_CELLS; i++) {
        sprintf(msg, "Cell%d:%.2lf\n\r", i, cellVoltages[i]);
        //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    for (int i = 0; i < MAX_TEMPS; i++) {
        sprintf(msg, "Temp%d:%.2lf\n\r", i, tempValues[i]);
        //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  HAL_UART_Receive_IT(&huart2, (uint8_t *)&received_char, 1);
  int count = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  count++;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2,0);
//	  HAL_Delay(1000);
//
//	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2,1);
//
//	  HAL_Delay(1000);
	  display_values();
	  //HAL_Delay(5000);
      P0DE7();
      P1C01();
      P0DE6();
      P1C00();
      P1A9B();
      P0A7E();
      P0CA7();
      P29FF();
      HAL_Delay(1000);

	  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adcResultsDMA, adcChannelCount);
	  while(adcConversionCplt ==0)
	  {
		  //its set to 1 inside the callback.
	  }
	  adcConversionCplt = 0;

	  float potADC[adcChannelCount];
	  for (int i = 0; i< adcChannelCount; i++)
	  {
		  potADC[i] = (float)adcResultsDMA[i] * (3.3 / 4095.0);
		  potADC[i] = (potADC[i] / 3.3) * (6.0);
	  }



//	  snprintf(txBuf, 225 ,"\n\npot1: %d\t, pot2: %d\t, pot3: %d\t, pot4: %d\t, pot5: %d\t, pot6: %d\t, pot7: %d\t,\n pot8: %d\t, pot9: %d\t, pot10: %d\t, pot11: %d\t, pot12: %d\t, pot13: %d\t, pot14: %d\n\n",
//	  adcResultsDMA[0], adcResultsDMA[1], adcResultsDMA[2], adcResultsDMA[3], adcResultsDMA[4], adcResultsDMA[5],adcResultsDMA[6], adcResultsDMA[7], adcResultsDMA[8], adcResultsDMA[9],
//	  adcResultsDMA[10], adcResultsDMA[11], adcResultsDMA[12], adcResultsDMA[13]);

	  snprintf(txBuf, 225, "\n\npot1: %0.2f\t, pot2: %0.2f\t, pot3: %0.2f\t, pot4: %0.2f\t, pot5: %0.2f\t, pot6: %0.2f\t, pot7: %0.2f\t,\n pot8: %0.2f\t, pot9: %0.2f\t, pot10: %0.2f\t, pot11: %0.2f\t, pot12: %0.2f\t, pot13: %0.2f\t, pot14: %0.2f\n",
			  potADC[0], potADC[1], potADC[2], potADC[3], potADC[4], potADC[5], potADC[6], potADC[7], potADC[8],potADC[9], potADC[10], potADC[11], potADC[12], potADC[13]);

	  uint32_t t1_read, t2_read;
	  float T0 = 25 + 273.15;
	  float t1_vol, t2_vol, r1_vol, r2_vol, r_1, r_2, ln1, ln2, Tx1, Tx2;
	  t1_read = adcResultsDMA[14];
	  t1_vol = (float)t1_read * (3.3 / 4095.0);
	  r1_vol = 3.3 - t1_vol;

	  r_1 = (t1_vol)/ (r1_vol / 10000.0);
	  ln1 = log(r_1 / 10000);
	  Tx1 = (1/ ((ln1/ 3977)+ (1/ T0)));
	  Tx1 = Tx1 - 273.15;


	  t2_read  = adcResultsDMA[15];
	  t2_vol = (float)t2_read * (3.3 / 4095.0);
	  r2_vol = 3.3 - t2_vol;
	  r_2 = (t2_vol)/ (r2_vol / 10000);
	  ln2 = log(r_2 / 10000);
	  Tx2 = (1/ ((ln2/ 3977)+ (1/ T0)));
	  Tx2 = Tx2 - 273.15;


	  snprintf(tempBuf, 50, "\n Temp1: %0.2f Temp2: %0.2f \n", Tx1, Tx2);
  	  HAL_UART_Transmit(&huart2, (uint8_t *) txBuf, sizeof(txBuf), HAL_MAX_DELAY);

  	  HAL_UART_Transmit(&huart2, (uint8_t *) tempBuf, strlen(tempBuf), HAL_MAX_DELAY);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
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
  htim1.Init.Prescaler = 713;
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
  sConfigOC.Pulse = 127;
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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
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
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);

  /* USER CODE END TIM3_Init 2 */

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
  huart2.Init.BaudRate = 9600;
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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_0
                          |GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE0
                           PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_0
                          |GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	//add if stmt to check if the handle is for hadc1 for potentiometers.
	adcConversionCplt =1;

}
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

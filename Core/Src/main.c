/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay_micros.h"
#include "string.h"
#include "drv8255.h"
#include "tasks.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FREERTOS 1
#define NORMAL ;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t USBbuffer[6] = {
    0,
};
uint32_t time = 0;
uint8_t position_sensor = 0;
uint8_t run_flag = 1;
uint32_t step_et = 0;
uint8_t direction = 0;
uint8_t end = 0;
uint8_t time_input = 0;
uint8_t irq_flag = 0;
////////////////////////////
char trans_str[80] = {
    0,
};
uint8_t TXBuffer[20] = {
    0,
};

/* Extern variables */
extern TIM_HandleTypeDef htim2;

extern drv8255_t servo_1;
extern drv8255_t step_1;
extern drv8255_t step_2;
extern enum TASK task;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void setdir(void);
uint8_t swap(uint8_t byte);
void step_run(uint8_t LSB, uint8_t Byte1, uint8_t Byte2, uint8_t MSB);
void send_stop(uint32_t steps_completed);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void setdir(void)
{
  if (position_sensor == 1)
  {
    if (USBbuffer[1] == 1)
    {
      GPIOB->BSRR |= (1 << 4); // set dir 1
      direction = USBbuffer[1];
    }
    else
    {
      run_flag = 5;
      memset(USBbuffer, 0, 6);
    }
  }
  else if (position_sensor == 2)
  {
    if (USBbuffer[1] == 0)
    {
      GPIOB->BSRR |= (1 << 20); // set dir 0
      direction = USBbuffer[1];
    }
    else
    {
      run_flag = 5;
      memset(USBbuffer, 0, 6);
    }
  }
  else if (position_sensor == 0)

  {
    if (USBbuffer[1] == 1)
    {
      GPIOB->BSRR |= (1 << 4); // set dir 1
      direction = USBbuffer[1];
    }
    else if (USBbuffer[1] == 0)
    {
      GPIOB->BSRR |= (1 << 20); // set dir 0
      direction = USBbuffer[1];
    }
  }
}

// step_run(USBbuffer[5], USBbuffer[4], USBbuffer[3], USBbuffer[2]);
void step_run(uint8_t LSB, uint8_t Byte1, uint8_t Byte2, uint8_t MSB)
{
  uint32_t steps = LSB | (Byte1 << 8) | (Byte2 << 16) | (MSB << 24); // cast 4 bytes to 32 bit
  uint32_t init_steps = steps;
  uint32_t step_et = 0;

  GPIOB->BSRR |= (1 << 6);  // set srven Pin
  GPIOB->BSRR |= (1 << 10); // set RELAY Pin
  HAL_Delay(200);

  while (steps != 0 && run_flag != 5 && USBbuffer[0] != 0x00)
  {
    time = USBbuffer[0];
    GPIOB->BSRR |= (1 << 5); // SET PUL+ pin
    delay_us(time * 100);
    GPIOB->BSRR |= (1 << 21); // RESET PUL+ pin
    delay_us(time * 100);
    steps--;
  }
  HAL_Delay(200);
  GPIOB->BSRR |= (1 << 26); // Reset RELAY Pin
  step_et = init_steps - steps;
  if (run_flag != 5)
    position_sensor = 0;
  delay_us(1000);
  GPIOB->BSRR |= (1 << 22); // reset srven
  send_stop(step_et);
  memset(USBbuffer, 0, 4);
  return;
}
#if FREERTOS == 0
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == Break1_Pin)
  {
    if (HAL_GPIO_ReadPin(RELAY_1_GPIO_Port, RELAY_1_Pin) == GPIO_PIN_SET)
    {
      if (USBbuffer[0] != 0x00)
      {
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        run_flag = 5;
        position_sensor = 1;
        end = 1;
        GPIOC->BSRR |= (1 << 13); // SET LED
        time_input = HAL_GetTick();
        irq_flag = 1;
      }
    }
  }
  else if (GPIO_Pin == Break2_Pin)
  {
    if (HAL_GPIO_ReadPin(RELAY_2_GPIO_Port, RELAY_2_Pin) == GPIO_PIN_SET)
    {
      if (USBbuffer[0] != 0x00)
      {
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        run_flag = 5;
        position_sensor = 2;
        end = 2;
        GPIOC->BSRR |= (1 << 29); // RESET LED
        time_input = HAL_GetTick();
        irq_flag = 1;
      }
    }
  }
  else if (GPIO_Pin == Break3_Pin)
  {
    if (HAL_GPIO_ReadPin(RELAY_2_GPIO_Port, RELAY_2_Pin) == GPIO_PIN_SET)
    {
      if (USBbuffer[0] != 0x00)
      {
        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        run_flag = 5;
        position_sensor = 2;
        end = 2;
        GPIOC->BSRR |= (1 << 29); // RESET LED
        time_input = HAL_GetTick();
        irq_flag = 1;
      }
    }
  }
}
#endif
uint8_t swap(uint8_t byte)
{
  return ((byte << 4) | (byte >> 4));
}

void send_stop(uint32_t steps_completed)
{
  TXBuffer[0] = position_sensor;
  TXBuffer[1] = direction; // �����������
  TXBuffer[5] = steps_completed & 0xFF;
  TXBuffer[4] = (steps_completed & (0xFF << 8)) >> 8;
  TXBuffer[3] = (steps_completed & (0xFF << 16)) >> 16;
  TXBuffer[2] = (steps_completed & (0xFF << 24)) >> 24;
  TXBuffer[6] = end;
  //  CDC_Transmit_FS(TXBuffer, 7);
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM11 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

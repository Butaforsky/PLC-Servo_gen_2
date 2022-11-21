/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "parser.h"
#include "tasks.h"
#include "uart_lib.h"
#include "DRV8255.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define user_uart huart1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern parser_t parser;
extern uart_t uart;
extern TIM_HandleTypeDef htim2;

enum TASK task = {
    0,
};

enum Y_N
{
  YES,
  NO,
} YES_NO;

enum DIRECTION
{
  DIR_FORWARD,
  DIR_BACKWARD,
} DIR;

extern drv8255_t servo_1;
extern drv8255_t step_1;
extern drv8255_t step_2;


/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myServo1Handle;
osThreadId myServo2Handle;
osThreadId myStepperHandle;
osThreadId myServo3Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);
void StartTask05(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myServo1 */
  osThreadDef(myServo1, StartTask02, osPriorityIdle, 0, 128);
  myServo1Handle = osThreadCreate(osThread(myServo1), NULL);

  /* definition and creation of myServo2 */
  osThreadDef(myServo2, StartTask03, osPriorityIdle, 0, 128);
  myServo2Handle = osThreadCreate(osThread(myServo2), NULL);

  /* definition and creation of myStepper */
  osThreadDef(myStepper, StartTask04, osPriorityIdle, 0, 128);
  myStepperHandle = osThreadCreate(osThread(myStepper), NULL);

  /* definition and creation of myServo3 */
  osThreadDef(myServo3, StartTask05, osPriorityIdle, 0, 128);
  myServo3Handle = osThreadCreate(osThread(myServo3), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  clear_get_new_command(&parser);
  input_clear(parser.first_word_desc);
  task = TASK_INIT;
  parser.uart_inst->tail = 0;

  uart_send(&uart, "Ready\r\n");
  uart_send(&uart, "Enter a command and press \"Enter\"\r\n");
  uart_send_r(&uart);
  HAL_UART_Receive_DMA(&user_uart, (u8 *)uart.byte, 1);

  /* Infinite loop */
  for (;;)
  {
    if (parser.end_string_found == 1)
    {
      task = uart_parse(&parser);
      clear_get_new_command(&parser);
      input_clear(parser.first_word_desc);
    }
    if(task == TASK_ERROR)
    {
      uart_send(&uart, "Error\r\n");
      task = TASK_INIT;
    }
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
 * @brief Function implementing the myServo1 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */

  /* Infinite loop */
  for (;;)
  {
    if (task == TASK_SET_X && servo_1.stage == 0)
    {
      uart_send(&uart, "set x pos\r\n");
      servo_1.stage = 1;
      servo_1.init_period = parser.parsed_parameters->x_arr;
      servo_1.target_position = parser.parsed_parameters->x_pos;
      drv8255_make_steps(&servo_1, servo_1.inversing, servo_1.target_position);
    }
    if (servo_1.stage == 2)
    {
      drv8255_set_dir(&servo_1, !servo_1.inversing);
      osDelay(2);
      uart_send(&uart, "set x pos completed\r\n");
      uart_send_r(&uart);
      servo_1.stage = 0;
      task = TASK_INIT;
    }
    if (task == TASK_STOP_X)
    {
      HAL_TIM_PWM_Stop(servo_1.htim, servo_1.channel);
      // uart_send(&uart, "stop x recieved\r\n");
      uart_send_r(&uart);
      servo_1.stage = 0;
      task = TASK_INIT;
    }

    osDelay(1000);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the myStepper thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for (;;)
  {
    if (task == TASK_SET_Y && step_1.stage == 0)
    {
      uart_send(&uart, "set y pos\r\n");
      step_1.stage = 1;
      step_1.target_position = parser.parsed_parameters->y_pos;
      step_1.init_period = parser.parsed_parameters->y_arr;
      drv8255_make_steps(&step_1, step_1.inversing, step_1.target_position);
    }
    else if (step_1.stage == 2)
    {
      drv8255_set_dir(&step_1, !step_1.inversing);
      osDelay(2);
      uart_send(&uart, "set y pos completed\r\n");
      uart_send_r(&uart);
      step_1.stage = 0;
      task = TASK_INIT;
    }
    if (task == TASK_STOP_Y)
    {
      uart_send(&uart, "stop y recieved\r\n");
      uart_send_r(&uart);
      HAL_TIM_PWM_Stop(step_1.htim, step_1.channel);
      step_1.stage = 0;
      task = TASK_INIT;
    }

    osDelay(1000);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
 * @brief Function implementing the myStepper2 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for (;;)
  {
    if (task == TASK_SET_Z && step_2.stage == 0)
    {
      uart_send(&uart, "set z pos\r\n");
      step_2.stage = 1;
      step_2.target_position = parser.parsed_parameters->z_pos;
      step_2.init_period = parser.parsed_parameters->z_arr;
      drv8255_make_steps_z(&step_2, step_2.inversing, step_2.target_position);
    }
    if (step_2.stage == 2 && task != TASK_STOP_Z)
    {
      uart_send(&uart, "set z pos completed\r\n");
      uart_send_r(&uart);
      step_2.stage = 0;
      task = TASK_INIT;
    }

    if (task == TASK_STOP_Z)
    {
      task = TASK_INIT;
      uart_send(&uart, "stop z recieved\r\n");
      uart_send_r(&uart);
      HAL_TIM_PWM_Stop_IT(step_2.htim, step_2.channel);
      step_2.stage = 0;
    }
    osDelay(1000);
  }
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
 * @brief Function implementing the extra thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask05 */
void StartTask05(void const * argument)
{
  /* USER CODE BEGIN StartTask05 */
  /* Infinite loop */
  for (;;)
  {
    GPIOC->ODR ^= (1 << 13);

    if (task == TASK_SET_RELAY_1)
    {
      if (parser.parsed_parameters->relay_1_state == !RELAY_ON)
      {
        uart_send(&uart, "relay 1 on\r\n");
        uart_send_r(&uart);
      }
      else if (parser.parsed_parameters->relay_1_state == !RELAY_OFF)
      {
        uart_send(&uart, "relay 1 off\r\n");
        uart_send_r(&uart);
      }
      HAL_GPIO_WritePin(RELAY_1_GPIO_Port, RELAY_1_Pin, !parser.parsed_parameters->relay_1_state);
      task = TASK_INIT;
    }
    else if (task == TASK_SET_RELAY_2)
    {
      if (parser.parsed_parameters->relay_2_state == !RELAY_ON)
      {
        uart_send(&uart, "relay 2 on\r\n");
        uart_send_r(&uart);
      }
      else if (parser.parsed_parameters->relay_2_state == !RELAY_OFF)
      {
        uart_send(&uart, "relay 2 off\r\n");
        uart_send_r(&uart);
      }
      HAL_GPIO_WritePin(RELAY_2_GPIO_Port, RELAY_2_Pin, !parser.parsed_parameters->relay_2_state);
      task = TASK_INIT;
    }
    else if (task == TASK_SET_RELAY_3)
    {
      if (parser.parsed_parameters->relay_3_state == !RELAY_ON)
      {
        uart_send(&uart, "relay 3 on\r\n");
        uart_send_r(&uart);
      }
      else if (parser.parsed_parameters->relay_3_state == !RELAY_OFF)
      {
        uart_send(&uart, "relay 3 off\r\n");
        uart_send_r(&uart);
      }
      HAL_GPIO_WritePin(RELAY_3_GPIO_Port, RELAY_3_Pin, !parser.parsed_parameters->relay_3_state);
      task = TASK_INIT;
    }

    if(task == TASK_UC_RESET)
    {
      NVIC_SystemReset();
    }
    osDelay(1);
  }
  /* USER CODE END StartTask05 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    GPIOC->ODR ^= (1 << 13);
    parser_uart_callback(parser.uart_inst->uart, &parser);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == Break1_Pin)
  {
    uart_send(&uart, "Break1_Pin = 0 \r\n");  //13
    task = TASK_STOP_X;
    HAL_TIM_PWM_Stop_IT(servo_1.htim, servo_1.channel);
  }
  else if (GPIO_Pin == Break2_Pin)            //14
  {
    uart_send(&uart, "Break2_Pin = 0 \r\n");
    task = TASK_STOP_Y;
    HAL_TIM_PWM_Stop_IT(step_1.htim, step_1.channel);
  }
  else if (GPIO_Pin == Break3_Pin)
  {
    uart_send(&uart, "Break3_Pin = 0 \r\n");  //15
    task = TASK_STOP_Y;
    HAL_TIM_PWM_Stop_IT(step_1.htim, step_1.channel);
  }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    if (servo_1.stage == 1)
    {
      if (drv8255_pulse_cmplt_callback(&servo_1) == COMPLETE)
      {
        servo_1.stage = 2;
      }
    }
    if (step_1.stage == 1)
    {
      if (drv8255_pulse_cmplt_callback(&step_1) == COMPLETE)
      {
        step_1.stage = 2;
      }
    }
    if (step_2.stage == 1)
    {
      if (drv8255_pulse_cmplt_callback(&step_2) == COMPLETE)
      {
          step_2.stage = 2;
      }
    }
  }
}
/* USER CODE END Application */

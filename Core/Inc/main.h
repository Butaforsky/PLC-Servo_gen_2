/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "short_types.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED0_Pin GPIO_PIN_13
#define LED0_GPIO_Port GPIOC
#define STEP_1_YELLOW_Pin GPIO_PIN_0
#define STEP_1_YELLOW_GPIO_Port GPIOA
#define STEP_2_Pin GPIO_PIN_1
#define STEP_2_GPIO_Port GPIOA
#define STEP_3_Pin GPIO_PIN_2
#define STEP_3_GPIO_Port GPIOA
#define DIR_1_WHITE_Pin GPIO_PIN_4
#define DIR_1_WHITE_GPIO_Port GPIOA
#define DIR_2_Pin GPIO_PIN_5
#define DIR_2_GPIO_Port GPIOA
#define DIR_3_Pin GPIO_PIN_6
#define DIR_3_GPIO_Port GPIOA
#define EN_1_BLACK_Pin GPIO_PIN_0
#define EN_1_BLACK_GPIO_Port GPIOB
#define EN_2_Pin GPIO_PIN_1
#define EN_2_GPIO_Port GPIOB
#define EN_3_Pin GPIO_PIN_2
#define EN_3_GPIO_Port GPIOB
#define Break1_Pin GPIO_PIN_13
#define Break1_GPIO_Port GPIOB
#define Break1_EXTI_IRQn EXTI15_10_IRQn
#define Break2_Pin GPIO_PIN_14
#define Break2_GPIO_Port GPIOB
#define Break2_EXTI_IRQn EXTI15_10_IRQn
#define Break3_Pin GPIO_PIN_15
#define Break3_GPIO_Port GPIOB
#define Break3_EXTI_IRQn EXTI15_10_IRQn
#define RS_485_EN_Pin GPIO_PIN_8
#define RS_485_EN_GPIO_Port GPIOA
#define RELAY_1_Pin GPIO_PIN_3
#define RELAY_1_GPIO_Port GPIOB
#define RELAY_2_Pin GPIO_PIN_4
#define RELAY_2_GPIO_Port GPIOB
#define RELAY_3_Pin GPIO_PIN_5
#define RELAY_3_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_7
#define LED1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

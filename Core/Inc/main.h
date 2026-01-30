/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define LED_PE0_ONOFF_Pin GPIO_PIN_0
#define LED_PE0_ONOFF_GPIO_Port GPIOE
#define IO_PB8_Light_Pin GPIO_PIN_8
#define IO_PB8_Light_GPIO_Port GPIOB
#define KH1_Pin GPIO_PIN_0
#define KH1_GPIO_Port GPIOF
#define KH3_Pin GPIO_PIN_2
#define KH3_GPIO_Port GPIOF
#define KH2_Pin GPIO_PIN_1
#define KH2_GPIO_Port GPIOF
#define KH4_Pin GPIO_PIN_3
#define KH4_GPIO_Port GPIOF
#define Flash_CS_Pin GPIO_PIN_5
#define Flash_CS_GPIO_Port GPIOH
#define KV5_Pin GPIO_PIN_4
#define KV5_GPIO_Port GPIOG
#define KV4_Pin GPIO_PIN_3
#define KV4_GPIO_Port GPIOG
#define IO_PH10_Light_Pin GPIO_PIN_10
#define IO_PH10_Light_GPIO_Port GPIOH
#define KV3_Pin GPIO_PIN_2
#define KV3_GPIO_Port GPIOG
#define KV2_Pin GPIO_PIN_1
#define KV2_GPIO_Port GPIOG
#define KH6_Pin GPIO_PIN_13
#define KH6_GPIO_Port GPIOF
#define KV1_Pin GPIO_PIN_0
#define KV1_GPIO_Port GPIOG
#define KH5_Pin GPIO_PIN_12
#define KH5_GPIO_Port GPIOF
#define K_Encorder_Pin GPIO_PIN_15
#define K_Encorder_GPIO_Port GPIOF
#define KH7_Pin GPIO_PIN_14
#define KH7_GPIO_Port GPIOF

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

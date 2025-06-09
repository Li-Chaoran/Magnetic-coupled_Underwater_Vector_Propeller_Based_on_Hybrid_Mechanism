/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define KEY_1_Pin GPIO_PIN_0
#define KEY_1_GPIO_Port GPIOA
#define KEY_1_EXTI_IRQn EXTI0_IRQn
#define KEY_2_Pin GPIO_PIN_1
#define KEY_2_GPIO_Port GPIOA
#define KEY_2_EXTI_IRQn EXTI1_IRQn
#define KEY_3_Pin GPIO_PIN_4
#define KEY_3_GPIO_Port GPIOA
#define KEY_3_EXTI_IRQn EXTI4_IRQn
#define LED_RED_Pin GPIO_PIN_5
#define LED_RED_GPIO_Port GPIOC
#define LED_BLUE_Pin GPIO_PIN_2
#define LED_BLUE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
/*----------------------------------声明全局变量----------------------------*/
extern uint8_t volatile g_machine_mode;           //推进器控制方式，默认0-屏幕控制，1为手柄控制
extern uint16_t volatile Rovmotor_PWM;       //ROV电机全局PWM
extern uint8_t volatile g_rov_status;           //ROV电机状态值，默认失能
extern uint8_t volatile g_dm4310_status;        //DM4310关节电机状态，默认失能
extern uint8_t volatile g_dm3910_1_status;        //DM3910_1关节电机状态，默认失能
extern uint8_t volatile g_dm3910_2_status;        //DM3910_2关节电机状态，默认失能
extern uint8_t volatile g_dm3910_3_status;        //DM3910_3关节电机状态，默认失能
extern uint8_t volatile g_transmit_status;      //CAN总线电机是否发送状态，默认不发送

extern float volatile g_dm4310_target_pos;     // DM4310关节电机目标位置
extern float volatile g_dm3910_1_target_pos;   // DM3910关节电机1目标位置
extern float volatile g_dm3910_2_target_pos;   // DM3910关节电机2目标位置
extern float volatile g_dm3910_3_target_pos;   // DM3910关节电机3目标位置

extern double volatile g_dm4310_real_pos;       // DM3410关节电机实际位置
extern double volatile g_dm3910_1_real_pos;     // DM3910关节电机1实际位置
extern double volatile g_dm3910_2_real_pos;     // DM3910关节电机2实际位置
extern double volatile g_dm3910_3_real_pos;     // DM3910关节电机3实际位置

extern uint8_t volatile g_handup;               //1表示抬手，0表示按下
extern uint16_t volatile g_start_tick;          // 系统启动时间，计数未到达10000回调函数不起作用。
/*----------------------------------声明全局变量----------------------------*/

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

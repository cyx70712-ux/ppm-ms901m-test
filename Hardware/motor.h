#ifndef __MOTOR_H
#define __MOTOR_H	

#include "stm32f10x.h"                  // Device header



void Motor_TIM2_Init(u16 arr,u16 psc);//电机初始化

void Servo_TIM3_Init(u16 arr, u16 psc);//舵机初始化

void Motor_calibration(void);//电调校准


#endif  






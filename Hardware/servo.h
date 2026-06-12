#ifndef __SERVO_H
#define __SERVO_H	

#include "stm32f10x.h"                  // Device header


void Servo_TIM3_Init(u16 arr, u16 psc);//舵机初始化


void Servo_Control(void);//舵机控制


#endif  






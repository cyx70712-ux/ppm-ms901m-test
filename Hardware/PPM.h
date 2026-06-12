#ifndef __PPM_H
#define __PPM_H	

#include "stm32f10x.h"                  // Device header



#define PPM_CHANNELS 10  // 
// 初始化PPM信号解析
void PPM_Init(void);

extern volatile uint16_t ppm_pwm_values[10];


static uint16_t mapPPMtoPWM(uint16_t ppm_value);

#endif  

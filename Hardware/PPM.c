#include "PPM.h"

#define PPM_CHANNELS 10  // 支持10个通道

volatile uint16_t ppm_pwm_values[PPM_CHANNELS]; // 存储映射后的PWM值
volatile static uint8_t ppm_index = 0;
volatile static uint32_t last_capture = 0;

// 映射函数：将600-1600μs映射为1000-2000
static uint16_t mapPPMtoPWM(uint16_t ppm_value) {
    const uint16_t PPM_MIN = 600;
    const uint16_t PPM_MAX = 1600;
    const uint16_t PWM_MIN = 1000;
    const uint16_t PWM_MAX = 2000;

    if (ppm_value < PPM_MIN) ppm_value = PPM_MIN;
    if (ppm_value > PPM_MAX) ppm_value = PPM_MAX;

    return (ppm_value - PPM_MIN) * (PWM_MAX - PWM_MIN) / (PPM_MAX - PPM_MIN) + PWM_MIN;
}




void PPM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 720-1;
	TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
	
	
	TIM_ICInitTypeDef TIM_ICInitStruct;
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStruct.TIM_ICFilter = 0x0;//关闭滤波
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//一分频
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM4,&TIM_ICInitStruct);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	
	NVIC_Init(&NVIC_InitStruct);
	TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4,ENABLE);
	
}

void TIM4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_CC1)) {
        const uint16_t current_capture = TIM_GetCapture1(TIM4);
        uint32_t diff = (current_capture >= last_capture)
                      ? (current_capture - last_capture)
                      : (0xFFFF - last_capture + current_capture +1 );
        diff *= 10;  // 转为微秒

        const uint8_t is_rising = (TIM4->CCER & TIM_CCER_CC1P) == 0;

        if (is_rising) {
            last_capture = current_capture;
            TIM_OC1PolarityConfig(TIM4, TIM_ICPolarity_Falling);
        } else {
            if (diff > 3500) {
                ppm_index = 0;
            } else if (ppm_index < 10) {
                ppm_pwm_values[ppm_index++] = mapPPMtoPWM(diff);
            }
            TIM_OC1PolarityConfig(TIM4, TIM_ICPolarity_Rising);
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    }

    if (TIM_GetITStatus(TIM4, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}


//舵机  pa6    TIM3  ch1 
#include "servo.h"
#include "PPM.h"
#include <stdint.h>

// 遥控器输入范围
#define RC_MIN          1000
#define RC_MID          1400
#define RC_MAX          1880

// 舵机输出范围
#define SERVO_MIN       1100
#define SERVO_MID       1500
#define SERVO_MAX       1900

#define DEADZONE        15
#define FILTER_SHIFT    3
#define SERVO_STEP      6


/* 安全限幅函数 */
static int16_t limit_i16(int16_t x, int16_t min, int16_t max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

/* 斜率限制函数 (防止舵机瞬间猛打扫齿) */
static int16_t slew(int16_t target, int16_t current, int16_t step) {
    if (target - current > step)  return current + step;
    if (target - current < -step) return current - step;
    return target;
}

/* 线性映射函数 */
static int16_t map_range(int16_t x, int16_t in1, int16_t in2, int16_t out1, int16_t out2) {
    x = limit_i16(x, in1, in2);
    return ((int32_t)(x - in1) * (out2 - out1)) / (in2 - in1) + out1;
}

/* 舵机定时器初始化 (PA6 - TIM3_CH1) */
void Servo_TIM3_Init(u16 arr, u16 psc) {
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef tim;
    TIM_OCInitTypeDef oc;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_6;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    tim.TIM_Period = arr;
    tim.TIM_Prescaler = psc;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tim);

    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = SERVO_MID;

    TIM_OC1Init(TIM3, &oc);
		TIM_CCxCmd(TIM3, TIM_Channel_1, TIM_CCx_Enable);

    // 【极其关键】开启预装载，防止动态更新 CCR1 时舵机抖动或发出异响
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

		TIM3->CCR1=SERVO_MID;//起始中位
		
    TIM_Cmd(TIM3, ENABLE);
}

/* 舵机逻辑控制函数 */
void Servo_Control(void) {
    int16_t pitch = ppm_pwm_values[1];
    static int32_t filter = SERVO_MID << FILTER_SHIFT;
    int16_t target;

    // 1. 无缝死区与分段映射 (消灭出死区时的突变跳跃)
    if (pitch > (RC_MID - DEADZONE) && pitch < (RC_MID + DEADZONE)) {
        target = SERVO_MID;
    } else if (pitch <= (RC_MID - DEADZONE)) {
        target = map_range(pitch, RC_MIN, RC_MID - DEADZONE, SERVO_MIN, SERVO_MID);
    } else {
        target = map_range(pitch, RC_MID + DEADZONE, RC_MAX, SERVO_MID, SERVO_MAX);
    }

    // 2. EMA 低通滤波 (带双向四舍五入补偿，保证正反打舵精度一致)
    int32_t diff = (target << FILTER_SHIFT) - filter;
    if (diff > 0) {
        diff += (1 << (FILTER_SHIFT - 1));
    } else if (diff < 0) {
        diff -= (1 << (FILTER_SHIFT - 1));
    }
    filter += diff >> FILTER_SHIFT;
    
    int16_t out = filter >> FILTER_SHIFT;

    // 3. 硬件保护：斜率限制 (限制每次循环的最大步进量)
    static int16_t last = SERVO_MID;
    out = slew(out, last, SERVO_STEP);
    last = out;

    // 4. 安全限幅并输出
    TIM3->CCR1 = limit_i16(out, SERVO_MIN, SERVO_MAX);
}


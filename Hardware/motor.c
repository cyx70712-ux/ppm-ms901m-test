/*=========================
     电机定时器初始化
    PA2 -> TIM2_CH3
    PA3 -> TIM2_CH4
=========================*/

#include "motor.h"
#include "PPM.h"
#include "Delay.h"
#include "stm32f10x.h"

/*=========================
      遥控器输入范围
=========================*/
#define RC_MIN          1000
#define RC_MID          1400
#define RC_MAX          1880
#define RC_DEADZONE     15

/*=========================
       电调输出范围
=========================*/
#define ESC_MIN         1000
#define ESC_MAX         2000

/*=========================
         控制参数
=========================*/
#define FILTER_SHIFT    3       // EMA滤波系数
#define MOTOR_STEP      15      // 电机最大步进(斜率限制)


/*=========================
          工具函数
=========================*/
static int16_t limit_i16(int16_t x, int16_t min, int16_t max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static int16_t slew(int16_t target, int16_t current, int16_t step) {
    if (target - current > step)  return current + step;
    if (target - current < -step) return current - step;
    return target;
}

static int16_t map_range(int16_t x, int16_t in1, int16_t in2, int16_t out1, int16_t out2) {
    x = limit_i16(x, in1, in2);
    return ((int32_t)(x - in1) * (out2 - out1)) / (in2 - in1) + out1;
}


void Motor_TIM2_Init(u16 arr, u16 psc) {
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef tim;
    TIM_OCInitTypeDef oc;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    tim.TIM_Period = arr;
    tim.TIM_Prescaler = psc;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &tim);

    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = ESC_MIN;

    TIM_OC3Init(TIM2, &oc);
    TIM_OC4Init(TIM2, &oc);

    // 开启预装载保护，极其关键
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    TIM_Cmd(TIM2, ENABLE);
}

/*=========================
       电调行程校准
=========================*/
void Motor_calibration(void) {
    TIM2->CCR3 = 2000;
    TIM2->CCR4 = 2000;
    Delay_ms(3000);

    TIM2->CCR3 = 1000;
    TIM2->CCR4 = 1000;
    Delay_ms(3000);
}

/*=========================
        电机控制核心
=========================*/
void Motor_Control(void) {
    uint16_t raw_throttle = ppm_pwm_values[2];
    
    static int32_t filter = ESC_MIN << FILTER_SHIFT;
    static int16_t last_left = ESC_MIN;
    static int16_t last_right = ESC_MIN;

    /*-----------------------
     1. 极其重要的失联保护
    ------------------------*/
    if (raw_throttle < 900 || raw_throttle > 2200) {
        TIM2->CCR3 = ESC_MIN;
        TIM2->CCR4 = ESC_MIN;
        
        // 重置所有内部状态，防止复联瞬间电机暴走
        filter = ESC_MIN << FILTER_SHIFT;
        last_left = ESC_MIN;
        last_right = ESC_MIN;
        return;
    }

    /*-----------------------
     2. 输入限幅与油门映射
    ------------------------*/
    uint16_t roll = limit_i16(ppm_pwm_values[0], RC_MIN, RC_MAX);
    uint16_t throttle = limit_i16(raw_throttle, RC_MIN, RC_MAX);

    int16_t target;
    if (throttle < 1050) {
        target = ESC_MIN; // 怠速死区
    } else {
        target = map_range(throttle, RC_MIN, RC_MAX, ESC_MIN, ESC_MAX);
    }

    /*-----------------------
     3. EMA平滑滤波 (带补偿)
    ------------------------*/
    int32_t diff = (target << FILTER_SHIFT) - filter;
    if (diff >= 0) {
        filter += (diff + (1 << (FILTER_SHIFT - 1))) >> FILTER_SHIFT;
    } else {
        filter += (diff - (1 << (FILTER_SHIFT - 1))) >> FILTER_SHIFT;
    }
    
    int16_t power = filter >> FILTER_SHIFT;

    /*-----------------------
     4. 差速混控解算
    ------------------------*/
    int16_t mix = 0;
    if (power > 1100) {
        int16_t offset = roll - RC_MID;
        
        // 摇杆防抖死区
        if (offset > -RC_DEADZONE && offset < RC_DEADZONE) {
            offset = 0;
        }
        
        // 动态差速比例
        mix = (offset * (power - 1000)) / 3000;
    }

    mix = limit_i16(mix, -150, 150); // 限制最大差速量

    int16_t left_target = limit_i16(power + mix, ESC_MIN, ESC_MAX);
    int16_t right_target = limit_i16(power - mix, ESC_MIN, ESC_MAX);

    /*-----------------------
     5. 最大步进斜率限制 (防过载)
    ------------------------*/
    last_left = slew(left_target, last_left, MOTOR_STEP);
    last_right = slew(right_target, last_right, MOTOR_STEP);

    /*-----------------------
     6. 输出到底层PWM
    ------------------------*/
    TIM2->CCR3 = last_left;
    TIM2->CCR4 = last_right;
}


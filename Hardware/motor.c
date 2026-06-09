//电机PA2 PA3 TIM2 ch3  -4 
//舵机  pa6  pa7  TIM3  ch1  2
//解锁
#include "stm32f10x.h"                  // Device header

#include "motor.h"
#include "Delay.h"


void Motor_TIM2_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInit;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIOA时钟
	
  GPIO.GPIO_Pin= GPIO_Pin_2|GPIO_Pin_3;     //TIM2 pwm通道引脚
  GPIO.GPIO_Mode=GPIO_Mode_AF_PP;
  GPIO.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO);
	
	//定时器TIM2初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 	
	TIM_OCInit.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInit.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInit.TIM_OCPolarity=TIM_OCPolarity_High;

	TIM_OC3Init(TIM2,&TIM_OCInit);    //通道3
	TIM_OC4Init(TIM2,&TIM_OCInit);    //通道4
	

	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);
	TIM_OC4PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM2, ENABLE);  //使能TIM2	
}
void Servo_TIM3_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInit;
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能GPIOA时钟
	
  GPIO.GPIO_Pin= GPIO_Pin_6|GPIO_Pin_7;     //TIM2 pwm通道引脚
  GPIO.GPIO_Mode=GPIO_Mode_AF_PP;
  GPIO.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO);
	
	//定时器TIM2初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 	
	TIM_OCInit.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInit.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInit.TIM_OCPolarity=TIM_OCPolarity_High;

	TIM_OC1Init(TIM3,&TIM_OCInit);    //通道3
	TIM_OC2Init(TIM3,&TIM_OCInit);    //通道4
	

	TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM3, ENABLE);  //使能TIM2	
}

void Motor_calibration(void){
	 
	 		TIM2->CCR1=2000;
			Delay_ms(1000);
			Delay_ms(1000);
			Delay_ms(1000);
			Delay_ms(1000);
			TIM2->CCR1=1000;
}




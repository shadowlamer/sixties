#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "inc/generator.h"

void TIM8_UP_IRQHandler()
{
}

void generator_start()
{
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
}

void generator_stop()
{
	TIM_CtrlPWMOutputs(TIM1,DISABLE);
}

void generator_init()
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_2);

	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);
	base_timer.TIM_Prescaler = 12;
	base_timer.TIM_Period = 100;
	base_timer.TIM_RepetitionCounter = 0;
	base_timer.TIM_ClockDivision = 4;
	base_timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &base_timer);

	TIM_OCInitTypeDef timer_oc;
	TIM_OCStructInit(&timer_oc);
	timer_oc.TIM_Pulse = 30;
	timer_oc.TIM_OCMode = TIM_OCMode_PWM1;
	timer_oc.TIM_OutputState = TIM_OutputState_Enable;
	timer_oc.TIM_OutputNState = TIM_OutputNState_Disable;
	timer_oc.TIM_OCPolarity = TIM_OCPolarity_High;
	timer_oc.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	timer_oc.TIM_OCIdleState = TIM_OCIdleState_Reset;
	timer_oc.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

	TIM_OC2Init(TIM1, &timer_oc);

	TIM_Cmd(TIM1, ENABLE);
}

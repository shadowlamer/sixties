#include "stm32f0xx_tim.h"
#include "stm32f0xx_rcc.h"
#include "inc/delay.h"

void delay_init()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE); //for delay timer

}

void delay_us(uint16_t value)
{
/*
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);
	base_timer.TIM_Prescaler = SystemCoreClock / 1000000;
	base_timer.TIM_Period = value;
	base_timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &base_timer);

	TIM_Cmd(TIM6, ENABLE);
	while (TIM_GetFlagStatus(TIM6, TIM_FLAG_Update) == RESET);
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	TIM_Cmd(TIM6, DISABLE);
*/
}

void delay_ms(uint16_t value)
{
	uint32_t i;
	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);
	base_timer.TIM_Prescaler = SystemCoreClock / 1000;
	base_timer.TIM_Period = 100;
	base_timer.TIM_CounterMode = TIM_CounterMode_Up;
	base_timer.TIM_RepetitionCounter = 1000;
	TIM_TimeBaseInit(TIM14, &base_timer);
	TIM_Cmd(TIM14, ENABLE);
	TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
	for (i=0;i<value*100;i++)
		while (TIM_GetITStatus(TIM14, TIM_IT_Update) == RESET);
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	TIM_Cmd(TIM14, DISABLE);
}

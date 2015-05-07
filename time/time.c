
#include "stm32f10x.h"
#include "time.h"

void time6_init(void)
{
 	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  /* Enable the TIM6 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* Time base configuration */
	// 每2000个计数产生一个更新事件
  TIM_TimeBaseStructure.TIM_Period = 2000;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	// 72M/36000=2kHz
  TIM_TimeBaseStructure.TIM_Prescaler = 36000-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	TIM_UpdateRequestConfig(TIM6, TIM_UpdateSource_Regular);

	/* TIM IT enable */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

  /* TIM2 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}

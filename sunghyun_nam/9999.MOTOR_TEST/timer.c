#include "device_driver.h"

#define TIM2_TICK		20						// usec
#define TIM2_FREQ   	(1000000./TIM2_TICK) 	// Hz
#define TIM2_1ms_Pls	(TIM2_FREQ/1000.)		// 개
#define TIM2_MAX		(0xFFFFu)

#define TIM3_TICK		20						// usec
#define TIM3_FREQ   	(1000000./TIM3_TICK) 	// Hz
#define TIM3_1ms_Pls	(TIM3_FREQ/1000.)		// 개
#define TIM3_MAX		(0xFFFFu)

#define TIM5_FREQ					8000000			// Hz
#define TIM5_TICK					(1000000./TIM5_FREQ)	// usec
#define TIME5_PLS_OF_1ms			(1000/TIM5_TICK)

extern volatile int TIM3_Expired;

void Timer2_Init(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	TIM2->CR1 = (1<<4)|(1<<3);
	TIM2->PSC = (unsigned int)(TIMXCLK / TIM2_FREQ + 0.5)-1;

	NVIC_EnableIRQ((IRQn_Type)28);
}

void Timer2_Start(int time)
{	
	TIM2->ARR = time * TIM2_1ms_Pls - 1;
	
	Macro_Set_Bit(TIM2->EGR, 0);
	Macro_Clear_Bit(TIM2->SR, 0);
	NVIC_ClearPendingIRQ((IRQn_Type)28);
	Macro_Set_Bit(TIM2->DIER, 0);
	Macro_Set_Bit(TIM2->CR1, 0);
}

void Timer2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, 0);
	Macro_Clear_Bit(TIM2->SR, 0);   
    NVIC_ClearPendingIRQ((IRQn_Type)28);
	Macro_Clear_Bit(TIM2->DIER, 0); 
}

void Timer3_Init(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 1);

	TIM3->CR1 = (1<<4)|(1<<3);
	TIM3->PSC = (unsigned int)(TIMXCLK / TIM3_FREQ + 0.5)-1;
	
	NVIC_EnableIRQ((IRQn_Type)29);
}

void Timer3_Start(int time)
{	
	TIM3_Expired = 0;

	Macro_Clear_Bit(TIM3->DIER, 0); 

	TIM3->ARR = time * TIM3_1ms_Pls - 1;

	Macro_Set_Bit(TIM3->EGR, 0);
	Macro_Clear_Bit(TIM3->SR, 0);
	NVIC_ClearPendingIRQ((IRQn_Type)29);
	Macro_Set_Bit(TIM3->DIER, 0);
	Macro_Set_Bit(TIM3->CR1, 0);
}

void Timer3_Stop(void)
{
	TIM3_Expired = 1;

	Macro_Clear_Bit(TIM3->CR1, 0);
	Macro_Clear_Bit(TIM3->SR, 0);   
    NVIC_ClearPendingIRQ((IRQn_Type)29);
}

void Timer5_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0);
	Macro_Set_Bit(RCC->APB1ENR, 3);

	TIM5->CR1 = (0x1<<4)|(0x0<<3); // down, repeat
	TIM5->PSC = TIMXCLK / TIM5_FREQ - 1;
	TIM5->ARR = (int)((double)TIM5_FREQ/PWM_INPUT_FREQ  + 0.5)	- 1;
	TIM5->CCER = (0<<5)|(1<<4)|(0<<1)|(1<<0);
	TIM5->CCMR1 = (0x6 << 4) | (0x6 << 12);
}

void Timer5_Out_Pwm_Generator(int duty)
{
	TIM5->CCR1 = TIM5->ARR * duty / 100;
	TIM5->CCR2 = TIM5->ARR * duty / 100;
	
	Macro_Set_Bit(TIM5->EGR,0);
	Macro_Set_Bit(TIM5->CR1,0);
}
#include "device_driver.h"

#define TIM2_TICK         	(20) 				// usec
#define TIM2_FREQ 	  		(1000000/TIM2_TICK)	// Hz
#define TIME2_PLS_OF_1ms  	(1000/TIM2_TICK)
#define TIM2_MAX	  		(0xffffu)

void Timer2_Init(void){
	Macro_Set_Bit(RCC->APB1ENR, 0);
	TIM2->CR1 = (1<<4)|(1<<3);
	TIM2->PSC = (unsigned int)(TIMXCLK/50000.0 + 0.5)-1;
	TIM2->ARR = 3000 * TIME2_PLS_OF_1ms;

}

void Timer2_Start()
{
	Macro_Clear_Bit(TIM2->DIER, 0);// DIER의 관계
	Macro_Set_Bit(TIM2->EGR,0);
	Macro_Clear_Bit(TIM2->SR, 0);
	Macro_Set_Bit(TIM2->CR1, 0);
	Macro_Set_Bit(TIM2->DIER, 0);
}

void Timer2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, 0);
	Macro_Clear_Bit(TIM2->SR, 0);
}

void TIM2_Interrupt_Enable(int en){
	if(en){
		// TIM4 Pending Clear
		Macro_Clear_Bit(TIM2->SR, 0);
		// NVIC Pending Clear
		NVIC_ClearPendingIRQ(28);
		// TIM4 Interrupt Enable
		//Macro_Set_Bit(TIM2->DIER, 0);
		// NVIC Interrupt Enable
		NVIC_EnableIRQ(28);
		Macro_Set_Bit(TIM2->CR1, 0);

	}
	else{
		NVIC_DisableIRQ(28);
		Macro_Clear_Bit(TIM2->CR1, 0);
		Macro_Clear_Bit(TIM2->DIER, 0);
	}
}

#define TIM3_TICK         	(20) 				// usec
#define TIM3_FREQ 	  		(1000000/TIM2_TICK)	// Hz
#define TIME3_PLS_OF_1ms  	(1000/TIM2_TICK)
#define TIME3_lsec 			(1000)

void TIM3_Repeat_Interrupt_Enable(int en)
{
	if(en)
	{
		// TIM3 Clock On
		Macro_Set_Bit(RCC->APB1ENR, 1);
		TIM3->CR1 = (1<<4)|(0<<3);
		TIM3->PSC = (unsigned int)(TIMXCLK/(double)TIM3_FREQ + 0.5)-1;
		TIM3->ARR = TIME3_PLS_OF_1ms * TIME3_lsec;
		Macro_Set_Bit(TIM3->EGR,0);

		// TIM3 Pending Clear
		Macro_Clear_Bit(TIM3->SR, 0);
		// NVIC Pending Clear
		NVIC_ClearPendingIRQ(29);
		// TIM3 Interrupt Enable
		Macro_Set_Bit(TIM3->DIER, 0);
		// NVIC Interrupt Enable
		NVIC_EnableIRQ(29);
		// TIM3 Start
		Macro_Set_Bit(TIM3->CR1, 0);
	}

	else
	{
		NVIC_DisableIRQ(29);
		Macro_Clear_Bit(TIM3->CR1, 0);
		Macro_Clear_Bit(TIM3->DIER, 0);
	}
}

#define TIM5_FREQ					(8000000)			// Hz
#define TIM5_TICK					(1000000/TIM3_FREQ)	// usec
#define TIME5_PLS_OF_1ms			(1000/TIM3_TICK)

void Timer5_Init(void){
	Macro_Set_Bit(RCC->APB1ENR, 3);
	TIM5->CR1 = (1<<4);
	TIM5->CCMR1 = (0x6 << 4) | (0x6 << 12);
	TIM5->CCER = (1 << 0) | (1 << 4);
}

void Timer5_Out_Pwm_Generator(int duty)
{
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	TIM5->PSC = (unsigned int)(TIMXCLK/(double)TIM5_FREQ + 0.5) - 1;
	// 요청한 주파수가 되도록 ARR 설정
	//TIM3->ARR = freq * TIME3_PLS_OF_1ms;
	TIM5->ARR = (int)(((double)TIM5_FREQ / PWM_INPUT_FREQ + 0.5)-1);
	// Duty Rate 50%가 되도록 CCR3 설정
	TIM5->CCR1 = (double)(TIM5->ARR / 100) * duty;
	TIM5->CCR2 = (double)(TIM5->ARR / 100) * duty;
	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM5->EGR, 0);
	// Down Counter, Repeat Mode, Timer Start
	Macro_Set_Bit(TIM5->CR1, 0);
	
}



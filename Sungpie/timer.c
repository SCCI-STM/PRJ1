#include "device_driver.h"
#include <stdio.h>

#define TIM2_TICK         	(20) 				// usec
#define TIM2_FREQ 	  		(1000000/TIM2_TICK)	// Hz
#define TIME2_PLS_OF_1ms  	(1000/TIM2_TICK)
#define TIM2_MAX	  		(0xffffu)

#define TIM3_TICK         	(20) 				// usec
#define TIM3_FREQ 	  		(1000000/TIM3_TICK)	// Hz
#define TIME3_PLS_OF_1ms  	(1000/TIM3_TICK)



// --------------- Timer2 ---------------
void Timer2_Init(void)
{
		// Timer2 ON (Clock ON)
		Macro_Set_Bit(RCC->APB1ENR, 0);

		// Down, One-Shot Mode
		TIM2->CR1 = (1<<4)|(1<<3);

		// 분주비(1991) 설정
		TIM2->PSC = (unsigned int)(TIMXCLK/50000.0 + 0.5)-1;

		// 카운트 값 설정
		TIM2->ARR =  TIME2_PLS_OF_1ms * 3000 - 1;
}

void TIM2_Interrupt_Enable(int en)
{
	if(en)
	{
		// TIM2 Pending Clear
		Macro_Clear_Bit(TIM2->SR, 0);

		// NVIC Pending Clear
		NVIC_ClearPendingIRQ(28);

		// NVIC Interrupt Enable
		NVIC_EnableIRQ(28);

		// TIM2 Start
		Macro_Set_Bit(TIM2->CR1, 0);
	}
	else
	{
		NVIC_DisableIRQ(28);
		Macro_Clear_Bit(TIM2->CR1, 0);
		Macro_Clear_Bit(TIM2->DIER, 0);
	}
}

void Timer2_Start(void)
{
	// 적재, Update Event 발생! SR의 UIF 비트 세트! -> 인터럽트 요청!
	// 하지만 아직 외부 인터럽트 허용 금지
	Macro_Set_Bit(TIM2->EGR, 0);

	Macro_Clear_Bit(TIM2->SR, 0);

	NVIC_ClearPendingIRQ(28);

	// TIM2 Interrupt Enable
	Macro_Set_Bit(TIM2->DIER, 0);
	
	Macro_Set_Bit(TIM2->CR1, 0);
}

void Timer2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, 0);

	Macro_Clear_Bit(TIM2->DIER, 0);
}

int Timer2_IsTimeout(void)
{
	if(Macro_Check_Bit_Set(TIM2->SR, 0))
	{
		Macro_Clear_Bit(TIM2->SR, 0);
		return 1;
	}
	else
	{
		return 0;
	}
}

// --------------- Timer3 ---------------
void Timer3_Init(void)
{
		// Timer3 ON (Clock ON)
		Macro_Set_Bit(RCC->APB1ENR, 1);

		// Down, One-Shot Mode
		TIM3->CR1 = (0x1 << 4)|(0x1 << 3);

		// 분주비(1991) 설정
		TIM3->PSC = (unsigned int)(TIMXCLK/50000.0 + 0.5)-1;

		// 카운트 값 설정
		TIM3->ARR =  TIME2_PLS_OF_1ms * 3000 - 1;
}

void Timer3_Delay(int time)
{
	TIM3->CR1 = (0x1<<4)|(0x1<<3);
	TIM3->PSC = (unsigned int)(TIMXCLK/(double)TIM2_FREQ + 0.5)-1;
	TIM3->ARR = TIME3_PLS_OF_1ms * time - 1;

	Macro_Set_Bit(TIM3->EGR,0);
	Macro_Clear_Bit(TIM3->SR, 0);
	Macro_Set_Bit(TIM3->CR1, 0);

	while(Macro_Check_Bit_Clear(TIM3->SR, 0));

	Macro_Clear_Bit(TIM3->CR1, 0);
}
// ---------------------------------------


// --------------- Timer5 ---------------
#if 0
#define TIM5_FREQ 	  			(10000000) 	      	// Hz
#define TIM5_TICK	  			(1000000/TIM5_FREQ)	// usec
#define TIM5_PLS_OF_1ms  		(1000/TIM5_TICK)
#endif

#define TIM5_TICK	  		(20) 				// usec
#define TIM5_FREQ 	  		(1000000/TIM5_TICK) // Hz
#define TIME5_PLS_OF_1ms  	(1000/TIM5_TICK)
#define TIM5_MAX	  		(0xffffu)

void Timer5_Init(void)
{
	// Timer5 ON (Clock ON)
	Macro_Set_Bit(RCC->APB1ENR, 3);

	// Down, Repeat Mode
	TIM5->CR1 = (0x1 << 4)|(0x0 << 3);

	// Macro_Write_Block(GPIOA->MODER, 0x3, 0x2, 0);  	// PA0 => ALT
	// Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 0); 	// PA0 => AF01
	TIM5->CCMR1 = (0x6 << 4) | (0x6 << 12);
	TIM5->CCER = (0<<5)|(1<<4)|(0<<1)|(1<<0);

}

void Timer5_Out_Pwm_Generator(void)
{
	int duty = ((Uart_Data- '0') * MOTOR_SPEED_UP) + (MOTOR_SPEED_INIT);

	printf("duty : %d\n" , duty);
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	TIM5->PSC = (unsigned int)(TIMXCLK/(double)TIM5_FREQ + 0.5) - 1;
	// 요청한 주파수가 되도록 ARR 설정
	//TIM3->ARR = freq * TIME3_PLS_OF_1ms;
	TIM5->ARR = (int)(((double)TIM5_FREQ / 5 + 0.5)-1);
	// Duty Rate 50%가 되도록 CCR3 설정
	TIM5->CCR1 = (double)(TIM5->ARR / 100) * duty;
	TIM5->CCR2 = (double)(TIM5->ARR / 100) * duty;
	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM5->EGR, 0);
	// Down Counter, Repeat Mode, Timer Start
	Macro_Set_Bit(TIM5->CR1, 0);
}
#if 0
void Timer5_Out_Pwm_Generator(int duty)
{
	// 분주비(1991) 설정
	TIM5->PSC = (unsigned int)(TIMXCLK/50000.0 + 0.5)-1;

	// 카운트 값 설정
	TIM5->ARR =  TIME5_PLS_OF_1ms * 1000 - 1;
	TIM5->CCR1 = (TIM5->ARR + 1) * freq / 100;
	TIM5->CCR2 = (TIM5->ARR + 1) * freq / 100;
}



#define TIM3_FREQ 	  			(8000000) 	      	// Hz
#define TIM3_TICK	  			(1000000/TIM3_FREQ)	// usec
#define TIME3_PLS_OF_1ms  		(1000/TIM3_TICK)

void TIM3_Out_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 1);
	Macro_Set_Bit(RCC->APB1ENR, 1);

	Macro_Write_Block(GPIOB->MODER, 0x3, 0x2, 0);  	// PB0 => ALT
	Macro_Write_Block(GPIOB->AFR[0], 0xf, 0x2, 0); 	// PB0 => AF02

	Macro_Write_Block(TIM3->CCMR2,0xff, 0x60, 0);
	TIM3->CCER = (0<<9)|(1<<8);
}

void TIM3_Out_Freq_Generation(unsigned short freq)
{
	TIM3->PSC = (unsigned int)(TIMXCLK/(double)TIM3_FREQ + 0.5)-1;
	TIM3->ARR = (double)TIM3_FREQ/freq-1;
	TIM3->CCR3 = TIM3->ARR/2;

	Macro_Set_Bit(TIM3->EGR,0);
	TIM3->CR1 = (1<<4)|(0<<3)|(0<<1)|(1<<0);
}

void TIM3_Out_Stop(void)
{
	Macro_Clear_Bit(TIM3->CR1, 0);
}
#endif
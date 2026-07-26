#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

void EXTI15_10_IRQHandler(void)
{
	if(is_key_pressed == 0)
	{
		Timer2_Start();
		is_key_pressed = 1;
		event = KEY_PRESSED;	
	}
	else
	{
		Timer2_Stop();
		is_key_pressed = 0;
		event = KEY_RELEASED;
	}
	EXTI->PR = 0x1 << 13;
	NVIC_ClearPendingIRQ(40);
}

// extern volatile int TIM2_Expired;
void TIM2_IRQHandler(void)
{
	is_long_pressed = 1;
	event = TIMER2_OUT;
	
	// TIM2 Interrupt Pending Clear
	Macro_Clear_Bit(TIM2->SR, 0);

	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(28);
}

extern volatile int Uart_Data_In;
extern volatile unsigned char Uart_Data;

void USART2_IRQHandler(void)
{
	Uart_Data = (unsigned char)USART2->DR;
	Uart_Data_In = 1;
	NVIC_ClearPendingIRQ(38);
}


#include "device_driver.h"
#include <stdio.h>
 

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

extern volatile Event event;

extern volatile int is_key_pressed;
extern volatile int is_long_pressed;

extern volatile int Uart_Data_In;
extern volatile unsigned char Uart_Data;

void EXTI15_10_IRQHandler(void)
{	
	EXTI->PR = 0x1 << 13;
	if (Key_IsPressed())
	{
		is_key_pressed = 1;
		is_long_pressed = 0;

		Timer2_Start(3000);
		event = EVENT_KEY_PRESSED;
	}
	else 
	{
		Timer2_Stop();
		is_key_pressed = 0;
		event = EVENT_KEY_RELEASED;
	}
	NVIC_ClearPendingIRQ((IRQn_Type)40);
}

void TIM2_IRQHandler(void)
{
	Timer2_Stop();
	
	is_long_pressed = 1;
	event = EVENT_TIMER_TIMEOUT;
}

void TIM3_IRQHandler(void)
{
	Timer3_Stop();
}

void USART2_IRQHandler(void)
{
	// 수신된 데이터는 Uart_Data에 저장
	Uart_Data = USART2->DR;
	// Uart_Data_In Flag Setting	
	Uart_Data_In = 1;
	// NVIC Pending Clear
	event = EVENT_UART_INPUT;
	NVIC_ClearPendingIRQ((IRQn_Type)38);
}

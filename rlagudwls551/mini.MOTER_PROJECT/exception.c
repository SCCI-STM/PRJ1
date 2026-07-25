#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR()
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

//방향전환 전에 딜레이 인터럽트 타이머 3번
extern volatile int TIM3_Expired;

void TIM3_IRQHandler(){
	TIM3_Expired = 1;
	// TIM4 Interrupt Pending Clear
	Macro_Clear_Bit(TIM3->SR, 0);
	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(29);
	
}

//키 인터럽트
extern volatile int is_long_pressed;
extern volatile int is_key_pressed;

void EXTI15_10_IRQHandler()
{	
	if(Macro_Check_Bit_Clear(GPIOC->IDR, 13)){//==========================키 눌렀을때 인터럽트
		printf("key push\n");
		event = KEY_PRESSED;
		is_key_pressed = 1;
		is_long_pressed = 0;

		Timer2_Start();
	}
	else{//====================================키 뗐을때 인터럽트
		printf("key rel\n");
		event = KEY_RELEASED;
		is_key_pressed = 0;
		Timer2_Stop();
	}

	EXTI->PR = 0x1 << 13;
	NVIC_ClearPendingIRQ(40);
}

//3초 타이머2 인터럽트
void TIM2_IRQHandler(){
	if (is_key_pressed){
		printf("3 sec out inter\n");
		event = TIMER2_OUT;
		is_long_pressed = 1;
	}

	Macro_Clear_Bit(TIM2->SR, 0);
    NVIC_ClearPendingIRQ(28);
	
}

//UART가 들어왔을때 인터럽트
extern volatile int Uart_Data_In;
extern volatile unsigned char input_motor_dir;
extern volatile int input_motor_speed;

void USART2_IRQHandler()
{
	event = UART_INPUT;
	if(USART2->DR == 'S' || USART2->DR == 's' || USART2->DR == 'F' || USART2->DR == 'f' || USART2->DR == 'R' || USART2->DR == 'r'){
		input_motor_dir = USART2->DR;
		Uart_Data_In = 1;
	}
	else if(USART2->DR >= '0' && USART2->DR <= '9'){
		input_motor_speed = USART2->DR- '0';
		Uart_Data_In = 2;
	}
	else{
		Uart_Data_In = 0;
	}
	
	NVIC_ClearPendingIRQ(38);
}

